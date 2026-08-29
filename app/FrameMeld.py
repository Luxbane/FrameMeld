import sys,json,subprocess,shutil,os,urllib.request
from pathlib import Path
from PySide6.QtCore import QThread,Signal
from PySide6.QtWidgets import QApplication,QMainWindow,QWidget,QVBoxLayout,QGridLayout,QHBoxLayout,QLabel,QPushButton,QLineEdit,QFileDialog,QComboBox,QSpinBox,QTextEdit,QMessageBox,QGroupBox

ROOT=Path(sys.executable).resolve().parent if getattr(sys,"frozen",False) else Path(__file__).resolve().parent.parent
APPDATA=Path(os.environ.get("LOCALAPPDATA",str(Path.home())))/"FrameMeld"
RUNTIME=APPDATA/"runtime"
CFG=APPDATA/"config.json"
SEVENZIP=ROOT/"tools"/"7za.exe"
URLS={"ffmpeg":"https://huggingface.co/Luxbane/FrameMeld/resolve/main/ffmpeg.7z",
"py-amp":"https://huggingface.co/Luxbane/FrameMeld/resolve/main/py-amp.7z",
"rife-cuda":"https://huggingface.co/Luxbane/FrameMeld/resolve/main/rife-cuda.7z",
"rife-ncnn":"https://huggingface.co/Luxbane/FrameMeld/resolve/main/rife-ncnn.7z"}
CHECKS={"ffmpeg":RUNTIME/"ffmpeg"/"ffmpeg.exe","py-amp":RUNTIME/"py-amp"/"python.exe","rife-cuda":RUNTIME/"rife-cuda"/"rife.py","rife-ncnn":RUNTIME/"rife-ncnn"/"rife-ncnn-vulkan.exe"}
MODELS=[
("rife-cuda","RIFE CUDA","NVIDIA GPUs (CUDA). Fastest option — recommended if you have an RTX/GTX card.",True),
("rife-ncnn","RIFE NCNN","AMD / Intel / NVIDIA (Vulkan). Broader GPU compatibility, no CUDA/PyTorch needed. Note: native 2x only — 4x runs two passes.",True),
]
D={"ffmpeg":str(RUNTIME/"ffmpeg"/"ffmpeg.exe"),
"ffprobe":str(RUNTIME/"ffmpeg"/"ffprobe.exe"),
"python":str(RUNTIME/"py-amp"/"python.exe"),
"rife_script":str(RUNTIME/"rife-cuda"/"rife.py"),
"model":str(RUNTIME/"rife-cuda"/"RIFE40"),
"ncnn_exe":str(RUNTIME/"rife-ncnn"/"rife-ncnn-vulkan.exe"),
"ncnn_model":str(RUNTIME/"rife-ncnn"/"rife-v3.1"),
"engine":"rife-cuda",
"preset":"p5","cq":28,"encoder":"av1_nvenc"}
ENCODERS=[
("AV1 NVENC","av1_nvenc"),("HEVC NVENC","hevc_nvenc"),("H.264 NVENC","h264_nvenc"),
("AV1 AMF (AMD)","av1_amf"),("HEVC AMF (AMD)","hevc_amf"),("H.264 AMF (AMD)","h264_amf"),
("AV1 QSV (Intel)","av1_qsv"),("HEVC QSV (Intel)","hevc_qsv"),("H.264 QSV (Intel)","h264_qsv"),
("AV1 (Software)","libsvtav1"),("HEVC (Software)","libx265"),("H.264 (Software)","libx264"),
]
ENC_VENDOR={"av1_nvenc":"NVIDIA","hevc_nvenc":"NVIDIA","h264_nvenc":"NVIDIA",
"av1_amf":"AMD","hevc_amf":"AMD","h264_amf":"AMD",
"av1_qsv":"Intel","hevc_qsv":"Intel","h264_qsv":"Intel"}
MODEL_VENDOR={"rife-cuda":"NVIDIA"}
PRESETS={
"nvenc":["p1","p2","p3","p4","p5","p6","p7"],
"amf":["speed","balanced","quality"],
"qsv":["veryfast","faster","fast","medium","slow","slower","veryslow"],
"lib":["ultrafast","superfast","veryfast","faster","fast","medium","slow","slower","veryslow"],
}
def preset_group(enc):
    if "nvenc" in enc:return "nvenc"
    if "amf" in enc:return "amf"
    if "qsv" in enc:return "qsv"
    return "lib"
APPDATA.mkdir(parents=True,exist_ok=True)
def load():
    try:return {**D,**json.loads(CFG.read_text())}
    except:return D.copy()
def save(c):CFG.write_text(json.dumps(c,indent=2))
def missing_runtime():
    return [k for k,p in CHECKS.items() if not p.exists()]
def detect_gpu():
    try:
        r=subprocess.run(["powershell","-NoProfile","-Command","Get-CimInstance Win32_VideoController | Select-Object -ExpandProperty Name"],capture_output=True,text=True,errors="replace",creationflags=subprocess.CREATE_NO_WINDOW,timeout=5)
        names=[n.strip() for n in r.stdout.splitlines() if n.strip()]
        if not names:return "Unknown",set()
        vendors=[];found=set()
        for n in names:
            u=n.upper()
            if "NVIDIA" in u:v="NVIDIA"
            elif "AMD" in u or "RADEON" in u:v="AMD"
            elif "INTEL" in u:v="Intel"
            else:v="Other"
            vendors.append(f"{v} ({n})");found.add(v)
        return " / ".join(vendors),found
    except Exception:
        return "Unknown",set()

class Job(QThread):
    log=Signal(str); done=Signal(bool,str)
    NOISY=("ffmpeg version","built with","configuration:","libavutil","libavcodec","libavformat","libavdevice","libavfilter","libswscale","libswresample","Metadata:","encoder ","handler_name","major_brand","minor_version","compatible_brands","Stream mapping:","Press [q]","Changing working dir","Added ","Duration:","Input #","Output #","Stream #")
    def clean(self,x):
        if any(n in x for n in self.NOISY):return None
        if ":\\" in x or "/" in x and "fps=" not in x and "=>" not in x:return None
        return x
    def __init__(self,c,inp,out,multi,scale):
        super().__init__();self.c=c;self.inp=inp;self.out=out;self.multi=multi;self.scale=scale;self.p=None;self.cancelled=False
    def stop(self):
        self.cancelled=True
        if self.p:
            try:self.p.terminate()
            except:pass
    def run_ncnn(self,c,srcdir,dstdir):
        cmd=[c["ncnn_exe"],"-i",str(srcdir),"-o",str(dstdir),"-m",c["ncnn_model"]]
        self.log.emit(" ".join(str(x) for x in [Path(cmd[0]).name]+cmd[1:]) if False else "")
        self.p=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors="replace",creationflags=subprocess.CREATE_NO_WINDOW,cwd=str(Path(c["ncnn_exe"]).parent))
        for x in self.p.stdout:
            cl=self.clean(x.rstrip())
            if cl:self.log.emit(cl)
        if self.p.wait()!=0:raise RuntimeError("RIFE NCNN failed. See log.")
    def run(self):
        work=None
        try:
            c=self.c
            engine=c.get("engine","rife-cuda")
            required=["ffmpeg"]
            required+=["python","rife_script","model"] if engine=="rife-cuda" else ["ncnn_exe","ncnn_model"]
            for k in required:
                if not Path(c[k]).exists():raise RuntimeError("Invalid path: "+k)
            chk=subprocess.run([c["ffmpeg"],"-hide_banner","-encoders"],capture_output=True,text=True,errors="replace",creationflags=subprocess.CREATE_NO_WINDOW)
            enc=c.get("encoder","av1_nvenc")
            if enc not in chk.stdout:raise RuntimeError(f"Selected FFmpeg has no {enc}.")
            work=ROOT/"work"; src=work/"input_frames"; dst=work/"rife_output"
            if work.exists():shutil.rmtree(work)
            src.mkdir(parents=True);dst.mkdir()
            self.log.emit("Extracting frames...")
            hwaccel=c.get("hwaccel","d3d11va")
            self.p=subprocess.Popen([c["ffmpeg"],"-y","-hwaccel",hwaccel,"-i",self.inp,"-fps_mode","passthrough","-compression_level","1",str(src/"%08d.png")],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors="replace",creationflags=subprocess.CREATE_NO_WINDOW)
            for x in self.p.stdout:
                cl=self.clean(x.rstrip())
                if cl:self.log.emit(cl)
            if self.p.wait()!=0:raise RuntimeError("FFmpeg extraction failed.")
            if engine=="rife-cuda":
                cmd=[c["python"],c["rife_script"],"--input",str(src),"--output",str(dst),"--model",c["model"],"--fp16","--multi",str(self.multi),"--scale",str(self.scale)]
                self.log.emit("Running RIFE CUDA...")
                self.p=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors="replace",creationflags=subprocess.CREATE_NO_WINDOW)
                for x in self.p.stdout:
                    cl=self.clean(x.rstrip())
                    if cl:self.log.emit(cl)
                if self.p.wait()!=0:raise RuntimeError("RIFE failed. See log.")
                final_dst=dst
            else:
                self.log.emit("Running RIFE NCNN...")
                self.run_ncnn(c,src,dst)
                final_dst=dst
                if self.multi==4:
                    pass2=work/"rife_output_pass2"
                    pass2.mkdir()
                    self.log.emit("Running RIFE NCNN (2nd pass for 4x)...")
                    self.run_ncnn(c,dst,pass2)
                    final_dst=pass2
            frames=sorted(final_dst.glob("*.png"))
            if not frames:raise RuntimeError("RIFE produced no PNG frames.")
            width=len(frames[0].stem); pattern=str(final_dst/("%0"+str(width)+"d.png"))
            self.log.emit(f"Encoding {enc}...")
            qflag="-crf" if enc.startswith("lib") else "-cq"
            cmd=[c["ffmpeg"],"-y","-framerate","60","-i",pattern,"-i",self.inp,"-map","0:v:0","-map","1:a?","-c:v",enc,"-preset",c["preset"],qflag,str(c["cq"]),"-pix_fmt","yuv420p","-c:a","copy",self.out]
            self.p=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors="replace",creationflags=subprocess.CREATE_NO_WINDOW)
            for x in self.p.stdout:
                cl=self.clean(x.rstrip())
                if cl:self.log.emit(cl)
            if self.p.wait()!=0:raise RuntimeError("Encoding failed.")
            shutil.rmtree(work)
            self.done.emit(True,self.out)
        except Exception as e:
            if self.cancelled:
                if work and work.exists():shutil.rmtree(work,ignore_errors=True)
                self.done.emit(False,"Cancelled.")
            else:
                self.done.emit(False,str(e))

class DownloadJob(QThread):
    progress=Signal(int); status=Signal(str); done=Signal(bool,str)
    def __init__(self,key,url):
        super().__init__();self.key=key;self.url=url
    def run(self):
        import tempfile
        tmp=Path(tempfile.gettempdir())/f"{self.key}.7z"
        last="Unknown error"
        for attempt in range(3):
            try:
                self.status.emit(f"Downloading {self.key}... (attempt {attempt+1}/3)")
                def hook(blocknum,blocksize,totalsize):
                    if totalsize>0:self.progress.emit(min(100,int(blocknum*blocksize*100/totalsize)))
                urllib.request.urlretrieve(self.url,str(tmp),reporthook=hook)
                self.status.emit(f"Extracting {self.key}...")
                RUNTIME.mkdir(parents=True,exist_ok=True)
                r=subprocess.run([str(SEVENZIP),"x",str(tmp),f"-o{RUNTIME}","-y"],capture_output=True,text=True,errors="replace",creationflags=subprocess.CREATE_NO_WINDOW)
                if r.returncode!=0:raise RuntimeError(r.stdout[-300:] if r.stdout else "7z extraction failed")
                self.done.emit(True,self.key);return
            except Exception as e:
                last=str(e);continue
            finally:
                if tmp.exists():
                    try:tmp.unlink()
                    except:pass
        self.done.emit(False,last)

class Main(QMainWindow):
    def __init__(self):
        super().__init__();self.c=load();self.job=None;self.show_setup=False;self.setWindowTitle("FrameMeld v1.2");self.resize(900,700);self.ui();self.update_model_desc();self.refresh_setup();self.refresh_preset();self.refresh_encoders()
    def pick(self,e,folder=False,filt="All files (*)"):
        p=QFileDialog.getExistingDirectory(self) if folder else QFileDialog.getOpenFileName(self,"Select","",filt)[0]
        if p:e.setText(p)
    def update_model_desc(self):
        key=self.modelpick.currentData()
        if key is None:self.modeldesc.setText("");return
        for k,lab,desc,avail in MODELS:
            if k==key:self.modeldesc.setText(desc)
    def refresh_encoders(self):
        for i in range(self.encoder.count()):
            val=self.encoder.itemData(i)
            if val is None:continue
            vendor=ENC_VENDOR.get(val)
            ok=(vendor is None) or (vendor in self.gpuvendors)
            self.encoder.model().item(i).setEnabled(ok)
    def refresh_preset(self):
        enc=self.encoder.currentData()
        if enc is None:
            self.preset.blockSignals(True);self.preset.clear();self.preset.blockSignals(False);return
        grp=preset_group(enc)
        self.preset.blockSignals(True)
        self.preset.clear();self.preset.addItems(PRESETS[grp])
        self.preset.blockSignals(False)
    def toggle_setup(self):
        self.show_setup=not self.show_setup
        self.refresh_setup()
    def refresh_setup(self):
        miss=missing_runtime()
        self.st_ffmpeg.setText("✓ Installed" if "ffmpeg" not in miss else "Not installed")
        self.bt_ffmpeg.setEnabled("ffmpeg" in miss)
        modelkey=self.modelpick.currentData()
        if modelkey is None:
            self.st_py.setText("—");self.bt_py.setEnabled(False)
            self.st_model.setText("Choose an engine first");self.bt_model.setEnabled(False)
            model_ok=False;py_ok=True
            self.enginegroup.setTitle("RIFE -> Encoder")
        else:
            py_needed=modelkey=="rife-cuda"
            if py_needed:
                self.st_py.setText("✓ Installed" if "py-amp" not in miss else "Not installed")
                self.bt_py.setEnabled("py-amp" in miss)
                py_ok="py-amp" not in miss
            else:
                self.st_py.setText("Not needed for this engine")
                self.bt_py.setEnabled(False)
                py_ok=True
            model_ok=CHECKS.get(modelkey,Path("nonexistent")).exists()
            self.st_model.setText("✓ Installed" if model_ok else "Not installed")
            self.bt_model.setEnabled(not model_ok)
            self.enginegroup.setTitle("RIFE CUDA -> NVENC" if modelkey=="rife-cuda" else "RIFE NCNN -> NVENC")
        enc_ok=self.encoder.currentData() is not None
        allgood=("ffmpeg" not in miss) and py_ok and model_ok and enc_ok
        self.setup.setVisible((not allgood) or self.show_setup)
        self.start.setEnabled(allgood)
    def download(self,key):
        self.bt_ffmpeg.setEnabled(False);self.bt_py.setEnabled(False);self.bt_model.setEnabled(False)
        self.dl=DownloadJob(key,URLS[key])
        self.dl.progress.connect(lambda p:self.dlprogress.setText(f"{p}%"))
        self.dl.status.connect(lambda s:self.dlprogress.setText(s))
        self.dl.done.connect(self.download_done)
        self.dl.start()
    def download_model(self):
        self.download(self.modelpick.currentData())
    def download_done(self,ok,msg):
        if not ok:QMessageBox.critical(self,"FrameMeld","Download failed:\n"+msg)
        self.dlprogress.setText("")
        self.refresh_setup()    
    def probe_input(self):
        p=self.inp.text()
        if not p or not Path(p).exists():return
        try:
            r=subprocess.run([self.c["ffprobe"],"-v","error","-select_streams","v:0","-show_entries","stream=width,height,r_frame_rate,nb_frames","-show_entries","format=duration","-of","json",p],capture_output=True,text=True,errors="replace",creationflags=subprocess.CREATE_NO_WINDOW)
            d=json.loads(r.stdout);s=d["streams"][0];dur=float(d["format"]["duration"])
            num,den=s["r_frame_rate"].split("/");fps=float(num)/float(den)
            frames=s.get("nb_frames") or str(int(dur*fps))
            h=int(dur//3600);m=int((dur%3600)//60);sec=int(dur%60)
            self.src_fps=fps
            self.info.setText(f"Size: {s['width']}x{s['height']} - Rate: {fps:.2f} FPS - Frames: {frames} - Duration: {h:02d}:{m:02d}:{sec:02d}")
        except Exception:self.src_fps=0;self.info.setText("")
    def ui(self):
        w=QWidget();self.setCentralWidget(w);L=QVBoxLayout(w);self.f={}
        gputext,self.gpuvendors=detect_gpu()
        top=QHBoxLayout();self.gpuinfo=QLabel(f"Detected GPU: {gputext}");top.addWidget(self.gpuinfo)
        self.togglesetup=QPushButton("Runtime Downloads");self.togglesetup.clicked.connect(self.toggle_setup);top.addWidget(self.togglesetup)
        L.addLayout(top)
        self.modelpick=QComboBox()
        self.modelpick.addItem("-- Please choose an engine --",None)
        self.modelpick.model().item(0).setEnabled(False)
        for key,label,desc,avail in MODELS:
            self.modelpick.addItem(label,key)
            req=MODEL_VENDOR.get(key)
            hw_ok=(req is None) or (req in self.gpuvendors)
            if not avail or not hw_ok:self.modelpick.model().item(self.modelpick.count()-1).setEnabled(False)
        self.modelpick.setCurrentIndex(0)
        self.modelpick.currentIndexChanged.connect(self.update_model_desc);self.modelpick.currentIndexChanged.connect(self.refresh_setup)
        self.setup=QGroupBox("Setup — Required Downloads");SU=QGridLayout(self.setup)
        self.st_ffmpeg=QLabel();self.bt_ffmpeg=QPushButton("Download");self.bt_ffmpeg.clicked.connect(lambda:self.download("ffmpeg"))
        self.st_py=QLabel();self.bt_py=QPushButton("Download");self.bt_py.clicked.connect(lambda:self.download("py-amp"))
        SU.addWidget(QLabel("FFmpeg (required):"),0,0);SU.addWidget(self.st_ffmpeg,0,1);SU.addWidget(self.bt_ffmpeg,0,2)
        SU.addWidget(QLabel("Python Runtime (required):"),1,0);SU.addWidget(self.st_py,1,1);SU.addWidget(self.bt_py,1,2)
        self.modeldesc=QLabel();self.modeldesc.setWordWrap(True)
        self.st_model=QLabel();self.bt_model=QPushButton("Download");self.bt_model.clicked.connect(self.download_model)
        SU.addWidget(QLabel("Selected AI Model:"),2,0);SU.addWidget(self.st_model,2,1);SU.addWidget(self.bt_model,2,2)
        SU.addWidget(self.modeldesc,3,0,1,3)
        self.dlprogress=QLabel("");SU.addWidget(self.dlprogress,4,0,1,3)
        L.addWidget(self.setup)
        io=QGroupBox("Video");I=QGridLayout(io);self.inp=QLineEdit();self.outdir=QLineEdit()
        bi=QPushButton("Input");bi.clicked.connect(lambda:(self.pick(self.inp,False,"Video (*.mkv *.mp4 *.mov *.webm);;All files (*)"),self.probe_input()))
        bo=QPushButton("Output Folder");bo.clicked.connect(lambda:self.pick(self.outdir,True))
        self.fmt=QComboBox();self.fmt.addItems([".mkv",".mp4",".mov",".webm"])
        I.addWidget(QLabel("Input:"),0,0);I.addWidget(self.inp,0,1);I.addWidget(bi,0,2);I.addWidget(QLabel("Output Folder:"),1,0);I.addWidget(self.outdir,1,1);I.addWidget(bo,1,2);I.addWidget(QLabel("Format:"),2,0);I.addWidget(self.fmt,2,1);self.info=QLabel("");I.addWidget(self.info,3,0,1,3);L.addWidget(io)
        self.enginegroup=QGroupBox("RIFE -> NVENC");S=QGridLayout(self.enginegroup);self.multi=QComboBox();self.multi.addItems(["2x","4x"]);self.scale=QComboBox();self.scale.addItems(["0.25","0.5","1.0","2.0","4.0"]);self.scale.setCurrentText("1.0");self.preset=QComboBox();self.cq=QSpinBox();self.cq.setRange(0,51);self.cq.setValue(int(self.c["cq"]));self.encoder=QComboBox();self.encoder.addItem("-- Please choose an encoder --",None);self.encoder.model().item(0).setEnabled(False);[self.encoder.addItem(lab,val) for lab,val in ENCODERS];self.encoder.setCurrentIndex(0);self.encoder.currentIndexChanged.connect(self.refresh_preset);self.encoder.currentIndexChanged.connect(self.refresh_setup)
        S.addWidget(QLabel("Engine:"),0,0);S.addWidget(self.modelpick,0,1);S.addWidget(QLabel("Multiplier:"),0,2);S.addWidget(self.multi,0,3);S.addWidget(QLabel("Scale:"),1,0);S.addWidget(self.scale,1,1);S.addWidget(QLabel("Encoder:"),1,2);S.addWidget(self.encoder,1,3);S.addWidget(QLabel("Preset:"),2,2);S.addWidget(self.preset,2,3);S.addWidget(QLabel("CQ:"),3,2);S.addWidget(self.cq,3,3);L.addWidget(self.enginegroup)
        row=QHBoxLayout();self.start=QPushButton("START");self.start.clicked.connect(self.startjob);self.cancel=QPushButton("CANCEL");self.cancel.clicked.connect(lambda:self.job and self.job.stop());self.cancel.setEnabled(False);row.addWidget(self.start);row.addWidget(self.cancel);L.addLayout(row);self.log=QTextEdit();self.log.setReadOnly(True);L.addWidget(self.log)
    def startjob(self):
        for k,e in self.f.items():self.c[k]=e.text()
        engine=self.modelpick.currentData();enc=self.encoder.currentData()
        if engine is None or enc is None:return QMessageBox.warning(self,"FrameMeld","Choose an engine and an encoder first.")
        self.c["preset"]=self.preset.currentText();self.c["cq"]=self.cq.value();self.c["encoder"]=enc;self.c["engine"]=engine
        self.c["hwaccel"]="cuda" if "NVIDIA" in self.gpuvendors else "d3d11va"
        save(self.c)
        if not Path(self.inp.text()).exists():return QMessageBox.warning(self,"FrameMeld","Choose a valid input video.")
        if not self.outdir.text() or not Path(self.outdir.text()).exists():return QMessageBox.warning(self,"FrameMeld","Choose a valid output folder.")
        mult=2 if self.multi.currentText()=="2x" else 4
        fps=round(getattr(self,"src_fps",0)*mult)
        stem=Path(self.inp.text()).stem
        outpath=str(Path(self.outdir.text())/f"{stem} {fps}fps{self.fmt.currentText()}")
        self.start.setEnabled(False);self.cancel.setEnabled(True);self.log.clear()
        self.job=Job(self.c,self.inp.text(),outpath,mult,float(self.scale.currentText()));self.job.log.connect(self.log.append);self.job.done.connect(self.finish);self.job.start()
    def finish(self,ok,msg):
        self.start.setEnabled(True);self.cancel.setEnabled(False);(QMessageBox.information if ok else QMessageBox.critical)(self,"FrameMeld",("Done:\n" if ok else "Failed:\n")+msg)

if __name__=="__main__":
    a=QApplication(sys.argv);a.setStyle("Fusion");m=Main();m.show();sys.exit(a.exec())
