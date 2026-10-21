#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <dxgi.h>
#include <urlmon.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <uxtheme.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <thread>
#include <atomic>
#include <mutex>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include <cmath>

namespace fs = std::filesystem;

// Control IDs
enum ControlID {
    ID_GPU_INFO = 1001,
    ID_BTN_TOGGLE_SETUP,
    ID_BTN_THEME,
    
    // Setup group
    ID_GRP_SETUP,
    ID_LBL_FFMPEG, ID_ST_FFMPEG, ID_BTN_FFMPEG,
    ID_LBL_PY, ID_ST_PY, ID_BTN_PY,
    ID_LBL_MODEL, ID_ST_MODEL, ID_BTN_MODEL,
    ID_LBL_MODEL_DESC,
    ID_LBL_DL_PROGRESS,
    
    // Video group
    ID_GRP_VIDEO,
    ID_LBL_INPUT, ID_EDT_INPUT, ID_BTN_INPUT,
    ID_LBL_OUTPUT, ID_EDT_OUTPUT, ID_BTN_OUTPUT,
    ID_LBL_FORMAT, ID_CMB_FORMAT,
    ID_LBL_INFO,
    
    // Engine & Encoder group
    ID_GRP_ENGINE,
    ID_LBL_ENGINE, ID_CMB_ENGINE,
    ID_LBL_MULTI, ID_CMB_MULTI,
    ID_LBL_SCALE, ID_CMB_SCALE,
    ID_LBL_ENCODER, ID_CMB_ENCODER,
    ID_LBL_PRESET, ID_CMB_PRESET,
    ID_LBL_CQ, ID_EDT_CQ, ID_SPN_CQ,
    
    // Actions
    ID_BTN_START,
    ID_BTN_CANCEL,
    ID_PROGRESS_BAR,
    ID_LBL_JOB_PROGRESS,
    ID_EDT_LOG,

    // Custom messages
    WM_APP_LOG = WM_APP + 1,
    WM_APP_JOB_PROGRESS,
    WM_APP_JOB_DONE,
    WM_APP_DL_PROGRESS,
    WM_APP_DL_DONE
};

struct ModelEntry {
    std::string key;
    std::wstring label;
    std::wstring desc;
    std::string vendorReq; // "NVIDIA" or ""
};

const std::vector<ModelEntry> MODELS = {
    {"rife-cuda-opt", L"RIFE CUDA (Optimized / Low CPU) [PRO]", L"PRO Exclusive \u2014 Direct in-memory streaming pipe, zero disk temp files & low CPU temperature.", "NVIDIA"},
    {"rife-cuda", L"RIFE CUDA", L"NVIDIA GPUs (CUDA). Fastest option \u2014 recommended if you have an RTX/GTX card.", "NVIDIA"},
    {"rife-ncnn-opt", L"RIFE NCNN (Optimized / Low CPU) [PRO]", L"PRO Exclusive \u2014 Fast low-temp buffer, multi-GPU targeting & zero CPU boost overheating.", ""},
    {"rife-ncnn", L"RIFE NCNN", L"AMD / Intel / NVIDIA (Vulkan). Broader GPU compatibility, no CUDA/PyTorch needed. Note: native 2x only \u2014 4x runs two passes.", ""}
};

struct EncoderEntry {
    std::wstring label;
    std::string id;
    std::string vendor; // "NVIDIA", "AMD", "Intel", or ""
};

const std::vector<EncoderEntry> ENCODERS = {
    {L"AV1 NVENC", "av1_nvenc", "NVIDIA"},
    {L"HEVC NVENC", "hevc_nvenc", "NVIDIA"},
    {L"H.264 NVENC", "h264_nvenc", "NVIDIA"},
    {L"AV1 AMF (AMD)", "av1_amf", "AMD"},
    {L"HEVC AMF (AMD)", "hevc_amf", "AMD"},
    {L"H.264 AMF (AMD)", "h264_amf", "AMD"},
    {L"AV1 QSV (Intel)", "av1_qsv", "Intel"},
    {L"HEVC QSV (Intel)", "hevc_qsv", "Intel"},
    {L"H.264 QSV (Intel)", "h264_qsv", "Intel"},
    {L"AV1 (Software)", "libsvtav1", ""},
    {L"HEVC (Software)", "libx265", ""},
    {L"H.264 (Software)", "libx264", ""}
};

const std::map<std::string, std::vector<std::string>> PRESETS = {
    {"nvenc", {"p1", "p2", "p3", "p4", "p5", "p6", "p7"}},
    {"amf", {"speed", "balanced", "quality"}},
    {"qsv", {"veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow"}},
    {"lib", {"ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow"}}
};

const std::map<std::string, std::wstring> URLS = {
    {"ffmpeg", L"https://huggingface.co/Luxbane/FrameMeld/resolve/main/ffmpeg.7z"},
    {"py-amp", L"https://huggingface.co/Luxbane/FrameMeld/resolve/main/py-amp.7z"},
    {"rife-cuda", L"https://huggingface.co/Luxbane/FrameMeld/resolve/main/rife-cuda.7z"},
    {"rife-ncnn", L"https://huggingface.co/Luxbane/FrameMeld/resolve/main/rife-ncnn.7z"}
};

// Global App State
struct AppState {
    HINSTANCE hInstance = NULL;
    HWND hWnd = NULL;
    HFONT hFont = NULL;
    HFONT hFontBold = NULL;
    
    // UI Handles
    HWND hGpuInfo = NULL;
    HWND hBtnTheme = NULL;
    HWND hBtnToggleSetup = NULL;
    
    HWND hGrpSetup = NULL;
    HWND hLblFFmpeg = NULL, hStFFmpeg = NULL, hBtnFFmpeg = NULL;
    HWND hLblPy = NULL, hStPy = NULL, hBtnPy = NULL;
    HWND hLblModel = NULL, hStModel = NULL, hBtnModel = NULL;
    HWND hLblModelDesc = NULL;
    HWND hLblDlProgress = NULL;
    
    HWND hGrpVideo = NULL;
    HWND hLblInput = NULL, hEdtInput = NULL, hBtnInput = NULL;
    HWND hLblOutput = NULL, hEdtOutput = NULL, hBtnOutput = NULL;
    HWND hLblFormat = NULL, hCmbFormat = NULL;
    HWND hLblInfo = NULL;
    
    HWND hGrpEngine = NULL;
    HWND hLblEngine = NULL, hCmbEngine = NULL;
    HWND hLblMulti = NULL, hCmbMulti = NULL;
    HWND hLblScale = NULL, hCmbScale = NULL;
    HWND hLblEncoder = NULL, hCmbEncoder = NULL;
    HWND hLblPreset = NULL, hCmbPreset = NULL;
    HWND hLblCq = NULL, hEdtCq = NULL, hSpnCq = NULL;
    
    HWND hBtnStart = NULL;
    HWND hBtnCancel = NULL;
    HWND hProgressBar = NULL;
    HWND hLblJobProgress = NULL;
    HWND hEdtLog = NULL;
    
    // Paths
    fs::path rootPath;
    fs::path appDataPath;
    fs::path runtimePath;
    fs::path cfgPath;
    fs::path sevenZipPath;
    
    // Hardware info
    std::wstring gpuText;
    std::set<std::string> gpuVendors;
    
    // Configuration values
    std::map<std::string, std::string> config;
    
    // State flags
    bool showSetup = false;
    bool isRestoring = false;
    bool isDarkMode = false;
    HBRUSH hDarkBgBrush = NULL;
    HBRUSH hDarkEditBrush = NULL;
    HBRUSH hDarkLogBrush = NULL;
    double srcFps = 0.0;
    long long totalSrcFrames = 0;
    long long totalTargetFrames = 0;
    int lastValidEngineIdx = 0;
    int lastValidEncoderIdx = 0;
    
    // Process control
    std::atomic<bool> jobRunning{false};
    std::atomic<bool> jobCancelled{false};
    PROCESS_INFORMATION activeProcess{};
    std::mutex procMutex;
    
    std::atomic<bool> dlRunning{false};
};

static AppState g_app;

// Forward declarations
void update_layout(bool showSetup);
void refresh_setup();
void refresh_presets();
void refresh_encoders();
void update_model_desc();
void save_settings();
void restore_settings();
void probe_input();
void on_input_file_selected(const std::wstring& filePath);
void start_job();
void cancel_job();
void start_download(const std::string& key);
void apply_theme(bool isDark);
bool get_system_dark_mode();

// Helper string conversions
std::wstring s2ws(const std::string& str) {
    if (str.empty()) return L"";
    int sz = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring res(sz - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &res[0], sz);
    return res;
}

std::string ws2s(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int sz = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string res(sz - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &res[0], sz, NULL, NULL);
    return res;
}

std::string get_preset_group(const std::string& enc) {
    if (enc.find("nvenc") != std::string::npos) return "nvenc";
    if (enc.find("amf") != std::string::npos) return "amf";
    if (enc.find("qsv") != std::string::npos) return "qsv";
    return "lib";
}

// Simple JSON Parser/Writer for config.json
void load_config() {
    g_app.config["ffmpeg"] = (g_app.runtimePath / "ffmpeg" / "ffmpeg.exe").string();
    g_app.config["ffprobe"] = (g_app.runtimePath / "ffmpeg" / "ffprobe.exe").string();
    g_app.config["python"] = (g_app.runtimePath / "py-amp" / "python.exe").string();
    g_app.config["rife_script"] = (g_app.runtimePath / "rife-cuda" / "rife.py").string();
    g_app.config["model"] = (g_app.runtimePath / "rife-cuda" / "RIFE40").string();
    g_app.config["ncnn_exe"] = (g_app.runtimePath / "rife-ncnn" / "rife-ncnn-vulkan.exe").string();
    g_app.config["ncnn_model"] = (g_app.runtimePath / "rife-ncnn" / "rife-v3.1").string();
    g_app.config["engine"] = "rife-cuda";
    g_app.config["preset"] = "p5";
    g_app.config["cq"] = "28";
    g_app.config["encoder"] = "av1_nvenc";
    
    if (!fs::exists(g_app.cfgPath)) return;
    
    std::ifstream file(g_app.cfgPath);
    if (!file.is_open()) return;
    
    std::string line;
    while (std::getline(file, line)) {
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        
        size_t k1 = line.find('"');
        size_t k2 = line.find('"', k1 + 1);
        if (k1 == std::string::npos || k2 == std::string::npos || k2 >= colon) continue;
        std::string key = line.substr(k1 + 1, k2 - k1 - 1);
        
        std::string valStr = line.substr(colon + 1);
        size_t v1 = valStr.find('"');
        if (v1 != std::string::npos) {
            size_t v2 = valStr.find('"', v1 + 1);
            if (v2 != std::string::npos) {
                g_app.config[key] = valStr.substr(v1 + 1, v2 - v1 - 1);
                continue;
            }
        }
        
        // Number or boolean value
        size_t numStart = valStr.find_first_not_of(" \t,");
        size_t numEnd = valStr.find_last_not_of(" \t,\r\n");
        if (numStart != std::string::npos && numEnd != std::string::npos && numEnd >= numStart) {
            g_app.config[key] = valStr.substr(numStart, numEnd - numStart + 1);
        }
    }
}

void save_config() {
    try {
        fs::create_directories(g_app.appDataPath);
        std::ofstream file(g_app.cfgPath);
        if (!file.is_open()) return;
        
        file << "{\n";
        size_t count = 0;
        for (const auto& [k, v] : g_app.config) {
            file << "  \"" << k << "\": ";
            if (k == "cq") {
                file << v;
            } else {
                file << "\"" << v << "\"";
            }
            if (++count < g_app.config.size()) file << ",";
            file << "\n";
        }
        file << "}\n";
    } catch (...) {}
}

// DXGI GPU Detection
void detect_gpu() {
    std::vector<std::wstring> names;
    g_app.gpuVendors.clear();
    
    IDXGIFactory* pFactory = nullptr;
    if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory))) {
        IDXGIAdapter* pAdapter = nullptr;
        for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC desc;
            pAdapter->GetDesc(&desc);
            pAdapter->Release();
            
            // Skip Microsoft Basic Render Driver
            if (desc.VendorId == 0x1414 || wcsstr(desc.Description, L"Microsoft") != nullptr) {
                continue;
            }
            
            std::string vendor = "Other";
            if (desc.VendorId == 0x10de || wcsstr(desc.Description, L"NVIDIA") != nullptr) {
                vendor = "NVIDIA";
            } else if (desc.VendorId == 0x1002 || wcsstr(desc.Description, L"AMD") != nullptr || wcsstr(desc.Description, L"Radeon") != nullptr) {
                vendor = "AMD";
            } else if (desc.VendorId == 0x8086 || wcsstr(desc.Description, L"Intel") != nullptr) {
                vendor = "Intel";
            }
            
            g_app.gpuVendors.insert(vendor);
            std::wstring descStr = desc.Description;
            std::wstring vendorW = s2ws(vendor);
            if (descStr.find(vendorW) != std::wstring::npos) {
                names.push_back(descStr);
            } else {
                names.push_back(vendorW + L" (" + descStr + L")");
            }
        }
        pFactory->Release();
    }
    
    if (names.empty()) {
        g_app.gpuText = L"Unknown";
    } else {
        std::wstring res;
        for (size_t i = 0; i < names.size(); ++i) {
            if (i > 0) res += L" / ";
            res += names[i];
        }
        g_app.gpuText = res;
    }
}

// Download status callback
class DownloadProgressCallback : public IBindStatusCallback {
public:
    ULONG m_ref = 1;
    HWND m_hWnd;
    std::string m_key;
    
    DownloadProgressCallback(HWND hWnd, const std::string& key) : m_hWnd(hWnd), m_key(key) {}

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown || riid == IID_IBindStatusCallback) {
            *ppv = this;
            AddRef();
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return ++m_ref; }
    STDMETHODIMP_(ULONG) Release() override {
        if (--m_ref == 0) { delete this; return 0; }
        return m_ref;
    }
    STDMETHODIMP OnStartBinding(DWORD, IBinding*) override { return S_OK; }
    STDMETHODIMP GetPriority(LONG*) override { return S_OK; }
    STDMETHODIMP OnLowResource(DWORD) override { return S_OK; }
    STDMETHODIMP OnStopBinding(HRESULT, LPCWSTR) override { return S_OK; }
    STDMETHODIMP GetBindInfo(DWORD*, BINDINFO*) override { return S_OK; }
    STDMETHODIMP OnDataAvailable(DWORD, DWORD, FORMATETC*, STGMEDIUM*) override { return S_OK; }
    STDMETHODIMP OnObjectAvailable(REFIID, IUnknown*) override { return S_OK; }
    STDMETHODIMP OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG, LPCWSTR) override {
        if (ulProgressMax > 0) {
            int pct = static_cast<int>((static_cast<double>(ulProgress) / ulProgressMax) * 100.0);
            if (pct > 100) pct = 100;
            PostMessage(m_hWnd, WM_APP_DL_PROGRESS, (WPARAM)pct, 0);
        }
        return S_OK;
    }
};

void log_text(const std::wstring& str) {
    if (!g_app.hWnd) return;
    std::wstring* msg = new std::wstring(str);
    PostMessage(g_app.hWnd, WM_APP_LOG, (WPARAM)msg, 0);
}

// Log line cleaner from Python NOISY filter
bool is_clean_log(const std::string& x) {
    static const char* NOISY[] = {
        "ffmpeg version", "built with", "configuration:", "libavutil", "libavcodec",
        "libavformat", "libavdevice", "libavfilter", "libswscale", "libswresample",
        "Metadata:", "encoder ", "handler_name", "major_brand", "minor_version",
        "compatible_brands", "Stream mapping:", "Press [q]", "Changing working dir",
        "Added ", "Duration:", "Input #", "Output #"
    };
    for (const char* n : NOISY) {
        if (x.find(n) != std::string::npos) return false;
    }
    if ((x.find(":\\") != std::string::npos || x.find("/") != std::string::npos)
        && x.find("fps=") == std::string::npos && x.find("=>") == std::string::npos) {
        return false;
    }
    return true;
}

void parse_job_progress(const std::string& line) {
    if (g_app.totalTargetFrames <= 0) return;
    
    // Check CUDA RIFE frame output: [T0] => 00000956.png
    size_t arrowPos = line.find("=> ");
    if (arrowPos != std::string::npos) {
        size_t numStart = arrowPos + 3;
        while (numStart < line.size() && line[numStart] == ' ') numStart++;
        long long curFrame = 0;
        try { curFrame = std::stoll(line.substr(numStart)); } catch (...) {}
        if (curFrame > 0) {
            double ratio = (double)curFrame / (double)g_app.totalTargetFrames;
            if (ratio > 1.0) ratio = 1.0;
            int pct = 5 + static_cast<int>(ratio * 85.0);
            int displayPct = static_cast<int>(ratio * 100.0);
            std::wstring* st = new std::wstring(L"Interpolating: " + std::to_wstring(displayPct) + L"%");
            PostMessage(g_app.hWnd, WM_APP_JOB_PROGRESS, pct, (LPARAM)st);
            return;
        }
    }
    
    // Check NCNN RIFE percentage
    size_t pctPos = line.find('%');
    if (pctPos != std::string::npos && line.find("=>") == std::string::npos && line.find("fps=") == std::string::npos) {
        size_t numStart = pctPos;
        while (numStart > 0 && (isdigit(line[numStart - 1]) || line[numStart - 1] == '.')) {
            numStart--;
        }
        if (numStart < pctPos) {
            try {
                double ncnnPct = std::stod(line.substr(numStart, pctPos - numStart));
                double ratio = ncnnPct / 100.0;
                if (ratio > 1.0) ratio = 1.0;
                int pct = 5 + static_cast<int>(ratio * 85.0);
                std::wstring* st = new std::wstring(L"Interpolating: " + std::to_wstring((int)ncnnPct) + L"%");
                PostMessage(g_app.hWnd, WM_APP_JOB_PROGRESS, pct, (LPARAM)st);
                return;
            } catch (...) {}
        }
    }
    
    // Check FFmpeg encoding progress: frame= 1234
    size_t frPos = line.find("frame=");
    if (frPos != std::string::npos) {
        size_t numStart = frPos + 6;
        while (numStart < line.size() && line[numStart] == ' ') numStart++;
        long long encFrame = 0;
        try { encFrame = std::stoll(line.substr(numStart)); } catch (...) {}
        if (encFrame > 0) {
            double ratio = (double)encFrame / (double)g_app.totalTargetFrames;
            if (ratio > 1.0) ratio = 1.0;
            int pct = 90 + static_cast<int>(ratio * 9.0);
            int displayPct = static_cast<int>(ratio * 100.0);
            std::wstring* st = new std::wstring(L"Encoding: " + std::to_wstring(displayPct) + L"%");
            PostMessage(g_app.hWnd, WM_APP_JOB_PROGRESS, pct, (LPARAM)st);
            return;
        }
    }
}

// Spawn process with stdout/stderr redirection
int run_process_capture(const std::wstring& cmd, const fs::path& workDir, std::string& outAll, bool filterLog, bool logToWindow = true) {
    HANDLE hRead = NULL, hWrite = NULL;
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return -1;
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);
    
    STARTUPINFO si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    
    PROCESS_INFORMATION pi{};
    std::vector<wchar_t> cmdBuf(cmd.begin(), cmd.end());
    cmdBuf.push_back(0);
    
    std::wstring dirStr = workDir.empty() ? L"" : workDir.wstring();
    LPCWSTR lpDir = dirStr.empty() ? NULL : dirStr.c_str();
    
    BOOL created = CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, lpDir, &si, &pi);
    CloseHandle(hWrite);
    
    if (!created) {
        CloseHandle(hRead);
        return -1;
    }
    
    {
        std::lock_guard<std::mutex> lock(g_app.procMutex);
        g_app.activeProcess = pi;
    }
    
    std::string lineBuf;
    char buffer[1024];
    DWORD bytesRead = 0;
    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = 0;
        outAll.append(buffer, bytesRead);
        
        if (logToWindow) {
            for (DWORD i = 0; i < bytesRead; ++i) {
                char c = buffer[i];
                if (c == '\r' || c == '\n') {
                    if (!lineBuf.empty()) {
                        parse_job_progress(lineBuf);
                        if (!filterLog || is_clean_log(lineBuf)) {
                            log_text(s2ws(lineBuf));
                        }
                        lineBuf.clear();
                    }
                } else {
                    lineBuf.push_back(c);
                }
            }
        }
    }
    if (logToWindow && !lineBuf.empty()) {
        parse_job_progress(lineBuf);
        if (!filterLog || is_clean_log(lineBuf)) {
            log_text(s2ws(lineBuf));
        }
    }
    
    CloseHandle(hRead);
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    {
        std::lock_guard<std::mutex> lock(g_app.procMutex);
        g_app.activeProcess.hProcess = NULL;
    }
    
    return static_cast<int>(exitCode);
}

void cancel_job() {
    g_app.jobCancelled = true;
    std::lock_guard<std::mutex> lock(g_app.procMutex);
    if (g_app.activeProcess.hProcess != NULL) {
        TerminateProcess(g_app.activeProcess.hProcess, 1);
    }
    SendMessageW(g_app.hProgressBar, PBM_SETPOS, 0, 0);
    SetWindowTextW(g_app.hLblJobProgress, L"Cancelled");
}

// Download Worker
void download_thread(std::string key) {
    g_app.dlRunning = true;
    
    wchar_t tempDir[MAX_PATH];
    GetTempPathW(MAX_PATH, tempDir);
    fs::path tmpFile = fs::path(tempDir) / (key + ".7z");
    
    auto it = URLS.find(key);
    if (it == URLS.end()) {
        g_app.dlRunning = false;
        PostMessage(g_app.hWnd, WM_APP_DL_DONE, 0, (LPARAM)new std::wstring(L"Invalid download URL"));
        return;
    }
    
    bool ok = false;
    std::wstring lastErr = L"Unknown error";
    
    for (int attempt = 1; attempt <= 3; ++attempt) {
        std::wstring statusMsg = L"Downloading " + s2ws(key) + L"... (attempt " + std::to_wstring(attempt) + L"/3)";
        SetWindowTextW(g_app.hLblDlProgress, statusMsg.c_str());
        
        DownloadProgressCallback* cb = new DownloadProgressCallback(g_app.hWnd, key);
        HRESULT hr = URLDownloadToFileW(NULL, it->second.c_str(), tmpFile.c_str(), 0, cb);
        cb->Release();
        
        if (SUCCEEDED(hr)) {
            SetWindowTextW(g_app.hLblDlProgress, (L"Extracting " + s2ws(key) + L"...").c_str());
            fs::create_directories(g_app.runtimePath);
            
            std::wstring cmd = L"\"" + g_app.sevenZipPath.wstring() + L"\" x \"" + tmpFile.wstring() + L"\" -o\"" + g_app.runtimePath.wstring() + L"\" -y";
            std::string outStr;
            int code = run_process_capture(cmd, g_app.runtimePath, outStr, false, false);
            if (code == 0) {
                ok = true;
                break;
            } else {
                lastErr = L"7z extraction failed: " + s2ws(outStr);
            }
        } else {
            lastErr = L"Download failed with error HRESULT 0x" + s2ws(std::to_string(hr));
        }
        
        try { fs::remove(tmpFile); } catch (...) {}
    }
    
    try { fs::remove(tmpFile); } catch (...) {}
    g_app.dlRunning = false;
    
    PostMessage(g_app.hWnd, WM_APP_DL_DONE, ok ? 1 : 0, (LPARAM)new std::wstring(lastErr));
}

void start_download(const std::string& key) {
    if (g_app.dlRunning) return;
    EnableWindow(g_app.hBtnFFmpeg, FALSE);
    EnableWindow(g_app.hBtnPy, FALSE);
    EnableWindow(g_app.hBtnModel, FALSE);
    std::thread(download_thread, key).detach();
}

// Background Job Worker
void job_thread(std::wstring inputPath, std::wstring outPath, int mult, double scale) {
    g_app.jobRunning = true;
    g_app.jobCancelled = false;
    
    bool success = false;
    std::wstring errMsg;
    fs::path workDir = g_app.rootPath / "work";
    
    try {
        std::string engine = g_app.config["engine"];
        if (engine.empty()) engine = "rife-cuda";
        
        fs::path ffmpegExe = g_app.runtimePath / "ffmpeg" / "ffmpeg.exe";
        fs::path pythonExe = g_app.runtimePath / "py-amp" / "python.exe";
        fs::path rifeScript = g_app.runtimePath / "rife-cuda" / "rife.py";
        fs::path rifeModel = g_app.runtimePath / "rife-cuda" / "RIFE40";
        fs::path ncnnExe = g_app.runtimePath / "rife-ncnn" / "rife-ncnn-vulkan.exe";
        fs::path ncnnModel = g_app.runtimePath / "rife-ncnn" / "rife-v3.1";
        
        if (!fs::exists(ffmpegExe)) throw std::runtime_error("Invalid path: ffmpeg");
        if (engine == "rife-cuda") {
            if (!fs::exists(pythonExe)) throw std::runtime_error("Invalid path: python");
            if (!fs::exists(rifeScript)) throw std::runtime_error("Invalid path: rife_script");
            if (!fs::exists(rifeModel)) throw std::runtime_error("Invalid path: model");
        } else if (engine == "rife-ncnn") {
            if (!fs::exists(ncnnExe)) throw std::runtime_error("Invalid path: ncnn_exe");
            if (!fs::exists(ncnnModel)) throw std::runtime_error("Invalid path: ncnn_model");
        } else {
            throw std::runtime_error("This engine is exclusive to FrameMeld Pro ($5).");
        }
        
        std::string enc = g_app.config["encoder"];
        if (enc.empty()) enc = "av1_nvenc";
        
        // Verify encoder in ffmpeg
        std::wstring chkCmd = L"\"" + ffmpegExe.wstring() + L"\" -hide_banner -encoders";
        std::string chkOut;
        run_process_capture(chkCmd, g_app.rootPath, chkOut, false, false);
        if (chkOut.find(enc) == std::string::npos) {
            throw std::runtime_error("Selected FFmpeg has no " + enc + ".");
        }

        fs::path srcDir = workDir / "input_frames";
        fs::path dstDir = workDir / "rife_output";
        if (fs::exists(workDir)) fs::remove_all(workDir);
        fs::create_directories(srcDir);
        fs::create_directories(dstDir);
        
        PostMessage(g_app.hWnd, WM_APP_JOB_PROGRESS, 2, (LPARAM)new std::wstring(L"Extracting frames..."));
        log_text(L"Extracting frames...");
        std::wstring hwaccel = (g_app.gpuVendors.count("NVIDIA") > 0) ? L"cuda" : L"d3d11va";
        
        std::wstring extractCmd = L"\"" + ffmpegExe.wstring() + L"\" -y -hwaccel " + hwaccel +
            L" -i \"" + inputPath + L"\" -fps_mode passthrough -compression_level 1 \"" +
            (srcDir / L"%08d.png").wstring() + L"\"";
            
        std::string dummyOut;
        int code = run_process_capture(extractCmd, g_app.rootPath, dummyOut, true);
        if (code != 0 || g_app.jobCancelled) throw std::runtime_error("FFmpeg extraction failed.");
        
        fs::path finalDst = dstDir;
        PostMessage(g_app.hWnd, WM_APP_JOB_PROGRESS, 5, (LPARAM)new std::wstring(L"Interpolating: 0%"));
        if (engine == "rife-cuda") {
            log_text(L"Running RIFE CUDA...");
            std::wstringstream rcmd;
            rcmd << L"\"" << pythonExe.wstring() << L"\" \"" << rifeScript.wstring()
                 << L"\" --input \"" << srcDir.wstring() << L"\" --output \"" << dstDir.wstring()
                 << L"\" --model \"" << rifeModel.wstring() << L"\" --fp16 --multi " << mult
                 << L" --scale " << scale;
            code = run_process_capture(rcmd.str(), g_app.rootPath, dummyOut, true);
            if (code != 0 || g_app.jobCancelled) throw std::runtime_error("RIFE failed. See log.");
        } else {
            log_text(L"Running RIFE NCNN...");
            std::wstring ncmd = L"\"" + ncnnExe.wstring() + L"\" -i \"" + srcDir.wstring() +
                L"\" -o \"" + dstDir.wstring() + L"\" -m \"" + ncnnModel.wstring() + L"\"";
            code = run_process_capture(ncmd, ncnnExe.parent_path(), dummyOut, true);
            if (code != 0 || g_app.jobCancelled) throw std::runtime_error("RIFE NCNN failed. See log.");
            
            if (mult == 4) {
                fs::path pass2 = workDir / "rife_output_pass2";
                fs::create_directories(pass2);
                log_text(L"Running RIFE NCNN (2nd pass for 4x)...");
                std::wstring ncmd2 = L"\"" + ncnnExe.wstring() + L"\" -i \"" + dstDir.wstring() +
                    L"\" -o \"" + pass2.wstring() + L"\" -m \"" + ncnnModel.wstring() + L"\"";
                code = run_process_capture(ncmd2, ncnnExe.parent_path(), dummyOut, true);
                if (code != 0 || g_app.jobCancelled) throw std::runtime_error("RIFE NCNN 2nd pass failed. See log.");
                finalDst = pass2;
            }
        }
        
        // Find generated frames
        std::vector<fs::path> frames;
        for (const auto& entry : fs::directory_iterator(finalDst)) {
            if (entry.path().extension() == ".png") {
                frames.push_back(entry.path());
            }
        }
        if (frames.empty()) throw std::runtime_error("RIFE produced no PNG frames.");
        std::sort(frames.begin(), frames.end());
        
        int widthDigits = static_cast<int>(frames[0].stem().wstring().length());
        std::wstringstream patternSs;
        patternSs << finalDst.wstring() << L"\\%0" << widthDigits << L"d.png";
        std::wstring pattern = patternSs.str();
        
        PostMessage(g_app.hWnd, WM_APP_JOB_PROGRESS, 90, (LPARAM)new std::wstring(L"Encoding video..."));
        log_text(L"Encoding " + s2ws(enc) + L"...");
        std::wstring qflag = (enc.rfind("lib", 0) == 0) ? L"-crf" : L"-cq";
        std::string preset = g_app.config["preset"];
        if (preset.empty()) preset = "p5";
        std::string cq = g_app.config["cq"];
        if (cq.empty()) cq = "28";
        
        std::wstring encodeCmd = L"\"" + ffmpegExe.wstring() + L"\" -y -framerate 60 -i \"" +
            pattern + L"\" -i \"" + inputPath + L"\" -map 0:v:0 -map 1:a? -c:v " + s2ws(enc) +
            L" -preset " + s2ws(preset) + L" " + qflag + L" " + s2ws(cq) +
            L" -pix_fmt yuv420p -c:a copy \"" + outPath + L"\"";
            
        code = run_process_capture(encodeCmd, g_app.rootPath, dummyOut, true);
        if (code != 0 || g_app.jobCancelled) throw std::runtime_error("Encoding failed.");
        
        try { fs::remove_all(workDir); } catch (...) {}
        success = true;
    } catch (const std::exception& ex) {
        if (g_app.jobCancelled) {
            try { if (fs::exists(workDir)) fs::remove_all(workDir); } catch (...) {}
            errMsg = L"Cancelled.";
        } else {
            errMsg = s2ws(ex.what());
        }
    }
    
    g_app.jobRunning = false;
    PostMessage(g_app.hWnd, WM_APP_JOB_DONE, success ? 1 : 0, (LPARAM)new std::wstring(success ? outPath : errMsg));
}

void show_pro_popup(const std::string& key) {
    std::wstring msg;
    if (key == "rife-cuda-opt") {
        msg = L"RIFE CUDA Optimized is an exclusive feature of FrameMeld Pro ($5).\n\n"
              L"It provides:\n"
              L"• Direct in-memory streaming pipe (Zero temporary PNGs)\n"
              L"• Low CPU usage & prevents overheating (60-75°C vs 100°C)\n"
              L"• Significant SSD lifespan saving & faster processing\n\n"
              L"Would you like to visit the page to get FrameMeld Pro?";
    } else {
        msg = L"RIFE NCNN Optimized is an exclusive feature of FrameMeld Pro ($5).\n\n"
              L"It provides:\n"
              L"• Fast low-temperature image buffer (Zero CPU zlib overhead)\n"
              L"• Regulated OpenMP threads (Prevents all-core CPU boost overheating)\n"
              L"• Multi-GPU targeting (AMD / Intel / NVIDIA Vulkan)\n"
              L"• Cool operating temperatures (~60-70°C vs 95-100°C)\n\n"
              L"Would you like to visit the page to get FrameMeld Pro?";
    }
    int res = MessageBoxW(g_app.hWnd, msg.c_str(), L"FrameMeld Pro Exclusive", MB_YESNO | MB_ICONINFORMATION);
    if (res == IDYES) {
        ShellExecuteW(NULL, L"open", L"https://luxbane.itch.io/framemeld", NULL, NULL, SW_SHOWNORMAL);
    }
}

void start_job() {
    if (g_app.jobRunning) return;
    
    // Check selection
    int engIdx = (int)SendMessage(g_app.hCmbEngine, CB_GETCURSEL, 0, 0);
    int encIdx = (int)SendMessage(g_app.hCmbEncoder, CB_GETCURSEL, 0, 0);
    if (engIdx <= 0 || encIdx <= 0) {
        MessageBoxW(g_app.hWnd, L"Choose an engine and an encoder first.", L"FrameMeld", MB_ICONWARNING);
        return;
    }

    if (MODELS[engIdx - 1].key == "rife-cuda-opt" || MODELS[engIdx - 1].key == "rife-ncnn-opt") {
        show_pro_popup(MODELS[engIdx - 1].key);
        return;
    }
    
    wchar_t inBuf[MAX_PATH], outDirBuf[MAX_PATH];
    GetWindowTextW(g_app.hEdtInput, inBuf, MAX_PATH);
    GetWindowTextW(g_app.hEdtOutput, outDirBuf, MAX_PATH);
    
    if (!fs::exists(inBuf)) {
        MessageBoxW(g_app.hWnd, L"Choose a valid input video.", L"FrameMeld", MB_ICONWARNING);
        return;
    }
    if (!fs::exists(outDirBuf)) {
        MessageBoxW(g_app.hWnd, L"Choose a valid output folder.", L"FrameMeld", MB_ICONWARNING);
        return;
    }
    
    save_settings();
    
    wchar_t multiBuf[32];
    GetWindowTextW(g_app.hCmbMulti, multiBuf, 32);
    int mult = (wcscmp(multiBuf, L"2x") == 0) ? 2 : 4;
    
    wchar_t scaleBuf[32];
    GetWindowTextW(g_app.hCmbScale, scaleBuf, 32);
    double scale = _wtof(scaleBuf);
    if (scale <= 0.0) scale = 1.0;
    
    int fps = static_cast<int>(std::round((g_app.srcFps > 0 ? g_app.srcFps : 30.0) * mult));
    
    fs::path inPath(inBuf);
    std::wstring stem = inPath.stem().wstring();
    
    wchar_t fmtBuf[32];
    GetWindowTextW(g_app.hCmbFormat, fmtBuf, 32);
    
    fs::path outFilePath = fs::path(outDirBuf) / (stem + L" " + std::to_wstring(fps) + L" fps" + fmtBuf);
    
    g_app.totalTargetFrames = (g_app.totalSrcFrames > 0 ? g_app.totalSrcFrames : 100) * mult;
    SendMessageW(g_app.hProgressBar, PBM_SETPOS, 0, 0);
    SetWindowTextW(g_app.hLblJobProgress, L"Starting...");

    EnableWindow(g_app.hBtnStart, FALSE);
    EnableWindow(g_app.hBtnCancel, TRUE);
    SetWindowTextW(g_app.hEdtLog, L"");
    
    std::thread(job_thread, inPath.wstring(), outFilePath.wstring(), mult, scale).detach();
}

// Handle selection of input video file (via dialog, paste, or drag-and-drop)
void on_input_file_selected(const std::wstring& filePath) {
    if (filePath.empty()) return;
    SetWindowTextW(g_app.hEdtInput, filePath.c_str());
    
    // Auto-set output folder to the input file's directory
    try {
        fs::path p(filePath);
        if (p.has_parent_path()) {
            SetWindowTextW(g_app.hEdtOutput, p.parent_path().wstring().c_str());
        }
        
        // Auto-match format combobox if extension matches
        std::wstring ext = p.extension().wstring();
        if (!ext.empty()) {
            std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
            int idx = (int)SendMessageW(g_app.hCmbFormat, CB_FINDSTRINGEXACT, -1, (LPARAM)ext.c_str());
            if (idx != CB_ERR) {
                SendMessageW(g_app.hCmbFormat, CB_SETCURSEL, idx, 0);
            }
        }
    } catch (...) {}
    
    probe_input();
}

// Probe input video via ffprobe
void probe_input() {
    wchar_t inBuf[MAX_PATH];
    GetWindowTextW(g_app.hEdtInput, inBuf, MAX_PATH);
    if (!inBuf[0] || !fs::exists(inBuf)) {
        g_app.srcFps = 0.0;
        SetWindowTextW(g_app.hLblInfo, L"");
        return;
    }
    
    fs::path ffprobeExe = g_app.runtimePath / "ffmpeg" / "ffprobe.exe";
    if (!fs::exists(ffprobeExe)) return;
    
    std::wstring cmd = L"\"" + ffprobeExe.wstring() + L"\" -v error -select_streams v:0 "
        L"-show_entries stream=width,height,r_frame_rate,nb_frames "
        L"-show_entries format=duration -of json \"" + std::wstring(inBuf) + L"\"";
        
    std::string outJson;
    if (run_process_capture(cmd, g_app.rootPath, outJson, false, false) != 0) {
        g_app.srcFps = 0.0;
        SetWindowTextW(g_app.hLblInfo, L"");
        return;
    }
    
    try {
        // Quick extraction from json
        int width = 0, height = 0;
        double fps = 0.0, dur = 0.0;
        long long frames = 0;
        
        size_t posW = outJson.find("\"width\":");
        if (posW != std::string::npos) width = std::stoi(outJson.substr(posW + 8));
        
        size_t posH = outJson.find("\"height\":");
        if (posH != std::string::npos) height = std::stoi(outJson.substr(posH + 9));
        
        size_t posFps = outJson.find("\"r_frame_rate\":");
        if (posFps != std::string::npos) {
            size_t q1 = outJson.find('"', posFps + 15);
            size_t q2 = outJson.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos) {
                std::string rate = outJson.substr(q1 + 1, q2 - q1 - 1);
                size_t slash = rate.find('/');
                if (slash != std::string::npos) {
                    double num = std::stod(rate.substr(0, slash));
                    double den = std::stod(rate.substr(slash + 1));
                    if (den > 0) fps = num / den;
                }
            }
        }
        
        size_t posDur = outJson.find("\"duration\":");
        if (posDur != std::string::npos) {
            size_t q1 = outJson.find('"', posDur + 11);
            size_t q2 = outJson.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos) {
                dur = std::stod(outJson.substr(q1 + 1, q2 - q1 - 1));
            }
        }
        
        size_t posFr = outJson.find("\"nb_frames\":");
        if (posFr != std::string::npos) {
            size_t q1 = outJson.find('"', posFr + 12);
            size_t q2 = outJson.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos) {
                try { frames = std::stoll(outJson.substr(q1 + 1, q2 - q1 - 1)); } catch (...) {}
            }
        }
        if (frames == 0 && dur > 0 && fps > 0) frames = static_cast<long long>(dur * fps);
        
        g_app.srcFps = fps;
        g_app.totalSrcFrames = frames;
        int h = static_cast<int>(dur / 3600);
        int m = static_cast<int>(std::fmod(dur, 3600.0) / 60);
        int sec = static_cast<int>(std::fmod(dur, 60.0));
        
        std::wstringstream ss;
        ss << L"Size: " << width << L"x" << height << L" - Rate: " << std::fixed << std::setprecision(2)
           << fps << L" FPS - Frames: " << frames << L" - Duration: "
           << std::setw(2) << std::setfill(L'0') << h << L":"
           << std::setw(2) << std::setfill(L'0') << m << L":"
           << std::setw(2) << std::setfill(L'0') << sec;
        SetWindowTextW(g_app.hLblInfo, ss.str().c_str());
    } catch (...) {
        g_app.srcFps = 0.0;
        g_app.totalSrcFrames = 0;
        SetWindowTextW(g_app.hLblInfo, L"");
    }
}

// UI State Refreshers
void update_model_desc() {
    int idx = (int)SendMessage(g_app.hCmbEngine, CB_GETCURSEL, 0, 0);
    if (idx <= 0) {
        SetWindowTextW(g_app.hLblModelDesc, L"");
        return;
    }
    int modelIdx = idx - 1;
    if (modelIdx >= 0 && modelIdx < (int)MODELS.size()) {
        SetWindowTextW(g_app.hLblModelDesc, MODELS[modelIdx].desc.c_str());
    }
}

void update_layout(bool showSetup) {
    int setupShow = showSetup ? SW_SHOW : SW_HIDE;
    
    ShowWindow(g_app.hGrpSetup, setupShow);
    ShowWindow(g_app.hLblFFmpeg, setupShow);
    ShowWindow(g_app.hStFFmpeg, setupShow);
    ShowWindow(g_app.hBtnFFmpeg, setupShow);
    ShowWindow(g_app.hLblPy, setupShow);
    ShowWindow(g_app.hStPy, setupShow);
    ShowWindow(g_app.hBtnPy, setupShow);
    ShowWindow(g_app.hLblModel, setupShow);
    ShowWindow(g_app.hStModel, setupShow);
    ShowWindow(g_app.hBtnModel, setupShow);
    ShowWindow(g_app.hLblModelDesc, setupShow);
    ShowWindow(g_app.hLblDlProgress, setupShow);
    
    int dy = showSetup ? 0 : -142;
    
    HDWP hdwp = BeginDeferWindowPos(30);
    if (hdwp) {
        auto moveCtrl = [&](HWND hCtrl, int x, int baseY, int w, int h) {
            if (hCtrl) {
                hdwp = DeferWindowPos(hdwp, hCtrl, NULL, x, baseY + dy, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
            }
        };
        
        moveCtrl(g_app.hGrpVideo, 20, 195, 850, 125);
        moveCtrl(g_app.hLblInput, 35, 218, 90, 20);
        moveCtrl(g_app.hEdtInput, 130, 216, 610, 24);
        moveCtrl(g_app.hBtnInput, 750, 215, 105, 26);
        moveCtrl(g_app.hLblOutput, 35, 248, 90, 20);
        moveCtrl(g_app.hEdtOutput, 130, 246, 610, 24);
        moveCtrl(g_app.hBtnOutput, 750, 245, 105, 26);
        moveCtrl(g_app.hLblFormat, 35, 278, 90, 20);
        moveCtrl(g_app.hCmbFormat, 130, 276, 120, 150);
        moveCtrl(g_app.hLblInfo, 265, 278, 590, 20);
        
        moveCtrl(g_app.hGrpEngine, 20, 328, 850, 135);
        moveCtrl(g_app.hLblEngine, 35, 352, 60, 20);
        moveCtrl(g_app.hCmbEngine, 100, 349, 320, 150);
        moveCtrl(g_app.hLblMulti, 460, 352, 70, 20);
        moveCtrl(g_app.hCmbMulti, 540, 349, 100, 100);
        moveCtrl(g_app.hLblScale, 35, 382, 60, 20);
        moveCtrl(g_app.hCmbScale, 100, 379, 120, 150);
        moveCtrl(g_app.hLblEncoder, 460, 382, 70, 20);
        moveCtrl(g_app.hCmbEncoder, 540, 379, 310, 200);
        moveCtrl(g_app.hLblPreset, 460, 412, 70, 20);
        moveCtrl(g_app.hCmbPreset, 540, 409, 140, 150);
        moveCtrl(g_app.hLblCq, 700, 412, 35, 20);
        moveCtrl(g_app.hEdtCq, 740, 409, 65, 24);
        
        moveCtrl(g_app.hBtnStart, 20, 473, 420, 34);
        moveCtrl(g_app.hBtnCancel, 450, 473, 420, 34);
        
        moveCtrl(g_app.hProgressBar, 20, 515, 710, 18);
        moveCtrl(g_app.hLblJobProgress, 740, 514, 130, 20);
        
        int logHeight = showSetup ? 156 : (156 + 142);
        moveCtrl(g_app.hEdtLog, 20, 539, 850, logHeight);
        
        EndDeferWindowPos(hdwp);
    }
    
    if (g_app.hSpnCq && g_app.hEdtCq) {
        SendMessage(g_app.hSpnCq, UDM_SETBUDDY, (WPARAM)g_app.hEdtCq, 0);
    }
    
    InvalidateRect(g_app.hWnd, NULL, TRUE);
    UpdateWindow(g_app.hWnd);
}

void refresh_setup() {
    bool hasFfmpeg = fs::exists(g_app.runtimePath / "ffmpeg" / "ffmpeg.exe");
    bool hasPy = fs::exists(g_app.runtimePath / "py-amp" / "python.exe");
    
    SetWindowTextW(g_app.hStFFmpeg, hasFfmpeg ? L"✓ Installed" : L"Not installed");
    EnableWindow(g_app.hBtnFFmpeg, !hasFfmpeg);
    
    int engIdx = (int)SendMessage(g_app.hCmbEngine, CB_GETCURSEL, 0, 0);
    bool modelOk = false;
    bool pyOk = true;
    
    if (engIdx <= 0) {
        SetWindowTextW(g_app.hStPy, L"—");
        EnableWindow(g_app.hBtnPy, FALSE);
        SetWindowTextW(g_app.hStModel, L"Choose an engine first");
        EnableWindow(g_app.hBtnModel, FALSE);
        SetWindowTextW(g_app.hGrpEngine, L"RIFE -> Encoder");
    } else {
        std::string engKey = MODELS[engIdx - 1].key;
        if (engKey == "rife-cuda-opt") {
            SetWindowTextW(g_app.hStPy, L"Requires FrameMeld Pro");
            EnableWindow(g_app.hBtnPy, FALSE);
            pyOk = true;
            modelOk = false;
            SetWindowTextW(g_app.hGrpEngine, L"RIFE CUDA (Optimized) [PRO] -> NVENC");
            SetWindowTextW(g_app.hStModel, L"FrameMeld Pro Feature");
            EnableWindow(g_app.hBtnModel, FALSE);
        } else if (engKey == "rife-ncnn-opt") {
            SetWindowTextW(g_app.hStPy, L"Not needed for this engine");
            EnableWindow(g_app.hBtnPy, FALSE);
            pyOk = true;
            modelOk = false;
            SetWindowTextW(g_app.hGrpEngine, L"RIFE NCNN (Optimized) [PRO] -> Encoder");
            SetWindowTextW(g_app.hStModel, L"FrameMeld Pro Feature");
            EnableWindow(g_app.hBtnModel, FALSE);
        } else if (engKey == "rife-cuda") {
            SetWindowTextW(g_app.hStPy, hasPy ? L"✓ Installed" : L"Not installed");
            EnableWindow(g_app.hBtnPy, !hasPy);
            pyOk = hasPy;
            modelOk = fs::exists(g_app.runtimePath / "rife-cuda" / "rife.py");
            SetWindowTextW(g_app.hGrpEngine, L"RIFE CUDA -> NVENC");
            SetWindowTextW(g_app.hStModel, modelOk ? L"✓ Installed" : L"Not installed");
            EnableWindow(g_app.hBtnModel, !modelOk);
        } else {
            SetWindowTextW(g_app.hStPy, L"Not needed for this engine");
            EnableWindow(g_app.hBtnPy, FALSE);
            pyOk = true;
            modelOk = fs::exists(g_app.runtimePath / "rife-ncnn" / "rife-ncnn-vulkan.exe");
            SetWindowTextW(g_app.hGrpEngine, L"RIFE NCNN -> Encoder");
            SetWindowTextW(g_app.hStModel, modelOk ? L"✓ Installed" : L"Not installed");
            EnableWindow(g_app.hBtnModel, !modelOk);
        }
    }
    
    int encIdx = (int)SendMessage(g_app.hCmbEncoder, CB_GETCURSEL, 0, 0);
    bool encOk = (encIdx > 0);
    if (encOk) {
        LRESULT isEnabled = SendMessage(g_app.hCmbEncoder, CB_GETITEMDATA, encIdx, 0);
        if (!isEnabled) encOk = false;
    }
    bool isProEngine = (engIdx > 0 && engIdx <= (int)MODELS.size() && (MODELS[engIdx - 1].key == "rife-cuda-opt" || MODELS[engIdx - 1].key == "rife-ncnn-opt"));
    bool allGood = hasFfmpeg && pyOk && (modelOk || isProEngine) && encOk;
    
    bool showIt = ((!allGood) || (!modelOk && !isProEngine)) || g_app.showSetup;
    update_layout(showIt);
    EnableWindow(g_app.hBtnStart, allGood && !g_app.jobRunning);
}

void refresh_presets() {
    int encIdx = (int)SendMessage(g_app.hCmbEncoder, CB_GETCURSEL, 0, 0);
    SendMessage(g_app.hCmbPreset, CB_RESETCONTENT, 0, 0);
    if (encIdx <= 0) return;
    
    std::string encId = ENCODERS[encIdx - 1].id;
    std::string grp = get_preset_group(encId);
    
    auto it = PRESETS.find(grp);
    if (it != PRESETS.end()) {
        for (const auto& p : it->second) {
            SendMessage(g_app.hCmbPreset, CB_ADDSTRING, 0, (LPARAM)s2ws(p).c_str());
        }
        SendMessage(g_app.hCmbPreset, CB_SETCURSEL, 0, 0);
    }
}

void refresh_encoders() {
    // Check compatibility
    int count = (int)SendMessage(g_app.hCmbEncoder, CB_GETCOUNT, 0, 0);
    for (int i = 1; i < count; ++i) {
        const auto& enc = ENCODERS[i - 1];
        bool ok = enc.vendor.empty() || (g_app.gpuVendors.count(enc.vendor) > 0);
        // Tag item data as enabled/disabled (1=enabled, 0=disabled)
        SendMessage(g_app.hCmbEncoder, CB_SETITEMDATA, i, ok ? 1 : 0);
    }
}

bool get_system_dark_mode() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD val = 1;
        DWORD sz = sizeof(val);
        if (RegQueryValueExW(hKey, L"AppsUseLightTheme", NULL, NULL, (LPBYTE)&val, &sz) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return (val == 0);
        }
        RegCloseKey(hKey);
    }
    return false;
}

void apply_theme(bool isDark) {
    g_app.isDarkMode = isDark;
    
    if (g_app.hBtnTheme) {
        SetWindowTextW(g_app.hBtnTheme, isDark ? L"Theme: 🌙 Dark" : L"Theme: ☀️ Light");
    }
    
    if (g_app.hWnd) {
        BOOL dwmDark = isDark ? TRUE : FALSE;
        DwmSetWindowAttribute(g_app.hWnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dwmDark, sizeof(dwmDark));
        
        LPCWSTR themeName = isDark ? L"DarkMode_Explorer" : L"Explorer";
        HWND controls[] = {
            g_app.hCmbFormat, g_app.hCmbEngine, g_app.hCmbMulti, g_app.hCmbScale,
            g_app.hCmbEncoder, g_app.hCmbPreset, g_app.hEdtLog,
            g_app.hEdtInput, g_app.hEdtOutput, g_app.hEdtCq,
            g_app.hBtnTheme, g_app.hBtnToggleSetup, g_app.hBtnInput, g_app.hBtnOutput,
            g_app.hBtnFFmpeg, g_app.hBtnPy, g_app.hBtnModel,
            g_app.hBtnStart, g_app.hBtnCancel, g_app.hProgressBar
        };
        for (HWND h : controls) {
            if (h) SetWindowTheme(h, themeName, NULL);
        }
        
        InvalidateRect(g_app.hWnd, NULL, TRUE);
        EnumChildWindows(g_app.hWnd, [](HWND hChild, LPARAM) -> BOOL {
            InvalidateRect(hChild, NULL, TRUE);
            return TRUE;
        }, 0);
        UpdateWindow(g_app.hWnd);
    }
}

void save_settings() {
    if (g_app.isRestoring) return;
    
    int engIdx = (int)SendMessage(g_app.hCmbEngine, CB_GETCURSEL, 0, 0);
    if (engIdx > 0 && engIdx <= (int)MODELS.size()) {
        g_app.config["engine"] = MODELS[engIdx - 1].key;
    }
    
    int encIdx = (int)SendMessage(g_app.hCmbEncoder, CB_GETCURSEL, 0, 0);
    if (encIdx > 0 && encIdx <= (int)ENCODERS.size()) {
        g_app.config["encoder"] = ENCODERS[encIdx - 1].id;
    }
    
    wchar_t presetBuf[64];
    GetWindowTextW(g_app.hCmbPreset, presetBuf, 64);
    if (presetBuf[0]) {
        g_app.config["preset"] = ws2s(presetBuf);
    }
    
    wchar_t cqBuf[32];
    GetWindowTextW(g_app.hEdtCq, cqBuf, 32);
    if (cqBuf[0]) {
        g_app.config["cq"] = ws2s(cqBuf);
    }
    
    g_app.config["theme"] = g_app.isDarkMode ? "dark" : "light";
    
    save_config();
}

void restore_settings() {
    if (!fs::exists(g_app.cfgPath)) {
        apply_theme(get_system_dark_mode());
        return;
    }
    g_app.isRestoring = true;
    
    std::string savedTheme = g_app.config["theme"];
    if (!savedTheme.empty()) {
        apply_theme(savedTheme == "dark");
    } else {
        apply_theme(get_system_dark_mode());
    }
    
    std::string savedEngine = g_app.config["engine"];
    for (size_t i = 0; i < MODELS.size(); ++i) {
        if (MODELS[i].key == savedEngine) {
            LRESULT isEnabled = SendMessage(g_app.hCmbEngine, CB_GETITEMDATA, i + 1, 0);
            if (isEnabled == 1) {
                SendMessage(g_app.hCmbEngine, CB_SETCURSEL, i + 1, 0);
                g_app.lastValidEngineIdx = (int)(i + 1);
                update_model_desc();
            }
            break;
        }
    }
    
    std::string savedEnc = g_app.config["encoder"];
    for (size_t i = 0; i < ENCODERS.size(); ++i) {
        if (ENCODERS[i].id == savedEnc) {
            LRESULT isEnabled = SendMessage(g_app.hCmbEncoder, CB_GETITEMDATA, i + 1, 0);
            if (isEnabled) {
                SendMessage(g_app.hCmbEncoder, CB_SETCURSEL, i + 1, 0);
                g_app.lastValidEncoderIdx = (int)(i + 1);
            }
            break;
        }
    }
    
    refresh_presets();
    
    std::string savedPreset = g_app.config["preset"];
    if (!savedPreset.empty()) {
        std::wstring wsPreset = s2ws(savedPreset);
        int idx = (int)SendMessage(g_app.hCmbPreset, CB_FINDSTRINGEXACT, -1, (LPARAM)wsPreset.c_str());
        if (idx != CB_ERR) {
            SendMessage(g_app.hCmbPreset, CB_SETCURSEL, idx, 0);
        }
    }
    
    std::string savedCq = g_app.config["cq"];
    if (!savedCq.empty()) {
        try {
            int val = std::stoi(savedCq);
            SendMessage(g_app.hSpnCq, UDM_SETPOS32, 0, val);
        } catch (...) {}
    }
    
    g_app.isRestoring = false;
    refresh_setup();
}

// Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_MEASUREITEM: {
        MEASUREITEMSTRUCT* pMIS = (MEASUREITEMSTRUCT*)lParam;
        if (pMIS->CtlType == ODT_COMBOBOX && (pMIS->CtlID == ID_CMB_ENGINE || pMIS->CtlID == ID_CMB_ENCODER)) {
            if (pMIS->itemID == (UINT)-1) {
                pMIS->itemHeight = 19;
            } else {
                pMIS->itemHeight = 22;
            }
            return TRUE;
        }
        break;
    }
    
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* pDIS = (DRAWITEMSTRUCT*)lParam;
        if (pDIS->CtlType == ODT_COMBOBOX && (pDIS->CtlID == ID_CMB_ENGINE || pDIS->CtlID == ID_CMB_ENCODER)) {
            if (pDIS->itemID == (UINT)-1) {
                HBRUSH hBr = CreateSolidBrush(g_app.isDarkMode ? RGB(45, 45, 45) : GetSysColor(COLOR_WINDOW));
                FillRect(pDIS->hDC, &pDIS->rcItem, hBr);
                DeleteObject(hBr);
                return TRUE;
            }

            wchar_t text[256] = {0};
            SendMessageW(pDIS->hwndItem, CB_GETLBTEXT, pDIS->itemID, (LPARAM)text);
            LRESULT itemData = SendMessageW(pDIS->hwndItem, CB_GETITEMDATA, pDIS->itemID, 0);

            bool isComboEdit = (pDIS->itemState & ODS_COMBOBOXEDIT) != 0;
            bool isPro = (itemData == 2);
            bool isDisabled = (itemData == 0);
            bool isGrayed = isPro || isDisabled;
            bool isSelected = (pDIS->itemState & ODS_SELECTED) != 0;
            bool isDark = g_app.isDarkMode;

            COLORREF bgColor;
            COLORREF textColor;

            if (isGrayed) {
                if (isSelected && !isComboEdit) {
                    bgColor = isDark ? RGB(45, 48, 55) : RGB(235, 238, 245);
                    textColor = isDark ? RGB(140, 140, 140) : RGB(115, 115, 115);
                } else {
                    bgColor = isDark ? (isComboEdit ? RGB(45, 45, 45) : RGB(32, 32, 32)) : GetSysColor(COLOR_WINDOW);
                    textColor = isDark ? RGB(105, 105, 105) : GetSysColor(COLOR_GRAYTEXT);
                }
            } else if (isSelected && !isComboEdit) {
                bgColor = isDark ? RGB(0, 120, 215) : GetSysColor(COLOR_HIGHLIGHT);
                textColor = isDark ? RGB(255, 255, 255) : GetSysColor(COLOR_HIGHLIGHTTEXT);
            } else {
                bgColor = isDark ? (isComboEdit ? RGB(45, 45, 45) : RGB(32, 32, 32)) : GetSysColor(COLOR_WINDOW);
                textColor = isDark ? RGB(240, 240, 240) : GetSysColor(COLOR_WINDOWTEXT);
            }

            HBRUSH hBr = CreateSolidBrush(bgColor);
            FillRect(pDIS->hDC, &pDIS->rcItem, hBr);
            DeleteObject(hBr);

            HFONT hOldFont = (HFONT)SelectObject(pDIS->hDC, g_app.hFont);
            SetBkMode(pDIS->hDC, TRANSPARENT);
            SetTextColor(pDIS->hDC, textColor);

            RECT rcText = pDIS->rcItem;
            rcText.left += 6;
            rcText.right -= 4;
            DrawTextW(pDIS->hDC, text, -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            SelectObject(pDIS->hDC, hOldFont);
            return TRUE;
        }
        break;
    }

    case WM_CREATE: {
        DragAcceptFiles(hWnd, TRUE);
        // Create Segoe UI Font
        g_app.hFont = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        g_app.hFontBold = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            
        // Brushes
        g_app.hDarkBgBrush = CreateSolidBrush(RGB(30, 30, 30));
        g_app.hDarkEditBrush = CreateSolidBrush(RGB(45, 45, 45));
        g_app.hDarkLogBrush = CreateSolidBrush(RGB(24, 24, 24));
            
        // Top bar
        g_app.hGpuInfo = CreateWindowW(L"STATIC", (L"Detected GPU: " + g_app.gpuText).c_str(),
            WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP | SS_NOPREFIX, 20, 15, 600, 24, hWnd, (HMENU)ID_GPU_INFO, g_app.hInstance, NULL);
        g_app.hBtnTheme = CreateWindowW(L"BUTTON", L"Theme: 🌙 Dark",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 630, 12, 105, 28, hWnd, (HMENU)ID_BTN_THEME, g_app.hInstance, NULL);
        g_app.hBtnToggleSetup = CreateWindowW(L"BUTTON", L"Runtime Downloads",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 742, 12, 130, 28, hWnd, (HMENU)ID_BTN_TOGGLE_SETUP, g_app.hInstance, NULL);
            
        // Setup Group
        g_app.hGrpSetup = CreateWindowW(L"BUTTON", L"Setup — Required Downloads",
            WS_CHILD | BS_GROUPBOX, 20, 48, 850, 140, hWnd, (HMENU)ID_GRP_SETUP, g_app.hInstance, NULL);
            
        g_app.hLblFFmpeg = CreateWindowW(L"STATIC", L"FFmpeg (required):", WS_VISIBLE | WS_CHILD | SS_LEFT, 35, 70, 180, 20, hWnd, (HMENU)ID_LBL_FFMPEG, g_app.hInstance, NULL);
        g_app.hStFFmpeg = CreateWindowW(L"STATIC", L"Not installed", WS_VISIBLE | WS_CHILD | SS_LEFT, 220, 70, 150, 20, hWnd, (HMENU)ID_ST_FFMPEG, g_app.hInstance, NULL);
        g_app.hBtnFFmpeg = CreateWindowW(L"BUTTON", L"Download", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 380, 68, 100, 24, hWnd, (HMENU)ID_BTN_FFMPEG, g_app.hInstance, NULL);
        
        g_app.hLblPy = CreateWindowW(L"STATIC", L"Python Runtime (required):", WS_VISIBLE | WS_CHILD | SS_LEFT, 35, 96, 180, 20, hWnd, (HMENU)ID_LBL_PY, g_app.hInstance, NULL);
        g_app.hStPy = CreateWindowW(L"STATIC", L"Not installed", WS_VISIBLE | WS_CHILD | SS_LEFT, 220, 96, 150, 20, hWnd, (HMENU)ID_ST_PY, g_app.hInstance, NULL);
        g_app.hBtnPy = CreateWindowW(L"BUTTON", L"Download", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 380, 94, 100, 24, hWnd, (HMENU)ID_BTN_PY, g_app.hInstance, NULL);
        
        g_app.hLblModel = CreateWindowW(L"STATIC", L"Selected AI Model:", WS_VISIBLE | WS_CHILD | SS_LEFT, 35, 122, 180, 20, hWnd, (HMENU)ID_LBL_MODEL, g_app.hInstance, NULL);
        g_app.hStModel = CreateWindowW(L"STATIC", L"Not installed", WS_VISIBLE | WS_CHILD | SS_LEFT, 220, 122, 150, 20, hWnd, (HMENU)ID_ST_MODEL, g_app.hInstance, NULL);
        g_app.hBtnModel = CreateWindowW(L"BUTTON", L"Download", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 380, 120, 100, 24, hWnd, (HMENU)ID_BTN_MODEL, g_app.hInstance, NULL);
        
        g_app.hLblModelDesc = CreateWindowW(L"STATIC", L"", WS_VISIBLE | WS_CHILD | SS_LEFT, 35, 148, 820, 18, hWnd, (HMENU)ID_LBL_MODEL_DESC, g_app.hInstance, NULL);
        g_app.hLblDlProgress = CreateWindowW(L"STATIC", L"", WS_VISIBLE | WS_CHILD | SS_LEFT, 35, 166, 820, 18, hWnd, (HMENU)ID_LBL_DL_PROGRESS, g_app.hInstance, NULL);
        
        // Video Group
        g_app.hGrpVideo = CreateWindowW(L"BUTTON", L"Video", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 20, 195, 850, 125, hWnd, (HMENU)ID_GRP_VIDEO, g_app.hInstance, NULL);
        
        g_app.hLblInput = CreateWindowW(L"STATIC", L"Input:", WS_VISIBLE | WS_CHILD | SS_LEFT, 35, 218, 90, 20, hWnd, (HMENU)ID_LBL_INPUT, g_app.hInstance, NULL);
        g_app.hEdtInput = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 130, 216, 610, 24, hWnd, (HMENU)ID_EDT_INPUT, g_app.hInstance, NULL);
        g_app.hBtnInput = CreateWindowW(L"BUTTON", L"Input", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 750, 215, 105, 26, hWnd, (HMENU)ID_BTN_INPUT, g_app.hInstance, NULL);
        
        g_app.hLblOutput = CreateWindowW(L"STATIC", L"Output Folder:", WS_VISIBLE | WS_CHILD | SS_LEFT, 35, 248, 90, 20, hWnd, (HMENU)ID_LBL_OUTPUT, g_app.hInstance, NULL);
        g_app.hEdtOutput = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 130, 246, 610, 24, hWnd, (HMENU)ID_EDT_OUTPUT, g_app.hInstance, NULL);
        g_app.hBtnOutput = CreateWindowW(L"BUTTON", L"Output Folder", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 750, 245, 105, 26, hWnd, (HMENU)ID_BTN_OUTPUT, g_app.hInstance, NULL);
        
        g_app.hLblFormat = CreateWindowW(L"STATIC", L"Format:", WS_VISIBLE | WS_CHILD | SS_LEFT, 35, 278, 90, 20, hWnd, (HMENU)ID_LBL_FORMAT, g_app.hInstance, NULL);
        g_app.hCmbFormat = CreateWindowW(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 130, 276, 120, 150, hWnd, (HMENU)ID_CMB_FORMAT, g_app.hInstance, NULL);
        SendMessage(g_app.hCmbFormat, CB_ADDSTRING, 0, (LPARAM)L".mkv");
        SendMessage(g_app.hCmbFormat, CB_ADDSTRING, 0, (LPARAM)L".mp4");
        SendMessage(g_app.hCmbFormat, CB_ADDSTRING, 0, (LPARAM)L".mov");
        SendMessage(g_app.hCmbFormat, CB_ADDSTRING, 0, (LPARAM)L".webm");
        SendMessage(g_app.hCmbFormat, CB_SETCURSEL, 0, 0);
        
        g_app.hLblInfo = CreateWindowW(L"STATIC", L"", WS_VISIBLE | WS_CHILD | SS_LEFT, 265, 278, 590, 20, hWnd, (HMENU)ID_LBL_INFO, g_app.hInstance, NULL);
        
        // RIFE -> Encoder Group
        g_app.hGrpEngine = CreateWindowW(L"BUTTON", L"RIFE -> NVENC", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 20, 328, 850, 135, hWnd, (HMENU)ID_GRP_ENGINE, g_app.hInstance, NULL);
        
        g_app.hLblEngine = CreateWindowW(L"STATIC", L"Engine:", WS_VISIBLE | WS_CHILD | SS_LEFT, 35, 352, 60, 20, hWnd, (HMENU)ID_LBL_ENGINE, g_app.hInstance, NULL);
        g_app.hCmbEngine = CreateWindowW(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS | WS_VSCROLL, 100, 349, 320, 200, hWnd, (HMENU)ID_CMB_ENGINE, g_app.hInstance, NULL);
        SendMessage(g_app.hCmbEngine, CB_ADDSTRING, 0, (LPARAM)L"-- Please choose an engine --");
        SendMessage(g_app.hCmbEngine, CB_SETITEMDATA, 0, 1);
        for (size_t i = 0; i < MODELS.size(); ++i) {
            bool isPro = (MODELS[i].key == "rife-cuda-opt" || MODELS[i].key == "rife-ncnn-opt");
            bool hardwareOk = MODELS[i].vendorReq.empty() || (g_app.gpuVendors.count(MODELS[i].vendorReq) > 0);
            std::wstring label = MODELS[i].label;
            if (!hardwareOk) {
                label += L" (Requires NVIDIA)";
            }
            LPARAM itemData = 1;
            if (!hardwareOk) {
                itemData = 0; // Hardware disabled
            } else if (isPro) {
                itemData = 2; // Pro exclusive (grayed out)
            }
            SendMessage(g_app.hCmbEngine, CB_ADDSTRING, 0, (LPARAM)label.c_str());
            SendMessage(g_app.hCmbEngine, CB_SETITEMDATA, i + 1, itemData);
        }
        SendMessage(g_app.hCmbEngine, CB_SETCURSEL, 0, 0);
        
        g_app.hLblMulti = CreateWindowW(L"STATIC", L"Multiplier:", WS_VISIBLE | WS_CHILD | SS_LEFT, 460, 352, 70, 20, hWnd, (HMENU)ID_LBL_MULTI, g_app.hInstance, NULL);
        g_app.hCmbMulti = CreateWindowW(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 540, 349, 100, 100, hWnd, (HMENU)ID_CMB_MULTI, g_app.hInstance, NULL);
        SendMessage(g_app.hCmbMulti, CB_ADDSTRING, 0, (LPARAM)L"2x");
        SendMessage(g_app.hCmbMulti, CB_ADDSTRING, 0, (LPARAM)L"4x");
        SendMessage(g_app.hCmbMulti, CB_SETCURSEL, 0, 0);
        
        g_app.hLblScale = CreateWindowW(L"STATIC", L"Scale:", WS_VISIBLE | WS_CHILD | SS_LEFT, 35, 382, 60, 20, hWnd, (HMENU)ID_LBL_SCALE, g_app.hInstance, NULL);
        g_app.hCmbScale = CreateWindowW(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 100, 379, 120, 150, hWnd, (HMENU)ID_CMB_SCALE, g_app.hInstance, NULL);
        SendMessage(g_app.hCmbScale, CB_ADDSTRING, 0, (LPARAM)L"0.25");
        SendMessage(g_app.hCmbScale, CB_ADDSTRING, 0, (LPARAM)L"0.5");
        SendMessage(g_app.hCmbScale, CB_ADDSTRING, 0, (LPARAM)L"1.0");
        SendMessage(g_app.hCmbScale, CB_ADDSTRING, 0, (LPARAM)L"2.0");
        SendMessage(g_app.hCmbScale, CB_ADDSTRING, 0, (LPARAM)L"4.0");
        SendMessage(g_app.hCmbScale, CB_SETCURSEL, 2, 0);
        
        g_app.hLblEncoder = CreateWindowW(L"STATIC", L"Encoder:", WS_VISIBLE | WS_CHILD | SS_LEFT, 460, 382, 70, 20, hWnd, (HMENU)ID_LBL_ENCODER, g_app.hInstance, NULL);
        g_app.hCmbEncoder = CreateWindowW(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS | WS_VSCROLL, 540, 379, 310, 220, hWnd, (HMENU)ID_CMB_ENCODER, g_app.hInstance, NULL);
        SendMessage(g_app.hCmbEncoder, CB_ADDSTRING, 0, (LPARAM)L"-- Please choose an encoder --");
        SendMessage(g_app.hCmbEncoder, CB_SETITEMDATA, 0, 1);
        for (size_t i = 0; i < ENCODERS.size(); ++i) {
            bool ok = ENCODERS[i].vendor.empty() || (g_app.gpuVendors.count(ENCODERS[i].vendor) > 0);
            std::wstring label = ENCODERS[i].label;
            if (!ok) {
                label += L" (Unsupported)";
            }
            SendMessage(g_app.hCmbEncoder, CB_ADDSTRING, 0, (LPARAM)label.c_str());
            SendMessage(g_app.hCmbEncoder, CB_SETITEMDATA, i + 1, ok ? 1 : 0);
        }
        SendMessage(g_app.hCmbEncoder, CB_SETCURSEL, 0, 0);
        
        g_app.hLblPreset = CreateWindowW(L"STATIC", L"Preset:", WS_VISIBLE | WS_CHILD | SS_LEFT, 460, 412, 70, 20, hWnd, (HMENU)ID_LBL_PRESET, g_app.hInstance, NULL);
        g_app.hCmbPreset = CreateWindowW(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 540, 409, 140, 150, hWnd, (HMENU)ID_CMB_PRESET, g_app.hInstance, NULL);
        
        g_app.hLblCq = CreateWindowW(L"STATIC", L"CQ:", WS_VISIBLE | WS_CHILD | SS_LEFT, 700, 412, 35, 20, hWnd, (HMENU)ID_LBL_CQ, g_app.hInstance, NULL);
        g_app.hEdtCq = CreateWindowW(L"EDIT", L"28", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 740, 409, 65, 24, hWnd, (HMENU)ID_EDT_CQ, g_app.hInstance, NULL);
        g_app.hSpnCq = CreateWindowW(UPDOWN_CLASS, NULL, WS_VISIBLE | WS_CHILD | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
            0, 0, 0, 0, hWnd, (HMENU)ID_SPN_CQ, g_app.hInstance, NULL);
        SendMessage(g_app.hSpnCq, UDM_SETBUDDY, (WPARAM)g_app.hEdtCq, 0);
        SendMessage(g_app.hSpnCq, UDM_SETRANGE32, 0, 51);
        SendMessage(g_app.hSpnCq, UDM_SETPOS32, 0, 28);
        
        // Buttons
        g_app.hBtnStart = CreateWindowW(L"BUTTON", L"START", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 20, 473, 420, 34, hWnd, (HMENU)ID_BTN_START, g_app.hInstance, NULL);
        g_app.hBtnCancel = CreateWindowW(L"BUTTON", L"CANCEL", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 450, 473, 420, 34, hWnd, (HMENU)ID_BTN_CANCEL, g_app.hInstance, NULL);
        EnableWindow(g_app.hBtnCancel, FALSE);
        
        // Progress Bar & Status
        g_app.hProgressBar = CreateWindowExW(0, PROGRESS_CLASS, NULL,
            WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
            20, 515, 710, 18, hWnd, (HMENU)ID_PROGRESS_BAR, g_app.hInstance, NULL);
        SendMessageW(g_app.hProgressBar, PBM_SETRANGE32, 0, 100);
        SendMessageW(g_app.hProgressBar, PBM_SETPOS, 0, 0);
        
        g_app.hLblJobProgress = CreateWindowW(L"STATIC", L"Ready",
            WS_VISIBLE | WS_CHILD | SS_RIGHT,
            740, 514, 130, 20, hWnd, (HMENU)ID_LBL_JOB_PROGRESS, g_app.hInstance, NULL);

        // Log box
        g_app.hEdtLog = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
            20, 539, 850, 156, hWnd, (HMENU)ID_EDT_LOG, g_app.hInstance, NULL);
        SendMessageW(g_app.hEdtLog, EM_SETLIMITTEXT, 0, 0);
            
        // Apply Segoe UI font to all child windows
        EnumChildWindows(hWnd, [](HWND hChild, LPARAM lParam) -> BOOL {
            SendMessage(hChild, WM_SETFONT, lParam, TRUE);
            return TRUE;
        }, (LPARAM)g_app.hFont);
        
        SendMessage(g_app.hGpuInfo, WM_SETFONT, (WPARAM)g_app.hFontBold, TRUE);
        SendMessage(g_app.hBtnStart, WM_SETFONT, (WPARAM)g_app.hFontBold, TRUE);
        
        refresh_presets();
        refresh_encoders();
        restore_settings();
        break;
    }
    
    case WM_COMMAND: {
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);
        
        if (id == ID_BTN_TOGGLE_SETUP) {
            g_app.showSetup = !g_app.showSetup;
            refresh_setup();
        }
        else if (id == ID_BTN_THEME) {
            apply_theme(!g_app.isDarkMode);
            save_settings();
        }
        else if (id == ID_BTN_FFMPEG) {
            start_download("ffmpeg");
        }
        else if (id == ID_BTN_PY) {
            start_download("py-amp");
        }
        else if (id == ID_BTN_MODEL) {
            int idx = (int)SendMessage(g_app.hCmbEngine, CB_GETCURSEL, 0, 0);
            if (idx > 0 && idx <= (int)MODELS.size()) {
                start_download(MODELS[idx - 1].key);
            }
        }
        else if (id == ID_BTN_INPUT) {
            wchar_t szFile[MAX_PATH] = {0};
            OPENFILENAMEW ofn{};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = L"Video (*.mkv;*.mp4;*.mov;*.webm)\0*.mkv;*.mp4;*.mov;*.webm\0All Files (*.*)\0*.*\0";
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            if (GetOpenFileNameW(&ofn)) {
                on_input_file_selected(szFile);
            }
        }
        else if (id == ID_EDT_INPUT && code == EN_KILLFOCUS) {
            wchar_t inBuf[MAX_PATH];
            GetWindowTextW(g_app.hEdtInput, inBuf, MAX_PATH);
            if (inBuf[0] && fs::exists(inBuf)) {
                wchar_t outBuf[MAX_PATH];
                GetWindowTextW(g_app.hEdtOutput, outBuf, MAX_PATH);
                if (!outBuf[0]) {
                    on_input_file_selected(inBuf);
                } else {
                    probe_input();
                }
            }
        }
        else if (id == ID_BTN_OUTPUT) {
            BROWSEINFOW bi{};
            bi.hwndOwner = hWnd;
            bi.lpszTitle = L"Select Output Folder";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
            PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);
            if (pidl) {
                wchar_t szFolder[MAX_PATH];
                if (SHGetPathFromIDListW(pidl, szFolder)) {
                    SetWindowTextW(g_app.hEdtOutput, szFolder);
                }
                CoTaskMemFree(pidl);
            }
        }
        else if (id == ID_CMB_ENGINE && code == CBN_SELCHANGE) {
            int sel = (int)SendMessage(g_app.hCmbEngine, CB_GETCURSEL, 0, 0);
            if (sel > 0 && sel <= (int)MODELS.size()) {
                LRESULT itemData = SendMessage(g_app.hCmbEngine, CB_GETITEMDATA, sel, 0);
                if (itemData == 2) {
                    SendMessage(g_app.hCmbEngine, CB_SETCURSEL, g_app.lastValidEngineIdx, 0);
                    show_pro_popup(MODELS[sel - 1].key);
                    return 0;
                }
                if (itemData == 0) {
                    SendMessage(g_app.hCmbEngine, CB_SETCURSEL, g_app.lastValidEngineIdx, 0);
                    MessageBeep(MB_OK);
                    return 0;
                }
                g_app.lastValidEngineIdx = sel;
            } else {
                g_app.lastValidEngineIdx = 0;
            }
            update_model_desc();
            refresh_setup();
            save_settings();
        }
        else if (id == ID_CMB_ENCODER && code == CBN_SELCHANGE) {
            int sel = (int)SendMessage(g_app.hCmbEncoder, CB_GETCURSEL, 0, 0);
            if (sel > 0) {
                LRESULT enabled = SendMessage(g_app.hCmbEncoder, CB_GETITEMDATA, sel, 0);
                if (!enabled) {
                    SendMessage(g_app.hCmbEncoder, CB_SETCURSEL, g_app.lastValidEncoderIdx, 0);
                    MessageBeep(MB_OK);
                    return 0;
                }
                g_app.lastValidEncoderIdx = sel;
            } else {
                g_app.lastValidEncoderIdx = 0;
            }
            refresh_presets();
            refresh_setup();
            save_settings();
        }
        else if (id == ID_CMB_PRESET && code == CBN_SELCHANGE) {
            save_settings();
        }
        else if (id == ID_EDT_CQ && code == EN_CHANGE) {
            save_settings();
        }
        else if (id == ID_BTN_START) {
            start_job();
        }
        else if (id == ID_BTN_CANCEL) {
            cancel_job();
        }
        break;
    }
    
    case WM_DROPFILES: {
        HDROP hDrop = (HDROP)wParam;
        wchar_t szFile[MAX_PATH];
        if (DragQueryFileW(hDrop, 0, szFile, MAX_PATH)) {
            on_input_file_selected(szFile);
        }
        DragFinish(hDrop);
        break;
    }
    
    case WM_APP_LOG: {
        std::wstring* msg = (std::wstring*)wParam;
        if (msg) {
            int len = GetWindowTextLengthW(g_app.hEdtLog);
            if (len > 200000) {
                // If log exceeds 200 KB, prune oldest 50 KB to keep buffer fast and responsive
                SendMessageW(g_app.hEdtLog, EM_SETSEL, 0, 50000);
                SendMessageW(g_app.hEdtLog, EM_REPLACESEL, FALSE, (LPARAM)L"");
                len = GetWindowTextLengthW(g_app.hEdtLog);
            }
            SendMessageW(g_app.hEdtLog, EM_SETSEL, len, len);
            std::wstring toAppend = *msg + L"\r\n";
            SendMessageW(g_app.hEdtLog, EM_REPLACESEL, FALSE, (LPARAM)toAppend.c_str());
            SendMessageW(g_app.hEdtLog, EM_SCROLLCARET, 0, 0);
            delete msg;
        }
        break;
    }
    
    case WM_APP_DL_PROGRESS: {
        int pct = (int)wParam;
        SetWindowTextW(g_app.hLblDlProgress, (std::to_wstring(pct) + L"%").c_str());
        break;
    }
    
    case WM_APP_DL_DONE: {
        int success = (int)wParam;
        std::wstring* msg = (std::wstring*)lParam;
        if (!success && msg) {
            MessageBoxW(hWnd, (L"Download failed:\n" + *msg).c_str(), L"FrameMeld", MB_ICONERROR);
        }
        SetWindowTextW(g_app.hLblDlProgress, L"");
        if (msg) delete msg;
        refresh_setup();
        break;
    }
    
    case WM_APP_JOB_PROGRESS: {
        int pct = (int)wParam;
        std::wstring* st = (std::wstring*)lParam;
        SendMessageW(g_app.hProgressBar, PBM_SETPOS, pct, 0);
        if (st) {
            SetWindowTextW(g_app.hLblJobProgress, st->c_str());
            delete st;
        }
        break;
    }
    
    case WM_APP_JOB_DONE: {
        int success = (int)wParam;
        std::wstring* msg = (std::wstring*)lParam;
        EnableWindow(g_app.hBtnStart, TRUE);
        EnableWindow(g_app.hBtnCancel, FALSE);
        if (success) {
            SendMessageW(g_app.hProgressBar, PBM_SETPOS, 100, 0);
            SetWindowTextW(g_app.hLblJobProgress, L"100% (Done)");
        } else {
            SendMessageW(g_app.hProgressBar, PBM_SETPOS, 0, 0);
            SetWindowTextW(g_app.hLblJobProgress, L"Stopped");
        }
        if (msg) {
            if (success) {
                MessageBoxW(hWnd, (L"Done:\n" + *msg).c_str(), L"FrameMeld", MB_ICONINFORMATION);
            } else {
                MessageBoxW(hWnd, (L"Failed:\n" + *msg).c_str(), L"FrameMeld", MB_ICONERROR);
            }
            delete msg;
        }
        break;
    }
    
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, g_app.isDarkMode ? g_app.hDarkBgBrush : GetSysColorBrush(COLOR_BTNFACE));
        return 1;
    }
    
    case WM_CTLCOLORSTATIC: {
        HWND hCtrl = (HWND)lParam;
        HDC hdcStatic = (HDC)wParam;
        if (hCtrl == g_app.hEdtLog) {
            if (g_app.isDarkMode) {
                SetTextColor(hdcStatic, RGB(225, 225, 225));
                SetBkColor(hdcStatic, RGB(24, 24, 24));
                return (LRESULT)g_app.hDarkLogBrush;
            } else {
                SetTextColor(hdcStatic, GetSysColor(COLOR_WINDOWTEXT));
                SetBkColor(hdcStatic, GetSysColor(COLOR_WINDOW));
                return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
            }
        }
        SetBkMode(hdcStatic, TRANSPARENT);
        if (g_app.isDarkMode) {
            SetTextColor(hdcStatic, RGB(225, 225, 225));
            return (LRESULT)g_app.hDarkBgBrush;
        } else {
            SetTextColor(hdcStatic, GetSysColor(COLOR_WINDOWTEXT));
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
    }
    
    case WM_CTLCOLOREDIT: {
        HDC hdcEdit = (HDC)wParam;
        if (g_app.isDarkMode) {
            SetTextColor(hdcEdit, RGB(245, 245, 245));
            SetBkColor(hdcEdit, RGB(45, 45, 45));
            return (LRESULT)g_app.hDarkEditBrush;
        } else {
            SetTextColor(hdcEdit, GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor(hdcEdit, GetSysColor(COLOR_WINDOW));
            return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
        }
    }
    
    case WM_CTLCOLORLISTBOX: {
        HDC hdc = (HDC)wParam;
        if (g_app.isDarkMode) {
            SetTextColor(hdc, RGB(245, 245, 245));
            SetBkColor(hdc, RGB(45, 45, 45));
            return (LRESULT)g_app.hDarkEditBrush;
        }
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    
    case WM_DESTROY: {
        if (g_app.hFont) DeleteObject(g_app.hFont);
        if (g_app.hFontBold) DeleteObject(g_app.hFontBold);
        if (g_app.hDarkBgBrush) DeleteObject(g_app.hDarkBgBrush);
        if (g_app.hDarkEditBrush) DeleteObject(g_app.hDarkEditBrush);
        if (g_app.hDarkLogBrush) DeleteObject(g_app.hDarkLogBrush);
        PostQuitMessage(0);
        break;
    }
    
    default:
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    INITCOMMONCONTROLSEX iccex{};
    iccex.dwSize = sizeof(iccex);
    iccex.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES | ICC_UPDOWN_CLASS | ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&iccex);
    
    // Per-monitor DPI awareness
    typedef BOOL(WINAPI* PFN_SetProcessDpiAwarenessContext)(void*);
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        auto pfn = (PFN_SetProcessDpiAwarenessContext)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (pfn) pfn((void*)-4); // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
        else SetProcessDPIAware();
    }
    
    g_app.hInstance = hInstance;
    
    // Paths
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    g_app.rootPath = fs::path(exePath).parent_path();
    
    wchar_t* localAppData = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &localAppData))) {
        g_app.appDataPath = fs::path(localAppData) / "FrameMeld";
        CoTaskMemFree(localAppData);
    } else {
        g_app.appDataPath = g_app.rootPath;
    }
    g_app.runtimePath = g_app.appDataPath / "runtime";
    g_app.cfgPath = g_app.appDataPath / "config.json";
    g_app.sevenZipPath = g_app.rootPath / "tools" / "7za.exe";
    if (!fs::exists(g_app.sevenZipPath)) {
        if (fs::exists(g_app.rootPath / "app" / "tools" / "7za.exe")) {
            g_app.sevenZipPath = g_app.rootPath / "app" / "tools" / "7za.exe";
        }
    }
    
    detect_gpu();
    load_config();
    
    // Load Application Icon
    HICON hAppIcon = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(1), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
    HICON hAppIconSm = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(1), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    if (!hAppIcon) hAppIcon = LoadIconW(NULL, IDI_APPLICATION);
    if (!hAppIconSm) hAppIconSm = hAppIcon;
    
    // Register Window Class
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = hAppIcon;
    wc.hIconSm = hAppIconSm;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"FrameMeldMainWindow";
    RegisterClassExW(&wc);
    
    // Create Window
    RECT wr = {0, 0, 890, 715};
    AdjustWindowRectEx(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE, 0);
    int w = wr.right - wr.left;
    int h = wr.bottom - wr.top;
    
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - w) / 2;
    int y = (screenH - h) / 2;
    
    g_app.hWnd = CreateWindowExW(0, L"FrameMeldMainWindow", L"FrameMeld v1.3.1",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        x, y, w, h, NULL, NULL, hInstance, NULL);
        
    if (!g_app.hWnd) return 1;
    
    if (hAppIcon) SendMessageW(g_app.hWnd, WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
    if (hAppIconSm) SendMessageW(g_app.hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hAppIconSm);
    
    ShowWindow(g_app.hWnd, nCmdShow);
    UpdateWindow(g_app.hWnd);
    
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    return static_cast<int>(msg.wParam);
}
