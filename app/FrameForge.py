import sys,json,subprocess,shutil,os,urllib.request
from pathlib import Path
from PySide6.QtCore import QThread,Signal
from PySide6.QtWidgets import QApplication,QMainWindow,QWidget,QVBoxLayout,QGridLayout,QHBoxLayout,QLabel,QPushButton,QLineEdit,QFileDialog,QComboBox,QSpinBox,QTextEdit,QMessageBox,QGroupBox

ROOT=Path(sys.executable).resolve().parent if getattr(sys,"frozen",False) else Path(__file__).resolve().parent.parent
APPDATA=Path(os.environ.get("LOCALAPPDATA",str(Path.home())))/"FrameForge"
RUNTIME=APPDATA/"runtime"
CFG=APPDATA/"config.json"
SEVENZIP=ROOT/"tools"/"7za.exe"
URLS={"ffmpeg":"https://huggingface.co/Luxbane/frameforge-runtime/resolve/main/ffmpeg.7z",
"py-amp":"https://huggingface.co/Luxbane/frameforge-runtime/resolve/main/py-amp.7z",
"rife-cuda":"https://huggingface.co/Luxbane/frameforge-runtime/resolve/main/rife-cuda.7z"}
CHECKS={"ffmpeg":RUNTIME/"ffmpeg"/"ffmpeg.exe","py-amp":RUNTIME/"py-amp"/"python.exe","rife-cuda":RUNTIME/"rife-cuda"/"rife.py"}
MODELS=[
("rife-cuda","RIFE CUDA","NVIDIA GPUs (CUDA). Fastest option — recommended if you have an RTX/GTX card.",True),
("rife-ncnn","RIFE NCNN","AMD / Intel / NVIDIA (Vulkan). Broader GPU compatibility. Coming soon.",False),
]
D={"ffmpeg":str(RUNTIME/"ffmpeg"/"ffmpeg.exe"),
"ffprobe":str(RUNTIME/"ffmpeg"/"ffprobe.exe"),
"python":str(RUNTIME/"py-amp"/"python.exe"),
"rife_script":str(RUNTIME/"rife-cuda"/"rife.py"),
"model":str(RUNTIME/"rife-cuda"/"RIFE40"),
"preset":"p5","cq":28,"encoder":"av1_nvenc"}
ENCODERS=[("AV1 NVENC","av1_nvenc"),("HEVC NVENC","hevc_nvenc"),("H.264 NVENC","h264_nvenc")]
APPDATA.mkdir(parents=True,exist_ok=True)
def load():
    try:return {**D,**json.loads(CFG.read_text())}
    except:return D.copy()
def save(c):CFG.write_text(json.dumps(c,indent=2))
def missing_runtime():
    return [k for k,p in CHECKS.items() if not p.exists()]

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
    def run(self):
        try:
            c=self.c
            for k in ("ffmpeg","python","rife_script","model"):
                if not Path(c[k]).exists():raise RuntimeError("Invalid path: "+k)
            chk=subprocess.run([c["ffmpeg"],"-hide_banner","-encoders"],capture_output=True,text=True,errors="replace",creationflags=subprocess.CREATE_NO_WINDOW)
            enc=c.get("encoder","av1_nvenc")
            if enc not in chk.stdout:raise RuntimeError(f"Selected FFmpeg has no {enc}.")
            work=ROOT/"work"; src=work/"input_frames"; dst=work/"rife_output"
            if work.exists():shutil.rmtree(work)
            src.mkdir(parents=True);dst.mkdir()
            self.log.emit("Extracting frames...")
            self.p=subprocess.Popen([c["ffmpeg"],"-y","-i",self.inp,"-fps_mode","passthrough",str(src/"%08d.png")],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors="replace",creationflags=subprocess.CREATE_NO_WINDOW)
            for x in self.p.stdout:
                cl=self.clean(x.rstrip())
                if cl:self.log.emit(cl)
            if self.p.wait()!=0:raise RuntimeError("FFmpeg extraction failed.")
            # CLI matches the uploaded Flowframes rife.py.
            cmd=[c["python"],c["rife_script"],"--input",str(src),"--output",str(dst),"--model",c["model"],"--fp16","--multi",str(self.multi),"--scale",str(self.scale)]
            self.log.emit("Running RIFE CUDA...")
            self.p=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors="replace",creationflags=subprocess.CREATE_NO_WINDOW)
            for x in self.p.stdout:
                cl=self.clean(x.rstrip())
                if cl:self.log.emit(cl)
            if self.p.wait()!=0:raise RuntimeError("RIFE failed. See log.")
            frames=sorted(dst.glob("*.png"))
            if not frames:raise RuntimeError("RIFE produced no PNG frames.")
            width=len(frames[0].stem); pattern=str(dst/("%0"+str(width)+"d.png"))
            self.log.emit(f"Encoding {enc}...")
            cmd=[c["ffmpeg"],"-y","-framerate","60","-i",pattern,"-i",self.inp,"-map","0:v:0","-map","1:a?","-c:v",enc,"-preset",c["preset"],"-cq",str(c["cq"]),"-pix_fmt","yuv420p","-c:a","copy",self.out]
            self.p=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors="replace",creationflags=subprocess.CREATE_NO_WINDOW)
            for x in self.p.stdout:
                cl=self.clean(x.rstrip())
                if cl:self.log.emit(cl)
            if self.p.wait()!=0:raise RuntimeError("AV1 NVENC failed.")
            shutil.rmtree(work)
            self.done.emit(True,self.out)
        except Exception as e:
            if self.cancelled:
                if work.exists():shutil.rmtree(work,ignore_errors=True)
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
        super().__init__();self.c=load();self.job=None;self.setWindowTitle("FrameForge v1.0");self.resize(900,700);self.ui();self.update_model_desc();self.refresh_setup()
    def pick(self,e,folder=False,filt="All files (*)"):
        p=QFileDialog.getExistingDirectory(self) if folder else QFileDialog.getOpenFileName(self,"Select","",filt)[0]
        if p:e.setText(p)
    def update_model_desc(self):
        key=self.modelpick.currentData()
        for k,lab,desc,avail in MODELS:
            if k==key:self.modeldesc.setText(desc)
    def refresh_setup(self):
        miss=missing_runtime()
        self.st_ffmpeg.setText("✓ Installed" if "ffmpeg" not in miss else "Not installed")
        self.bt_ffmpeg.setEnabled("ffmpeg" in miss)
        self.st_py.setText("✓ Installed" if "py-amp" not in miss else "Not installed")
        self.bt_py.setEnabled("py-amp" in miss)
        modelkey=self.modelpick.currentData()
        model_ok=CHECKS.get(modelkey,Path("nonexistent")).exists()
        self.st_model.setText("✓ Installed" if model_ok else "Not installed")
        self.bt_model.setEnabled(not model_ok)
        allgood=("ffmpeg" not in miss) and ("py-amp" not in miss) and model_ok
        self.setup.setVisible(not allgood)
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
        if not ok:QMessageBox.critical(self,"FrameForge","Download failed:\n"+msg)
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
        self.setup=QGroupBox("Setup — Required Downloads");SU=QGridLayout(self.setup)
        self.st_ffmpeg=QLabel();self.bt_ffmpeg=QPushButton("Download");self.bt_ffmpeg.clicked.connect(lambda:self.download("ffmpeg"))
        self.st_py=QLabel();self.bt_py=QPushButton("Download");self.bt_py.clicked.connect(lambda:self.download("py-amp"))
        SU.addWidget(QLabel("FFmpeg (required):"),0,0);SU.addWidget(self.st_ffmpeg,0,1);SU.addWidget(self.bt_ffmpeg,0,2)
        SU.addWidget(QLabel("Python Runtime (required):"),1,0);SU.addWidget(self.st_py,1,1);SU.addWidget(self.bt_py,1,2)
        self.modelpick=QComboBox()
        for key,label,desc,avail in MODELS:
            self.modelpick.addItem(label,key)
            if not avail:self.modelpick.model().item(self.modelpick.count()-1).setEnabled(False)
        self.modeldesc=QLabel();self.modeldesc.setWordWrap(True)
        self.modelpick.currentIndexChanged.connect(self.update_model_desc)
        self.st_model=QLabel();self.bt_model=QPushButton("Download");self.bt_model.clicked.connect(self.download_model)
        SU.addWidget(QLabel("AI Model (required):"),2,0);SU.addWidget(self.modelpick,2,1);SU.addWidget(self.bt_model,2,2)
        SU.addWidget(self.modeldesc,3,0,1,3)
        SU.addWidget(self.st_model,4,0,1,3)
        self.dlprogress=QLabel("");SU.addWidget(self.dlprogress,5,0,1,3)
        L.addWidget(self.setup)
        io=QGroupBox("Video");I=QGridLayout(io);self.inp=QLineEdit();self.outdir=QLineEdit()
        bi=QPushButton("Input");bi.clicked.connect(lambda:(self.pick(self.inp,False,"Video (*.mkv *.mp4 *.mov *.webm);;All files (*)"),self.probe_input()))
        bo=QPushButton("Output Folder");bo.clicked.connect(lambda:self.pick(self.outdir,True))
        self.fmt=QComboBox();self.fmt.addItems([".mkv",".mp4",".mov",".webm"])
        I.addWidget(QLabel("Input:"),0,0);I.addWidget(self.inp,0,1);I.addWidget(bi,0,2);I.addWidget(QLabel("Output Folder:"),1,0);I.addWidget(self.outdir,1,1);I.addWidget(bo,1,2);I.addWidget(QLabel("Format:"),2,0);I.addWidget(self.fmt,2,1);self.info=QLabel("");I.addWidget(self.info,3,0,1,3);L.addWidget(io)
        s=QGroupBox("RIFE CUDA -> NVENC");S=QGridLayout(s);self.multi=QComboBox();self.multi.addItems(["2x","4x"]);self.scale=QComboBox();self.scale.addItems(["0.25","0.5","1.0","2.0","4.0"]);self.scale.setCurrentText("1.0");self.preset=QComboBox();self.preset.addItems(["p1","p2","p3","p4","p5","p6","p7"]);self.preset.setCurrentText(self.c["preset"]);self.cq=QSpinBox();self.cq.setRange(0,51);self.cq.setValue(int(self.c["cq"]));self.encoder=QComboBox();[self.encoder.addItem(lab,val) for lab,val in ENCODERS];idx=self.encoder.findData(self.c.get("encoder","av1_nvenc"));self.encoder.setCurrentIndex(idx if idx>=0 else 0)
        S.addWidget(QLabel("Engine:"),0,0);S.addWidget(QLabel("RIFE CUDA / FP16"),0,1);S.addWidget(QLabel("Multiplier:"),0,2);S.addWidget(self.multi,0,3);S.addWidget(QLabel("Scale:"),1,0);S.addWidget(self.scale,1,1);S.addWidget(QLabel("Encoder:"),1,2);S.addWidget(self.encoder,1,3);S.addWidget(QLabel("Preset:"),2,2);S.addWidget(self.preset,2,3);S.addWidget(QLabel("CQ:"),3,2);S.addWidget(self.cq,3,3);L.addWidget(s)
        row=QHBoxLayout();self.start=QPushButton("START");self.start.clicked.connect(self.startjob);self.cancel=QPushButton("CANCEL");self.cancel.clicked.connect(lambda:self.job and self.job.stop());self.cancel.setEnabled(False);row.addWidget(self.start);row.addWidget(self.cancel);L.addLayout(row);self.log=QTextEdit();self.log.setReadOnly(True);L.addWidget(self.log)
    def startjob(self):
        for k,e in self.f.items():self.c[k]=e.text()
        self.c["preset"]=self.preset.currentText();self.c["cq"]=self.cq.value();self.c["encoder"]=self.encoder.currentData();save(self.c)
        if not Path(self.inp.text()).exists():return QMessageBox.warning(self,"FrameForge","Choose a valid input video.")
        if not self.outdir.text() or not Path(self.outdir.text()).exists():return QMessageBox.warning(self,"FrameForge","Choose a valid output folder.")
        mult=2 if self.multi.currentText()=="2x" else 4
        fps=round(getattr(self,"src_fps",0)*mult)
        stem=Path(self.inp.text()).stem
        outpath=str(Path(self.outdir.text())/f"{stem} {fps}fps{self.fmt.currentText()}")
        self.start.setEnabled(False);self.cancel.setEnabled(True);self.log.clear()
        self.job=Job(self.c,self.inp.text(),outpath,mult,float(self.scale.currentText()));self.job.log.connect(self.log.append);self.job.done.connect(self.finish);self.job.start()
    def finish(self,ok,msg):
        self.start.setEnabled(True);self.cancel.setEnabled(False);(QMessageBox.information if ok else QMessageBox.critical)(self,"FrameForge",("Done:\n" if ok else "Failed:\n")+msg)

if __name__=="__main__":
    a=QApplication(sys.argv);a.setStyle("Fusion");m=Main();m.show();sys.exit(a.exec())
