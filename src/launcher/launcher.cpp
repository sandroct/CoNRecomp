// Launcher do Champions of Norrath - PC Port (PS2Recomp).
//
// Uma aba para escolher a ISO e dar JOGAR, e abas para vídeo (GS do PCSX2),
// áudio, sistema e controles (estilo PCSX2). As escolhas viram variáveis de
// ambiente PS2X_* do con.exe (é assim que o runtime lê as opções: ele não tem
// leitor de config.ini; só controls.ini/<PS2X_PAD*> e os PS2X_*), além de um
// controls.ini escrito ao lado do jogo, no mesmo formato do Urban Reign.
//
// Build: tools/build_launcher.bat (cl.exe, sem CMake). Sem dependências.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>

// Alguns SDKs não expõem as flags de BROWSEINFO sem _WIN32_WINNT antigo.
#ifndef BIF_RETURNONLYDIRS
#define BIF_RETURNONLYDIRS 0x0001
#endif
#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE 0x0040
#endif

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ---------------------------------------------------------------------------
// Constantes de UI

enum
{
    PAGE_GAME = 0,
    PAGE_VIDEO,
    PAGE_AUDIO_SYS,
    PAGE_CONTROLS,
    PAGE_COUNT
};

static const wchar_t *kTabTitles[PAGE_COUNT] = {L"Jogo", L"Vídeo", L"Áudio e Sistema", L"Controles"};

// ---------------------------------------------------------------------------
// Estado

struct PadState
{
    bool typeNone = false;          // [PadN] Type = None
    char deadzone[16] = "0";
    char axisScale[16] = "1.33";
    char buttonDeadzone[16] = "0";
    char invertL[8] = "0";
    char invertR[8] = "0";
    char largeMotor[16] = "1";
    char smallMotor[16] = "1";
    std::vector<std::pair<std::string, std::string>> bindings; // chave -> "Keyboard/W;SDL-0/-LeftY"
};

struct Settings
{
    // Jogo
    std::wstring iso;
    std::wstring elf;
    std::wstring pnach;
    bool hdTextures = true;
    std::wstring textureFolder;
    bool texturePrecache = false;
    // Vídeo (GS do PCSX2)
    int renderer = 0;        // 0 = pcsx2 (pcsx2gs.dll), 1 = nativo
    int scale = 1;           // 1..8
    int vsync = 2;           // 0 off, 1 FIFO, 2 mailbox
    bool stretch = false;
    bool fxaa = true;
    int textureFiltering = -1;   // -1 = padrão (PS2), 0..3
    int trilinear = -1;          // -1 auto, 0 off, 1 ps2, 2 forçado
    int anisotropy = -1;         // -1 = padrão, 0/2/4/8/16
    int cas = 0;                 // 0 off, 1 sharpen, 2 sharpen+resize
    int casSharpness = 50;
    bool shadeboost = false;
    int brightness = 50, contrast = 50, saturation = 50, gamma = 50;
    int tvShader = 0;            // 0..7
    int presentFilter = -1;      // -1 padrão, 0/1/2
    int dithering = -1;          // -1 padrão, 0..3
    int blending = -1;           // -1 padrão, 0..5
    // Áudio
    bool mute = false;
    bool audioRateControl = true;
    bool audioStretch = true;
    bool audioStats = false;
    std::wstring audioDump;
    // Sistema
    bool showFps = false;
    bool vsyncLock = true;
    bool mtvu = true;
    bool mtvuStream = true;
    bool sdlInput = true;
    bool gameMenu = true;
    int eeLoopCycles = 16;
    int multitap = 0;            // bits: 1 = porta 1, 2 = porta 2
    // Controles
    int currentPad = 0;
    std::vector<PadState> pads{8};
};

static Settings g_settings;
static HFONT g_font = nullptr;
static HWND g_main = nullptr;
static HWND g_tab = nullptr;
static HWND g_status = nullptr;
static HWND g_play = nullptr;
static std::vector<HWND> g_pages[PAGE_COUNT];
static std::wstring g_exeDir;
static std::wstring g_launcherIni;
static std::wstring g_controlsIni;

// ---------------------------------------------------------------------------
// Utilidades

static std::wstring widen(const std::string &text)
{
    return std::wstring(text.begin(), text.end());
}

static std::string narrow(const std::wstring &text)
{
    std::string out;
    for (wchar_t ch : text)
        out += (ch >= 0x20 && ch < 0x7F) ? static_cast<char>(ch) : '?';
    return out;
}

static std::string trim(const std::string &text)
{
    size_t a = text.find_first_not_of(" \t\r\n");
    if (a == std::string::npos)
        return {};
    size_t b = text.find_last_not_of(" \t\r\n");
    return text.substr(a, b - a + 1);
}

static std::string lower(std::string text)
{
    for (char &c : text)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return text;
}

static std::wstring folderOf(const std::wstring &path)
{
    const size_t slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? L"." : path.substr(0, slash);
}

static bool fileExists(const std::wstring &path)
{
    const DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

static bool directoryExists(const std::wstring &path)
{
    const DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

// Registro simples de widgets, para ler/gravar o ini sem espalhar handles.
static std::vector<std::pair<std::string, HWND>> g_widgets;

static void remember(const char *name, HWND window)
{
    g_widgets.emplace_back(name, window);
}

static HWND widget(const char *name)
{
    for (const auto &entry : g_widgets)
        if (entry.first == name)
            return entry.second;
    return nullptr;
}

static void setText(const char *name, const std::wstring &text)
{
    if (const HWND control = widget(name))
        SetWindowTextW(control, text.c_str());
}

static void setTextA(const char *name, const std::string &text)
{
    setText(name, widen(text));
}

static std::wstring textOf(const char *name)
{
    std::wstring out;
    if (const HWND control = widget(name))
    {
        const int length = GetWindowTextLengthW(control);
        out.resize(length > 0 ? length : 0);
        if (length > 0)
            GetWindowTextW(control, out.data(), length + 1);
    }
    return out;
}

static void check(const char *name, bool value)
{
    if (const HWND control = widget(name))
        CheckDlgButton(control, BST_CHECKED, value ? BST_CHECKED : BST_UNCHECKED);
}

static bool checked(const char *name)
{
    if (const HWND control = widget(name))
        return IsDlgButtonChecked(control, BST_CHECKED) == BST_CHECKED;
    return false;
}

static void comboSelect(const char *name, int index)
{
    if (const HWND control = widget(name))
        SendMessageW(control, CB_SETCURSEL, index >= 0 ? index : 0, 0);
}

static int comboValue(const char *name)
{
    if (const HWND control = widget(name))
        return static_cast<int>(SendMessageW(control, CB_GETCURSEL, 0, 0));
    return -1;
}

static void comboAdd(const char *name, const wchar_t *items[], int count)
{
    const HWND control = widget(name);
    if (!control)
        return;
    for (int i = 0; i < count; ++i)
        SendMessageW(control, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(items[i]));
}

static void sliderSet(const char *name, int value)
{
    if (const HWND control = widget(name))
        SendMessageW(control, TBM_SETPOS, TRUE, value);
}

static int sliderValue(const char *name)
{
    if (const HWND control = widget(name))
        return static_cast<int>(SendMessageW(control, TBM_GETPOS, 0, 0));
    return 0;
}

static std::wstring browseForFile(HWND owner, const wchar_t *title, const wchar_t *filter, const wchar_t *extension)
{
    wchar_t file[MAX_PATH] = {};
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title;
    ofn.lpstrDefExt = extension;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    return GetOpenFileNameW(&ofn) ? std::wstring(file) : std::wstring();
}

static std::wstring browseForFolder(HWND owner, const wchar_t *title)
{
    wchar_t path[MAX_PATH] = {};
    BROWSEINFOW bi{};
    bi.hwndOwner = owner;
    bi.lpszTitle = title;
    bi.ulFlags = BIF_RETURNONLYDIRS | BIF_NEWDIALOGSTYLE;
    const PIDLIST_ABSOLUTE list = SHBrowseForFolderW(&bi);
    if (!list)
        return {};
    const bool ok = SHGetPathFromIDListW(list, path);
    CoTaskMemFree(list);
    return ok ? std::wstring(path) : std::wstring();
}

static void logLine(const std::wstring &text)
{
    std::wofstream stream(g_launcherIni + L".log", std::ios::app);
    if (stream)
        stream << text << L"\n";
}

static void statusLine(const std::wstring &text)
{
    SetWindowTextW(g_status, text.c_str());
}

// Aplica a fonte em toda a árvore de filhos.
static void applyFontRecursive(HWND window)
{
    if (!window)
        return;
    SetWindowFont(window, g_font, TRUE);
    for (HWND child = GetWindow(window, GW_CHILD); child; child = GetWindow(child, GW_HWNDNEXT))
        applyFontRecursive(child);
}

static void showPage(int index)
{
    for (int page = 0; page < PAGE_COUNT; ++page)
        for (HWND window : g_pages[page])
            ShowWindow(window, index == page ? SW_SHOW : SW_HIDE);
}

// ---------------------------------------------------------------------------
// Construtores de widgets (posições fixas; duas colunas por aba)

static const int kCol1 = 14;
static const int kCol2 = 372;
static const int kRowH = 26;

static HWND makeLabel(int page, const wchar_t *text, int x, int y, int w)
{
    HWND control = CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT,
                                   x, y, w, 18, g_main, nullptr, nullptr, nullptr);
    g_pages[page].push_back(control);
    return control;
}

static HWND makeEdit(int page, const char *name, const wchar_t *text, int x, int y, int w, int h = 20, bool readOnly = false)
{
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
    if (readOnly)
        style |= ES_READONLY;
    HWND control = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", text, style, x, y, w, h, g_main, nullptr, nullptr, nullptr);
    g_pages[page].push_back(control);
    remember(name, control);
    return control;
}

static HWND makeCheck(int page, const char *name, const wchar_t *text, int x, int y, int w)
{
    HWND control = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                                   x, y, w, 20, g_main, nullptr, nullptr, nullptr);
    g_pages[page].push_back(control);
    remember(name, control);
    return control;
}

static HWND makeCombo(int page, const char *name, int x, int y, int w)
{
    HWND control = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                   x, y, w, 220, g_main, nullptr, nullptr, nullptr);
    g_pages[page].push_back(control);
    remember(name, control);
    return control;
}

static HWND makeButton(int page, const char *name, const wchar_t *text, int x, int y, int w, int h = 22)
{
    HWND control = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   x, y, w, h, g_main, nullptr, nullptr, nullptr);
    g_pages[page].push_back(control);
    remember(name, control);
    return control;
}

static HWND makeSlider(int page, const char *name, int x, int y, int w, int min, int max, int value)
{
    HWND control = CreateWindowExW(0, TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
                                   x, y, w, 26, g_main, nullptr, nullptr, nullptr);
    SendMessageW(control, TBM_SETRANGE, TRUE, (max << 16) | min);
    SendMessageW(control, TBM_SETPOS, TRUE, value);
    g_pages[page].push_back(control);
    remember(name, control);
    return control;
}

static HWND makeSpinlessNumber(int page, const char *name, int x, int y, int w, const wchar_t *text)
{
    return makeEdit(page, name, text, x, y, w);
}

// ---------------------------------------------------------------------------
// Abas

static void buildPageGame()
{
    const int page = PAGE_GAME;
    int y = 14;
    makeLabel(page, L"Imagem do disco (.iso) do jogo:", kCol1, y, 400);
    y += 18;
    makeEdit(page, "iso", L"", kCol1, y, 470);
    makeButton(page, "browseIso", L"Procurar...", 494, y - 2, 90);
    y += kRowH + 4;
    makeLabel(page, L"Executável do jogo (ELF) - deixe o padrão para o SLUS_205.65 ao lado do con.exe:", kCol1, y, 620);
    y += 18;
    makeEdit(page, "elf", L"", kCol1, y, 470);
    makeButton(page, "browseElf", L"Procurar...", 494, y - 2, 90);
    y += kRowH + 4;
    makeLabel(page, L"Patches do PCSX2 (.pnach, separados por ;):", kCol1, y, 400);
    y += 18;
    makeEdit(page, "pnach", L"", kCol1, y, 470);
    makeButton(page, "browsePnach", L"Procurar...", 494, y - 2, 90);
    y += kRowH + 8;
    makeButton(page, "openFolder", L"Abrir pasta do jogo", kCol1, y, 150);
    makeButton(page, "detectIso", L"Detectar ISO automaticamente", 174, y, 190);
    y += 34;
    makeLabel(page, L"No jogo: F2 menu, F3 contador de FPS, F11 tela cheia, F5/F9 salvar/carregar estado.", kCol1, y, 620);
    y += 18;
    makeLabel(page, L"Tela cheia, 16:9 e 4:3 são opções dentro do jogo (não há como forçá-las pelo launcher).", kCol1, y, 620);
}

static void buildPageVideo()
{
    const int page = PAGE_VIDEO;
    static const wchar_t *renderers[] = {L"PCSX2 GS (DX11) - recomendado", L"GS nativo do port"};
    static const wchar_t *scales[] = {L"1x - 640x448 (PS2)", L"2x - 1280x896", L"3x - 1920x1344",
                                      L"4x - 2560x1792", L"5x - 3200x2240", L"6x - 3840x2688",
                                      L"7x - 4480x3136", L"8x - 5120x3584"};
    static const wchar_t *vsyncs[] = {L"Desligado", L"FIFO (espera o monitor)", L"Mailbox (padrão, sem tearing)"};
    static const wchar_t *filtering[] = {L"Nearest (pixels)", L"Bilinear forçado", L"Como o PS2 (padrão)", L"Forçado exceto sprites"};
    static const wchar_t *trilinears[] = {L"Automático (padrão)", L"Desligado", L"Como o PS2", L"Forçado"};
    static const wchar_t *anisotropics[] = {L"Padrão", L"Desligado", L"2x", L"4x", L"8x", L"16x"};
    static const wchar_t *cass[] = {L"Desligado", L"Só nitidez", L"Nitidez + redimensionar"};
    static const wchar_t *tvShaders[] = {L"Nenhum", L"Scanlines", L"Diagonal", L"Triangular", L"Ondas",
                                         L"CRT (Lottes)", L"4xRGSS", L"NxAGSS"};
    static const wchar_t *presents[] = {L"Padrão", L"Nenhum (pixels nítidos)", L"Suave", L"Nítido"};
    static const wchar_t *ditherings[] = {L"Padrão", L"Desligado", L"Escalonado", L"Não escalonado", L"Forçar 32 bits"};
    static const wchar_t *blendings[] = {L"Padrão", L"Mínima", L"Básica", L"Média", L"Alta", L"Muito alta", L"Máxima"};

    int y = 14;
    makeLabel(page, L"Renderizador:", kCol1, y, 160);
    makeCombo(page, "renderer", kCol1, y + 16, 340);
    makeLabel(page, L"Resolução interna:", kCol2, y, 200);
    makeCombo(page, "scale", kCol2, y + 16, 220);
    y += 42;
    makeLabel(page, L"VSync:", kCol1, y, 160);
    makeCombo(page, "vsync", kCol1, y + 16, 340);
    makeLabel(page, L"Filtragem de textura:", kCol2, y, 200);
    makeCombo(page, "textureFiltering", kCol2, y + 16, 220);
    y += 42;
    makeCheck(page, "fxaa", L"FXAA (anti-serrilado leve)", kCol1, y, 300);
    makeLabel(page, L"Trilinear:", kCol2, y, 200);
    makeCombo(page, "trilinear", kCol2, y + 16, 220);
    y += 28;
    makeCheck(page, "stretch", L"Esticar a imagem para a janela", kCol1, y, 300);
    makeLabel(page, L"Anisotrópico:", kCol2, y, 200);
    makeCombo(page, "anisotropy", kCol2, y + 16, 220);
    y += 42;
    makeLabel(page, L"Nitidez (AMD CAS):", kCol1, y, 200);
    makeCombo(page, "cas", kCol1, y + 16, 340);
    makeLabel(page, L"Força da nitidez:", kCol2, y, 200);
    makeSlider(page, "casSharpness", kCol2, y + 14, 150, 0, 100, 50);
    y += 44;
    makeCheck(page, "shadeboost", L"Correção de cor (brilho, contraste, saturação, gama)", kCol1, y, 340);
    makeLabel(page, L"Filtro de tela:", kCol2, y, 200);
    makeCombo(page, "presentFilter", kCol2, y + 16, 220);
    y += 30;
    static const struct { const char *slider; const wchar_t *label; } colors[] = {
        {"brightness", L"Brilho"}, {"contrast", L"Contraste"}, {"saturation", L"Saturação"}, {"gamma", L"Gama"}};
    for (int i = 0; i < 4; ++i)
    {
        const int x = kCol1 + i * 155;
        makeLabel(page, colors[i].label, x, y, 120);
        makeSlider(page, colors[i].slider, x, y + 14, 120, 0, 100, 50);
    }
    y += 40;
    (void)0;
    makeLabel(page, L"Shader de TV:", kCol1, y, 200);
    makeCombo(page, "tvShader", kCol1, y + 16, 340);
    makeLabel(page, L"Dithering:", kCol2, y, 200);
    makeCombo(page, "dithering", kCol2, y + 16, 220);
    y += 42;
    makeLabel(page, L"Precisão de blending:", kCol1, y, 200);
    makeCombo(page, "blending", kCol1, y + 16, 340);
    makeCheck(page, "texturePrecache", L"Pré-carregar texturas HD ao iniciar (usa muita memória)", kCol2, y + 16, 320);
    y += 44;
    makeCheck(page, "hdTextures", L"Texturas HD (pacotes PCSX2 em textures\\SLUS-20565)", kCol1, y, 330);
    makeEdit(page, "textureFolder", L"", kCol1, y + 22, 300);
    makeButton(page, "browseTextures", L"Pasta...", 320, y + 20, 70);

    comboAdd("renderer", renderers, 2);
    comboAdd("scale", scales, 8);
    comboAdd("vsync", vsyncs, 3);
    comboAdd("textureFiltering", filtering, 4);
    comboAdd("trilinear", trilinears, 4);
    comboAdd("anisotropy", anisotropics, 6);
    comboAdd("cas", cass, 3);
    comboAdd("tvShader", tvShaders, 8);
    comboAdd("presentFilter", presents, 4);
    comboAdd("dithering", ditherings, 5);
    comboAdd("blending", blendings, 7);
}

static void buildPageAudioSys()
{
    const int page = PAGE_AUDIO_SYS;
    int y = 14;
    makeLabel(page, L"Áudio", kCol1, y, 200);
    y += 18;
    makeCheck(page, "mute", L"Sem som", kCol1, y, 200);
    makeLabel(page, L"Sistema e desempenho", kCol2, y, 300);
    y += 24;
    makeCheck(page, "audioRateControl", L"Controle de taxa (mantém a fila cheia)", kCol1, y, 300);
    makeCheck(page, "showFps", L"Contador de FPS ao iniciar (F3)", kCol2, y, 300);
    y += 24;
    makeCheck(page, "audioStretch", L"Time-stretch (evita pitch estranho)", kCol1, y, 300);
    makeCheck(page, "vsyncLock", L"Travar o vblank na tela (60 Hz)", kCol2, y, 300);
    y += 24;
    makeCheck(page, "audioStats", L"Estatísticas de áudio no console", kCol1, y, 300);
    makeCheck(page, "mtvu", L"MTVU (VU1 e GS em threads)", kCol2, y, 300);
    y += 24;
    makeLabel(page, L"Gravar o áudio para (.wav):", kCol1, y, 300);
    makeEdit(page, "audioDump", L"", kCol1, y + 18, 240);
    makeButton(page, "browseAudioDump", L"Arquivo...", 254, y + 16, 80);
    makeCheck(page, "mtvuStream", L"MTVU em stream (mais rápido)", kCol1, y + 46, 300);
    y += 74;
    makeLabel(page, L"Entrada e jogo", kCol1, y, 300);
    y += 18;
    makeCheck(page, "sdlInput", L"Controles via SDL (vibração, controles PS)", kCol1, y, 300);
    makeCheck(page, "gameMenu", L"Menu do jogo no F2", kCol2, y, 300);
    y += 24;
    makeLabel(page, L"Multitap (mais jogadores por porta):", kCol1, y, 300);
    y += 18;
    makeCheck(page, "multitap1", L"Porta 1", kCol1, y, 120);
    makeCheck(page, "multitap2", L"Porta 2", kCol1 + 130, y, 120);
    makeLabel(page, L"Ciclos do EE por laço:", kCol2, y, 200);
    makeSpinlessNumber(page, "eeLoopCycles", kCol2, y + 18, 90, L"16");
    y += 44;
    makeLabel(page, L"Os valores padrão do runtime voltam sozinhos quando uma opção é desmarcada.", kCol1, y, 620);
}

// Botões do DualShock 2 na ordem do runtime (kPadButtonKeys + kStickKeys).
static const struct { const char *key; const wchar_t *label; } kPadButtons[] = {
    {"Up", L"Direcional ↑"}, {"Down", L"Direcional ↓"}, {"Left", L"Direcional ←"}, {"Right", L"Direcional →"},
    {"Triangle", L"Triângulo"}, {"Circle", L"Círculo"}, {"Cross", L"X"}, {"Square", L"Quadrado"},
    {"Select", L"Select"}, {"Start", L"Start"}, {"L1", L"L1"}, {"L2", L"L2"},
    {"R1", L"R1"}, {"R2", L"R2"}, {"L3", L"L3"}, {"R3", L"R3"},
    {"LUp", L"Analógico esq. ↑"}, {"LDown", L"Analógico esq. ↓"}, {"LLeft", L"Analógico esq. ←"}, {"LRight", L"Analógico esq. →"},
    {"RUp", L"Analógico dir. ↑"}, {"RDown", L"Analógico dir. ↓"}, {"RLeft", L"Analógico esq. ←"}, {"RRight", L"Analógico dir. →"},
};
static const int kPadButtonCount = static_cast<int>(sizeof(kPadButtons) / sizeof(kPadButtons[0]));

static void buildPageControls()
{
    const int page = PAGE_CONTROLS;
    int y = 14;
    makeLabel(page, L"Jogador:", kCol1, y, 60);
    makeCombo(page, "padSelect", kCol1 + 62, y - 4, 120);
    static const wchar_t *pads[] = {L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8"};
    comboAdd("padSelect", pads, 8);
    makeLabel(page, L"Controle:", kCol1 + 200, y, 80);
    makeCombo(page, "padType", kCol1 + 282, y - 4, 150);
    static const wchar_t *types[] = {L"DualShock 2", L"Nenhum (desligado)"};
    comboAdd("padType", types, 2);
    makeButton(page, "defaultsThisPad", L"Padrão deste jogador", kCol1 + 450, y - 4, 160);
    y += kRowH + 50;

    static const struct { const char *name; const wchar_t *label; int w; } numbers[] = {
        {"deadzone", L"Zona morta", 60}, {"axisScale", L"Escala do eixo", 60}, {"buttonDeadzone", L"Zona morta dos botões", 60},
        {"invertL", L"Inverter anal. esq. (0=não, 1=X, 2=Y, 3=ambos)", 50}, {"invertR", L"Inverter anal. dir.", 50},
        {"largeMotor", L"Força do motor grande", 60}, {"smallMotor", L"Força do motor pequeno", 60},
    };
    // Duas fileiras de campos numéricos para não estourar a largura da janela.
    const int rowWidth = 190;
    for (size_t i = 0; i < sizeof(numbers) / sizeof(numbers[0]); ++i)
    {
        const int fieldRow = static_cast<int>(i) / 4;
        const int column = static_cast<int>(i) % 4;
        const int x = kCol1 + column * rowWidth;
        const int rowY = y + fieldRow * 44;
        makeLabel(page, numbers[i].label, x, rowY, rowWidth - 8);
        makeSpinlessNumber(page, numbers[i].name, x, rowY + 18, numbers[i].w, L"");
    }
    makeLabel(page, L" Ligações (separadas por ; - qualquer uma pressiona o botão):", kCol1, y, 620);
    y += 20;
    // Grid em duas colunas: 12 botões por coluna.
    const int rows = (kPadButtonCount + 1) / 2;
    for (int i = 0; i < kPadButtonCount; ++i)
    {
        const int column = i < rows ? 0 : 1;
        const int row = i < rows ? i : i - rows;
        const int bx = kCol1 + column * 320;
        const int by = y + row * 26;
        makeLabel(page, kPadButtons[i].label, bx, by, 110);
        makeEdit(page, (std::string("bind_") + kPadButtons[i].key).c_str(), L"", bx + 112, by - 2, 200);
    }
    y += rows * 26 + 8;
    makeLabel(page, L"Exemplos: Keyboard/W  |  SDL-0/FaceSouth (X do controle 1)  |  SDL-0/+LeftTrigger  |  SDL-0/-LeftY", kCol1, y, 640);
}

// ---------------------------------------------------------------------------
// Persistência

static void loadSettings()
{
    std::ifstream stream(narrow(g_launcherIni));
    if (!stream)
        return;
    std::string line;
    std::string section;
    while (std::getline(stream, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        if (line.front() == '[' && line.back() == ']')
        {
            section = line.substr(1, line.size() - 2);
            continue;
        }
        const size_t equals = line.find('=');
        if (equals == std::string::npos)
            continue;
        const std::string key = trim(line.substr(0, equals));
        const std::string value = trim(line.substr(equals + 1));
        if (section == "Jogo")
        {
            if (key == "iso") g_settings.iso = widen(value);
            else if (key == "elf") g_settings.elf = widen(value);
            else if (key == "pnach") g_settings.pnach = widen(value);
            else if (key == "hdTextures") g_settings.hdTextures = value == "1";
            else if (key == "textureFolder") g_settings.textureFolder = widen(value);
            else if (key == "texturePrecache") g_settings.texturePrecache = value == "1";
        }
        else if (section == "Video")
        {
            if (key == "renderer") g_settings.renderer = std::atoi(value.c_str());
            else if (key == "scale") g_settings.scale = std::atoi(value.c_str());
            else if (key == "vsync") g_settings.vsync = std::atoi(value.c_str());
            else if (key == "stretch") g_settings.stretch = value == "1";
            else if (key == "fxaa") g_settings.fxaa = value == "1";
            else if (key == "textureFiltering") g_settings.textureFiltering = std::atoi(value.c_str());
            else if (key == "trilinear") g_settings.trilinear = std::atoi(value.c_str());
            else if (key == "anisotropy") g_settings.anisotropy = std::atoi(value.c_str());
            else if (key == "cas") g_settings.cas = std::atoi(value.c_str());
            else if (key == "casSharpness") g_settings.casSharpness = std::atoi(value.c_str());
            else if (key == "shadeboost") g_settings.shadeboost = value == "1";
            else if (key == "brightness") g_settings.brightness = std::atoi(value.c_str());
            else if (key == "contrast") g_settings.contrast = std::atoi(value.c_str());
            else if (key == "saturation") g_settings.saturation = std::atoi(value.c_str());
            else if (key == "gamma") g_settings.gamma = std::atoi(value.c_str());
            else if (key == "tvShader") g_settings.tvShader = std::atoi(value.c_str());
            else if (key == "presentFilter") g_settings.presentFilter = std::atoi(value.c_str());
            else if (key == "dithering") g_settings.dithering = std::atoi(value.c_str());
            else if (key == "blending") g_settings.blending = std::atoi(value.c_str());
        }
        else if (section == "Audio")
        {
            if (key == "mute") g_settings.mute = value == "1";
            else if (key == "rateControl") g_settings.audioRateControl = value != "0";
            else if (key == "stretch") g_settings.audioStretch = value != "0";
            else if (key == "stats") g_settings.audioStats = value == "1";
            else if (key == "dump") g_settings.audioDump = widen(value);
        }
        else if (section == "Sistema")
        {
            if (key == "showFps") g_settings.showFps = value == "1";
            else if (key == "vsyncLock") g_settings.vsyncLock = value != "0";
            else if (key == "mtvu") g_settings.mtvu = value != "0";
            else if (key == "mtvuStream") g_settings.mtvuStream = value != "0";
            else if (key == "sdlInput") g_settings.sdlInput = value != "0";
            else if (key == "gameMenu") g_settings.gameMenu = value != "0";
            else if (key == "eeLoopCycles") g_settings.eeLoopCycles = std::atoi(value.c_str());
            else if (key == "multitap") g_settings.multitap = std::atoi(value.c_str());
        }
        else if (section.rfind("Pad", 0) == 0 && section.size() == 4)
        {
            const int pad = std::atoi(section.c_str() + 3) - 1;
            if (pad < 0 || pad >= static_cast<int>(g_settings.pads.size()))
                continue;
            PadState &state = g_settings.pads[pad];
            if (key == "TypeNone") state.typeNone = value == "1";
            else if (key == "Deadzone") std::snprintf(state.deadzone, sizeof(state.deadzone), "%s", value.c_str());
            else if (key == "AxisScale") std::snprintf(state.axisScale, sizeof(state.axisScale), "%s", value.c_str());
            else if (key == "ButtonDeadzone") std::snprintf(state.buttonDeadzone, sizeof(state.buttonDeadzone), "%s", value.c_str());
            else if (key == "InvertL") std::snprintf(state.invertL, sizeof(state.invertL), "%s", value.c_str());
            else if (key == "InvertR") std::snprintf(state.invertR, sizeof(state.invertR), "%s", value.c_str());
            else if (key == "LargeMotorScale") std::snprintf(state.largeMotor, sizeof(state.largeMotor), "%s", value.c_str());
            else if (key == "SmallMotorScale") std::snprintf(state.smallMotor, sizeof(state.smallMotor), "%s", value.c_str());
            else
            {
                bool found = false;
                for (const auto &button : kPadButtons)
                    if (key == button.key) { found = true; break; }
                if (found)
                {
                    bool exists = false;
                    for (auto &binding : state.bindings)
                        if (binding.first == key) { binding.second = value; exists = true; break; }
                    if (!exists)
                        state.bindings.emplace_back(key, value);
                }
            }
        }
    }
}

static void saveSettings()
{
    std::ofstream stream(narrow(g_launcherIni), std::ios::trunc);
    if (!stream)
        return;
    stream << "# Champions of Norrath - launcher\n";
    stream << "[Jogo]\n";
    stream << "iso=" << narrow(g_settings.iso) << "\n";
    stream << "elf=" << narrow(g_settings.elf) << "\n";
    stream << "pnach=" << narrow(g_settings.pnach) << "\n";
    stream << "hdTextures=" << (g_settings.hdTextures ? 1 : 0) << "\n";
    stream << "textureFolder=" << narrow(g_settings.textureFolder) << "\n";
    stream << "texturePrecache=" << (g_settings.texturePrecache ? 1 : 0) << "\n";
    stream << "[Video]\n";
    stream << "renderer=" << g_settings.renderer << "\n";
    stream << "scale=" << g_settings.scale << "\n";
    stream << "vsync=" << g_settings.vsync << "\n";
    stream << "stretch=" << (g_settings.stretch ? 1 : 0) << "\n";
    stream << "fxaa=" << (g_settings.fxaa ? 1 : 0) << "\n";
    stream << "textureFiltering=" << g_settings.textureFiltering << "\n";
    stream << "trilinear=" << g_settings.trilinear << "\n";
    stream << "anisotropy=" << g_settings.anisotropy << "\n";
    stream << "cas=" << g_settings.cas << "\n";
    stream << "casSharpness=" << g_settings.casSharpness << "\n";
    stream << "shadeboost=" << (g_settings.shadeboost ? 1 : 0) << "\n";
    stream << "brightness=" << g_settings.brightness << "\n";
    stream << "contrast=" << g_settings.contrast << "\n";
    stream << "saturation=" << g_settings.saturation << "\n";
    stream << "gamma=" << g_settings.gamma << "\n";
    stream << "tvShader=" << g_settings.tvShader << "\n";
    stream << "presentFilter=" << g_settings.presentFilter << "\n";
    stream << "dithering=" << g_settings.dithering << "\n";
    stream << "blending=" << g_settings.blending << "\n";
    stream << "[Audio]\n";
    stream << "mute=" << (g_settings.mute ? 1 : 0) << "\n";
    stream << "rateControl=" << (g_settings.audioRateControl ? 1 : 0) << "\n";
    stream << "stretch=" << (g_settings.audioStretch ? 1 : 0) << "\n";
    stream << "stats=" << (g_settings.audioStats ? 1 : 0) << "\n";
    stream << "dump=" << narrow(g_settings.audioDump) << "\n";
    stream << "[Sistema]\n";
    stream << "showFps=" << (g_settings.showFps ? 1 : 0) << "\n";
    stream << "vsyncLock=" << (g_settings.vsyncLock ? 1 : 0) << "\n";
    stream << "mtvu=" << (g_settings.mtvu ? 1 : 0) << "\n";
    stream << "mtvuStream=" << (g_settings.mtvuStream ? 1 : 0) << "\n";
    stream << "sdlInput=" << (g_settings.sdlInput ? 1 : 0) << "\n";
    stream << "gameMenu=" << (g_settings.gameMenu ? 1 : 0) << "\n";
    stream << "eeLoopCycles=" << g_settings.eeLoopCycles << "\n";
    stream << "multitap=" << g_settings.multitap << "\n";
    for (size_t i = 0; i < g_settings.pads.size(); ++i)
    {
        const PadState &state = g_settings.pads[i];
        stream << "[Pad" << (i + 1) << "]\n";
        stream << "TypeNone=" << (state.typeNone ? 1 : 0) << "\n";
        stream << "Deadzone=" << state.deadzone << "\n";
        stream << "AxisScale=" << state.axisScale << "\n";
        stream << "ButtonDeadzone=" << state.buttonDeadzone << "\n";
        stream << "InvertL=" << state.invertL << "\n";
        stream << "InvertR=" << state.invertR << "\n";
        stream << "LargeMotorScale=" << state.largeMotor << "\n";
        stream << "SmallMotorScale=" << state.smallMotor << "\n";
        for (const auto &binding : state.bindings)
            stream << binding.first << "=" << binding.second << "\n";
    }
}

static void loadPadDefaults(int padIndex)
{
    PadState &state = g_settings.pads[padIndex];
    state.typeNone = false;
    std::snprintf(state.deadzone, sizeof(state.deadzone), "0");
    std::snprintf(state.axisScale, sizeof(state.axisScale), "1.33");
    std::snprintf(state.buttonDeadzone, sizeof(state.buttonDeadzone), "0");
    std::snprintf(state.invertL, sizeof(state.invertL), "0");
    std::snprintf(state.invertR, sizeof(state.invertR), "0");
    std::snprintf(state.largeMotor, sizeof(state.largeMotor), "1");
    std::snprintf(state.smallMotor, sizeof(state.smallMotor), "1");
    state.bindings.clear();
    const std::string sdl = "SDL-" + std::to_string(padIndex) + "/";
    const bool keyboard = padIndex == 0;
    auto add = [&](const char *key, const char *keyboardBinding, const char *element)
    {
        std::string value;
        if (keyboard && keyboardBinding)
            value = std::string("Keyboard/") + keyboardBinding;
        if (!value.empty())
            value += ";";
        value += sdl + element;
        state.bindings.emplace_back(key, value);
    };
    add("Up", nullptr, "DPadUp");
    add("Down", nullptr, "DPadDown");
    add("Left", nullptr, "DPadLeft");
    add("Right", nullptr, "DPadRight");
    add("Triangle", "Up", "FaceNorth");
    add("Circle", "Right", "FaceEast");
    add("Cross", "Down", "FaceSouth");
    add("Square", "Left", "FaceWest");
    add("Select", "Tab", "Back");
    add("Start", "Return", "Start");
    add("L1", "Q", "LeftShoulder");
    add("L2", "1", "+LeftTrigger");
    add("R1", "E", "RightShoulder");
    add("R2", "3", "+RightTrigger");
    add("L3", "F", "LeftStick");
    add("R3", "G", "RightStick");
    add("LUp", "W", "-LeftY");
    add("LDown", "S", "+LeftY");
    add("LLeft", "A", "-LeftX");
    add("LRight", "D", "+LeftX");
    add("RUp", nullptr, "-RightY");
    add("RDown", nullptr, "+RightY");
    add("RLeft", nullptr, "-RightX");
    add("RRight", nullptr, "+RightX");
}

// ---------------------------------------------------------------------------
// UI <-> estado

static void refreshControlsTab()
{
    const int pad = g_settings.currentPad;
    const PadState &state = g_settings.pads[pad];
    comboSelect("padSelect", pad);
    comboSelect("padType", state.typeNone ? 1 : 0);
    setTextA("deadzone", state.deadzone);
    setTextA("axisScale", state.axisScale);
    setTextA("buttonDeadzone", state.buttonDeadzone);
    setTextA("invertL", state.invertL);
    setTextA("invertR", state.invertR);
    setTextA("largeMotor", state.largeMotor);
    setTextA("smallMotor", state.smallMotor);
    for (const auto &button : kPadButtons)
    {
        const std::string name = std::string("bind_") + button.key;
        std::string value;
        for (const auto &binding : state.bindings)
            if (binding.first == button.key)
            {
                if (!value.empty())
                    value = binding.second;
                else
                    value = binding.second;
            }
        setTextA(name.c_str(), value);
    }
}

static void collectControlsTab()
{
    const int pad = g_settings.currentPad;
    PadState &state = g_settings.pads[pad];
    state.typeNone = comboValue("padType") == 1;
    std::snprintf(state.deadzone, sizeof(state.deadzone), "%s", narrow(textOf("deadzone")).c_str());
    std::snprintf(state.axisScale, sizeof(state.axisScale), "%s", narrow(textOf("axisScale")).c_str());
    std::snprintf(state.buttonDeadzone, sizeof(state.buttonDeadzone), "%s", narrow(textOf("buttonDeadzone")).c_str());
    std::snprintf(state.invertL, sizeof(state.invertL), "%s", narrow(textOf("invertL")).c_str());
    std::snprintf(state.invertR, sizeof(state.invertR), "%s", narrow(textOf("invertR")).c_str());
    std::snprintf(state.largeMotor, sizeof(state.largeMotor), "%s", narrow(textOf("largeMotor")).c_str());
    std::snprintf(state.smallMotor, sizeof(state.smallMotor), "%s", narrow(textOf("smallMotor")).c_str());
    for (const auto &button : kPadButtons)
    {
        const std::string name = std::string("bind_") + button.key;
        const std::string value = narrow(textOf(name.c_str()));
        bool exists = false;
        for (auto &binding : state.bindings)
            if (binding.first == button.key) { binding.second = value; exists = true; break; }
        if (!exists && !value.empty())
            state.bindings.emplace_back(button.key, value);
    }
}

static void uiToSettings()
{
    g_settings.iso = textOf("iso");
    g_settings.elf = textOf("elf");
    g_settings.pnach = textOf("pnach");
    g_settings.hdTextures = checked("hdTextures");
    g_settings.textureFolder = textOf("textureFolder");
    g_settings.texturePrecache = checked("texturePrecache");

    g_settings.renderer = comboValue("renderer");
    g_settings.scale = comboValue("scale") + 1;
    g_settings.vsync = comboValue("vsync");
    g_settings.stretch = checked("stretch");
    g_settings.fxaa = checked("fxaa");
    g_settings.textureFiltering = comboValue("textureFiltering") - 1;
    g_settings.trilinear = comboValue("trilinear") - 1;
    g_settings.anisotropy = comboValue("anisotropy") - 1;
    g_settings.cas = comboValue("cas");
    g_settings.casSharpness = sliderValue("casSharpness");
    g_settings.shadeboost = checked("shadeboost");
    g_settings.brightness = sliderValue("brightness");
    g_settings.contrast = sliderValue("contrast");
    g_settings.saturation = sliderValue("saturation");
    g_settings.gamma = sliderValue("gamma");
    g_settings.tvShader = comboValue("tvShader");
    g_settings.presentFilter = comboValue("presentFilter") - 1;
    g_settings.dithering = comboValue("dithering") - 1;
    g_settings.blending = comboValue("blending") - 1;

    g_settings.mute = checked("mute");
    g_settings.audioRateControl = checked("audioRateControl");
    g_settings.audioStretch = checked("audioStretch");
    g_settings.audioStats = checked("audioStats");
    g_settings.audioDump = textOf("audioDump");

    g_settings.showFps = checked("showFps");
    g_settings.vsyncLock = checked("vsyncLock");
    g_settings.mtvu = checked("mtvu");
    g_settings.mtvuStream = checked("mtvuStream");
    g_settings.sdlInput = checked("sdlInput");
    g_settings.gameMenu = checked("gameMenu");
    g_settings.eeLoopCycles = std::atoi(narrow(textOf("eeLoopCycles")).c_str());
    if (g_settings.eeLoopCycles <= 0)
        g_settings.eeLoopCycles = 16;
    g_settings.multitap = (checked("multitap1") ? 1 : 0) | (checked("multitap2") ? 2 : 0);
    collectControlsTab();
}

static void settingsToUi()
{
    setText("iso", g_settings.iso);
    setText("elf", g_settings.elf);
    setText("pnach", g_settings.pnach);
    check("hdTextures", g_settings.hdTextures);
    setText("textureFolder", g_settings.textureFolder);
    check("texturePrecache", g_settings.texturePrecache);

    comboSelect("renderer", g_settings.renderer);
    comboSelect("scale", g_settings.scale - 1);
    comboSelect("vsync", g_settings.vsync);
    check("stretch", g_settings.stretch);
    check("fxaa", g_settings.fxaa);
    comboSelect("textureFiltering", g_settings.textureFiltering + 1);
    comboSelect("trilinear", g_settings.trilinear + 1);
    comboSelect("anisotropy", g_settings.anisotropy + 1);
    comboSelect("cas", g_settings.cas);
    sliderSet("casSharpness", g_settings.casSharpness);
    check("shadeboost", g_settings.shadeboost);
    sliderSet("brightness", g_settings.brightness);
    sliderSet("contrast", g_settings.contrast);
    sliderSet("saturation", g_settings.saturation);
    sliderSet("gamma", g_settings.gamma);
    comboSelect("tvShader", g_settings.tvShader);
    comboSelect("presentFilter", g_settings.presentFilter + 1);
    comboSelect("dithering", g_settings.dithering + 1);
    comboSelect("blending", g_settings.blending + 1);

    check("mute", g_settings.mute);
    check("audioRateControl", g_settings.audioRateControl);
    check("audioStretch", g_settings.audioStretch);
    check("audioStats", g_settings.audioStats);
    setText("audioDump", g_settings.audioDump);

    check("showFps", g_settings.showFps);
    check("vsyncLock", g_settings.vsyncLock);
    check("mtvu", g_settings.mtvu);
    check("mtvuStream", g_settings.mtvuStream);
    check("sdlInput", g_settings.sdlInput);
    check("gameMenu", g_settings.gameMenu);
    setTextA("eeLoopCycles", std::to_string(g_settings.eeLoopCycles));
    check("multitap1", (g_settings.multitap & 1) != 0);
    check("multitap2", (g_settings.multitap & 2) != 0);
    refreshControlsTab();
}

// ---------------------------------------------------------------------------
// controls.ini + ambiente do jogo

static void writeControlsIni()
{
    std::ofstream stream(narrow(g_controlsIni), std::ios::trunc);
    if (!stream)
        return;
    stream << "# Champions of Norrath - controles (escrito pelo launcher).\n";
    stream << "# O launcher aplica estes valores como variáveis PS2X_PAD* no con.exe;\n";
    stream << "# o arquivo fica aqui para você consultar ou levar para outro PCSX2.\n";
    stream << "[InputSources]\n";
    stream << "SDL = " << (g_settings.sdlInput ? "true" : "false") << "\n\n";
    stream << "[Pad]\n";
    stream << "MultitapPort1 = " << ((g_settings.multitap & 1) ? "true" : "false") << "\n";
    stream << "MultitapPort2 = " << ((g_settings.multitap & 2) ? "true" : "false") << "\n";
    for (size_t i = 0; i < g_settings.pads.size(); ++i)
    {
        const PadState &state = g_settings.pads[i];
        stream << "\n[Pad" << (i + 1) << "]\n";
        stream << "Type = " << (state.typeNone ? "None" : "DualShock2") << "\n";
        stream << "InvertL = " << state.invertL << "\n";
        stream << "InvertR = " << state.invertR << "\n";
        stream << "Deadzone = " << state.deadzone << "\n";
        stream << "AxisScale = " << state.axisScale << "\n";
        stream << "ButtonDeadzone = " << state.buttonDeadzone << "\n";
        stream << "LargeMotorScale = " << state.largeMotor << "\n";
        stream << "SmallMotorScale = " << state.smallMotor << "\n";
        for (const auto &button : kPadButtons)
            for (const auto &binding : state.bindings)
                if (binding.first == button.key)
                    stream << button.key << " = " << binding.second << "\n";
    }
}

static void setEnvOrClear(const wchar_t *name, const std::wstring &value)
{
    if (value.empty())
        SetEnvironmentVariableW(name, nullptr);
    else
        SetEnvironmentVariableW(name, value.c_str());
}

static void applyEnvironment()
{
    setEnvOrClear(L"PS2X_GS_RENDERER", g_settings.renderer == 1 ? L"nativo" : L"pcsx2");
    setEnvOrClear(L"PS2X_GS_SCALE", std::to_wstring(g_settings.scale));
    setEnvOrClear(L"PS2X_PCSX2_VSYNC", std::to_wstring(g_settings.vsync));
    setEnvOrClear(L"PS2X_PCSX2_STRETCH", g_settings.stretch ? L"1" : L"0");
    setEnvOrClear(L"PS2X_PCSX2_FXAA", g_settings.fxaa ? L"1" : L"0");
    setEnvOrClear(L"PS2X_PCSX2_TEXTURE_FILTERING", g_settings.textureFiltering >= 0 ? std::to_wstring(g_settings.textureFiltering) : L"");
    setEnvOrClear(L"PS2X_PCSX2_TRILINEAR", g_settings.trilinear >= 0 ? std::to_wstring(g_settings.trilinear) : L"");
    setEnvOrClear(L"PS2X_PCSX2_ANISOTROPY", g_settings.anisotropy >= 0 ? std::to_wstring(g_settings.anisotropy) : L"");
    setEnvOrClear(L"PS2X_PCSX2_CAS", std::to_wstring(g_settings.cas));
    setEnvOrClear(L"PS2X_PCSX2_CAS_SHARPNESS", g_settings.cas ? std::to_wstring(g_settings.casSharpness) : L"");
    setEnvOrClear(L"PS2X_PCSX2_SHADEBOOST", g_settings.shadeboost ? L"1" : L"0");
    setEnvOrClear(L"PS2X_PCSX2_BRIGHTNESS", g_settings.shadeboost ? std::to_wstring(g_settings.brightness) : L"");
    setEnvOrClear(L"PS2X_PCSX2_CONTRAST", g_settings.shadeboost ? std::to_wstring(g_settings.contrast) : L"");
    setEnvOrClear(L"PS2X_PCSX2_SATURATION", g_settings.shadeboost ? std::to_wstring(g_settings.saturation) : L"");
    setEnvOrClear(L"PS2X_PCSX2_GAMMA", g_settings.shadeboost ? std::to_wstring(g_settings.gamma) : L"");
    setEnvOrClear(L"PS2X_PCSX2_TV_SHADER", std::to_wstring(g_settings.tvShader));
    setEnvOrClear(L"PS2X_PCSX2_PRESENT_FILTER", g_settings.presentFilter >= 0 ? std::to_wstring(g_settings.presentFilter) : L"");
    setEnvOrClear(L"PS2X_PCSX2_DITHERING", g_settings.dithering >= 0 ? std::to_wstring(g_settings.dithering) : L"");
    setEnvOrClear(L"PS2X_PCSX2_BLENDING", g_settings.blending >= 0 ? std::to_wstring(g_settings.blending) : L"");
    setEnvOrClear(L"PS2X_PCSX2_TEXTURES", g_settings.hdTextures ? L"1" : L"0");
    setEnvOrClear(L"PS2X_PCSX2_TEXTURE_DIR", g_settings.textureFolder);
    setEnvOrClear(L"PS2X_PCSX2_TEXTURE_PRECACHE", g_settings.texturePrecache ? L"1" : L"0");

    setEnvOrClear(L"PS2X_AUDIO_MUTE", g_settings.mute ? L"1" : L"0");
    setEnvOrClear(L"PS2X_AUDIO_RATE_CONTROL", g_settings.audioRateControl ? L"1" : L"0");
    setEnvOrClear(L"PS2X_AUDIO_STRETCH", g_settings.audioStretch ? L"1" : L"0");
    setEnvOrClear(L"PS2X_AUDIO_STATS", g_settings.audioStats ? L"1" : L"0");
    setEnvOrClear(L"PS2X_AUDIO_DUMP", g_settings.audioDump);

    setEnvOrClear(L"PS2X_SHOW_FPS", g_settings.showFps ? L"1" : L"0");
    setEnvOrClear(L"PS2X_VSYNC_LOCK", g_settings.vsyncLock ? L"1" : L"0");
    setEnvOrClear(L"PS2X_MTVU", g_settings.mtvu ? L"1" : L"0");
    setEnvOrClear(L"PS2X_MTVU_STREAM", g_settings.mtvuStream ? L"1" : L"0");
    setEnvOrClear(L"PS2X_SDL_INPUT", g_settings.sdlInput ? L"1" : L"0");
    setEnvOrClear(L"PS2X_GAME_MENU", g_settings.gameMenu ? L"1" : L"0");
    setEnvOrClear(L"PS2X_EE_LOOP_CYCLES", std::to_wstring(g_settings.eeLoopCycles));
    setEnvOrClear(L"PS2X_MULTITAP", std::to_wstring(g_settings.multitap));
    setEnvOrClear(L"PS2X_PNACH", g_settings.pnach);
    setEnvOrClear(L"PS2X_CD_IMAGE", g_settings.iso);

    for (size_t pad = 0; pad < g_settings.pads.size(); ++pad)
    {
        const std::string prefixA = "PS2X_PAD" + std::to_string(pad + 1);
        const std::wstring prefix = widen(prefixA);
        const PadState &state = g_settings.pads[pad];
        setEnvOrClear(prefix.c_str(), state.typeNone ? L"" : L"1");
        auto push = [&](const wchar_t *suffix, const std::string &value)
        {
            setEnvOrClear((prefix + suffix).c_str(), widen(value));
        };
        push(L"_TYPE", state.typeNone ? "None" : "DualShock2");
        push(L"_DEADZONE", state.deadzone);
        push(L"_AXISSCALE", state.axisScale);
        push(L"_BUTTONDEADZONE", state.buttonDeadzone);
        push(L"_INVERTL", state.invertL);
        push(L"_INVERTR", state.invertR);
        push(L"_LARGEMOTORSCALE", state.largeMotor);
        push(L"_SMALLMOTORSCALE", state.smallMotor);
        for (const auto &binding : state.bindings)
            push(widen("_" + lower(binding.first)).c_str(), binding.second);
    }
}

// ---------------------------------------------------------------------------
// JOGAR

static void playGame()
{
    uiToSettings();
    saveSettings();
    writeControlsIni();
    applyEnvironment();

    if (g_settings.iso.empty() || !fileExists(g_settings.iso))
    {
        if (g_main)
            MessageBoxW(g_main, L"Escolha a imagem do disco (.iso) do jogo na aba Jogo.", L"Falta a ISO", MB_ICONWARNING);
        else
            logLine(L"ERRO: ISO não encontrada (" + g_settings.iso + L")");
        return;
    }
    std::wstring elf = g_settings.elf;
    if (elf.empty() || !fileExists(elf))
    {
        if (g_main)
            MessageBoxW(g_main, L"Escolha o executável do jogo (ELF, SLUS_205.65) na aba Jogo.", L"Falta o ELF", MB_ICONWARNING);
        else
            logLine(L"ERRO: ELF não encontrado");
        return;
    }
    const std::wstring game = g_exeDir + L"\\con.exe";
    if (!fileExists(game))
    {
        if (g_main)
            MessageBoxW(g_main, L"con.exe não está na pasta do launcher.", L"Falta o con.exe", MB_ICONERROR);
        else
            logLine(L"ERRO: con.exe não encontrado");
        return;
    }

    std::wstring command = L"\"" + game + L"\" \"" + elf + L"\"";
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(game.c_str(), command.data(), nullptr, nullptr, FALSE, 0, nullptr, g_exeDir.c_str(), &si, &pi))
    {
        if (g_main)
            MessageBoxW(g_main, L"Não foi possível iniciar o jogo.", L"Erro", MB_ICONERROR);
        return;
    }
    CloseHandle(pi.hThread);
    if (g_main)
        statusLine(L"Jogo rodando...");
    logLine(L"JOGAR: " + command);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    wchar_t line[256];
    std::swprintf(line, 256, L"Jogo encerrou (código %lu).", code);
    statusLine(line);
    logLine(line);
}

// ---------------------------------------------------------------------------
// Janela

static void autoDetectIso(bool quiet)
{
    if (!g_settings.iso.empty() && fileExists(g_settings.iso))
        return;
    WIN32_FIND_DATAW data{};
    const std::wstring search = g_exeDir + L"\\*.iso";
    const HANDLE find = FindFirstFileW(search.c_str(), &data);
    if (find == INVALID_HANDLE_VALUE)
    {
        logLine(L"autoDetectIso: FindFirstFileW falhou em " + search + L" (erro " + std::to_wstring(GetLastError()) + L")");
        return;
    }
    do
    {
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;
        if (data.nFileSizeLow < (64u << 20) && data.nFileSizeHigh == 0)
            continue; // ignora ISOs minúsculas
        const std::wstring candidate = g_exeDir + L"\\" + data.cFileName;
        if (fileExists(candidate) && (g_settings.iso.empty() || candidate < g_settings.iso))
            g_settings.iso = candidate;
    } while (FindNextFileW(find, &data));
    FindClose(find);
    if (!quiet && !g_settings.iso.empty())
        statusLine(L"ISO detectada: " + g_settings.iso);
}

static const wchar_t *kWindowClass = L"CoNRecompLauncher";

static LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_TAB_CLASSES | ICC_STANDARD_CLASSES};
        InitCommonControlsEx(&controls);

        RECT area{};
        GetClientRect(window, &area);

        g_tab = CreateWindowExW(0, WC_TABCONTROLW, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                8, 8, area.right - 16, 28, window, nullptr, nullptr, nullptr);
        TCITEMW item{};
        item.mask = TCIF_TEXT;
        for (int i = 0; i < PAGE_COUNT; ++i)
        {
            item.pszText = const_cast<wchar_t *>(kTabTitles[i]);
            SendMessageW(g_tab, TCM_INSERTITEM, i, reinterpret_cast<LPARAM>(&item));
        }

        buildPageGame();
        buildPageVideo();
        buildPageAudioSys();
        buildPageControls();
        settingsToUi();

        const int contentTop = 44;
        const int bottomHeight = 62;
        MoveWindow(g_tab, 8, 8, area.right - 16, 28, TRUE);

        g_play = CreateWindowExW(0, L"BUTTON", L"JOGAR", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                                 area.right - 178, area.bottom - bottomHeight - 6, 160, 40, window,
                                 reinterpret_cast<HMENU>(1001), nullptr, nullptr);
        HWND save = CreateWindowExW(0, L"BUTTON", L"Salvar", WS_CHILD | WS_VISIBLE,
                                    12, area.bottom - bottomHeight - 6, 90, 24, window,
                                    reinterpret_cast<HMENU>(1002), nullptr, nullptr);
        HWND reset = CreateWindowExW(0, L"BUTTON", L"Padrões", WS_CHILD | WS_VISIBLE,
                                     110, area.bottom - bottomHeight - 6, 90, 24, window,
                                     reinterpret_cast<HMENU>(1003), nullptr, nullptr);
        g_status = CreateWindowExW(0, STATUSCLASSNAMEW, L"", WS_CHILD | WS_VISIBLE | SBT_NOBORDERS,
                                   0, 0, 0, 0, window, nullptr, nullptr, nullptr);
        (void)save;
        (void)reset;
        SetWindowPos(g_status, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        applyFontRecursive(window);
        showPage(PAGE_GAME);
        return 0;
    }
    case WM_SIZE:
    {
        const int width = LOWORD(lparam);
        const int height = HIWORD(lparam);
        const int bottomHeight = 62;
        MoveWindow(g_tab, 8, 8, width - 16, 28, TRUE);
        SetWindowPos(g_play, nullptr, width - 178, height - bottomHeight - 6, 160, 40, SWP_NOZORDER);
        SendMessageW(g_status, WM_SIZE, 0, 0);
        const int contentHeight = height - 44 - bottomHeight - 6;
        for (int page = 0; page < PAGE_COUNT; ++page)
            for (HWND control : g_pages[page])
                SetWindowPos(control, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOZORDER);
        (void)contentHeight;
        return 0;
    }
    case WM_NOTIFY:
    {
        const LPNMHDR header = reinterpret_cast<LPNMHDR>(lparam);
        if (header->hwndFrom == g_tab && header->code == TCN_SELCHANGE)
        {
            const int index = static_cast<int>(SendMessageW(g_tab, TCM_GETCURSEL, 0, 0));
            if (index >= 0 && index < PAGE_COUNT)
            {
                if (index != PAGE_CONTROLS)
                    collectControlsTab();
                else
                    refreshControlsTab();
                showPage(index);
            }
        }
        return 0;
    }
    case WM_DRAWITEM:
    {
        const LPDRAWITEMSTRUCT item = reinterpret_cast<LPDRAWITEMSTRUCT>(lparam);
        if (item->hwndItem == g_play)
        {
            const bool pressed = (item->itemState & ODS_SELECTED) != 0;
            const HBRUSH brush = CreateSolidBrush(pressed ? RGB(20, 110, 40) : RGB(28, 140, 52));
            FillRect(item->hDC, &item->rcItem, brush);
            DeleteObject(brush);
            SetBkMode(item->hDC, TRANSPARENT);
            SetTextColor(item->hDC, RGB(255, 255, 255));
            DrawTextW(item->hDC, L"JOGAR", -1, const_cast<RECT *>(&item->rcItem),
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        return TRUE;
    }
    case WM_COMMAND:
    {
        const int control = LOWORD(wparam);
        const int code = HIWORD(wparam);
        if (control == 1001)
        {
            playGame();
        }
        else if (control == 1002)
        {
            uiToSettings();
            saveSettings();
            statusLine(L"Configurações salvas.");
        }
        else if (control == 1003)
        {
            for (int pad = 0; pad < static_cast<int>(g_settings.pads.size()); ++pad)
                loadPadDefaults(pad);
            refreshControlsTab();
            statusLine(L"Controles nos valores padrão do runtime.");
        }
        else if (code == CBN_SELCHANGE)
        {
            const HWND source = reinterpret_cast<HWND>(lparam);
            if (source == widget("padSelect"))
            {
                collectControlsTab();
                g_settings.currentPad = comboValue("padSelect");
                refreshControlsTab();
            }
            else if (source == widget("padType"))
            {
                g_settings.pads[g_settings.currentPad].typeNone = comboValue("padType") == 1;
            }
        }
        else if (code == BN_CLICKED)
        {
            const HWND source = reinterpret_cast<HWND>(lparam);
            if (source == widget("browseIso"))
            {
                const std::wstring picked = browseForFile(window, L"Escolha a imagem do disco", L"Imagens de disco\0*.iso;*.bin;*.iso.gz\0Todos os arquivos\0*.*\0", L"iso");
                if (!picked.empty())
                {
                    g_settings.iso = picked;
                    setText("iso", picked);
                }
            }
            else if (source == widget("browseElf"))
            {
                const std::wstring picked = browseForFile(window, L"Escolha o executável do jogo (ELF)", L"ELF\0SLUS_205.65;*.elf\0Todos os arquivos\0*.*\0", L"elf");
                if (!picked.empty())
                {
                    g_settings.elf = picked;
                    setText("elf", picked);
                }
            }
            else if (source == widget("browsePnach"))
            {
                const std::wstring picked = browseForFile(window, L"Escolha um patch do PCSX2", L"Patches\0*.pnach\0Todos os arquivos\0*.*\0", L"pnach");
                if (!picked.empty())
                {
                    g_settings.pnach = picked;
                    setText("pnach", picked);
                }
            }
            else if (source == widget("browseAudioDump"))
            {
                const std::wstring picked = browseForFile(window, L"Arquivo para gravar o áudio", L"Áudio\0*.wav\0Todos os arquivos\0*.*\0", L"wav");
                if (!picked.empty())
                {
                    g_settings.audioDump = picked;
                    setText("audioDump", picked);
                }
            }
            else if (source == widget("browseTextures"))
            {
                const std::wstring picked = browseForFolder(window, L"Pasta das texturas HD (com \"replacements\")");
                if (!picked.empty())
                {
                    g_settings.textureFolder = picked;
                    setText("textureFolder", picked);
                }
            }
            else if (source == widget("openFolder"))
            {
                ShellExecuteW(window, L"open", g_exeDir.c_str(), nullptr, nullptr, SW_SHOW);
            }
            else if (source == widget("detectIso"))
            {
                autoDetectIso(false);
                setText("iso", g_settings.iso);
            }
            else if (source == widget("defaultsThisPad"))
            {
                loadPadDefaults(g_settings.currentPad);
                refreshControlsTab();
                statusLine(L"Jogador " + std::to_wstring(g_settings.currentPad + 1) + L" nos valores padrão.");
            }
        }
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(window, message, wparam, lparam);
    }
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int showCommand)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    wchar_t module[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, module, MAX_PATH);
    g_exeDir = folderOf(module);
    g_launcherIni = g_exeDir + L"\\launcher.ini";
    g_controlsIni = g_exeDir + L"\\controls.ini";

    for (int pad = 0; pad < static_cast<int>(g_settings.pads.size()); ++pad)
        loadPadDefaults(pad);
    loadSettings();
    // O ELF padrão é o SLUS_205.65 ao lado do jogo.
    if (g_settings.elf.empty())
        g_settings.elf = g_exeDir + L"\\SLUS_205.65";
    autoDetectIso(true);
    if (g_settings.elf.empty() && fileExists(g_exeDir + L"\\SLUS_205.65"))
        g_settings.elf = g_exeDir + L"\\SLUS_205.65";

    // Launcher.exe --jogar / -jogar: modo sem janela (testes e atalhos).
    const std::wstring commandLine = GetCommandLineW();
    if (commandLine.find(L"--jogar") != std::wstring::npos ||
        commandLine.find(L"-jogar") != std::wstring::npos)
    {
        playGame();
        return 0;
    }

    g_font = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                         OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                         DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_TAB_CLASSES | ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = kWindowClass;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    RegisterClassExW(&windowClass);

    HWND window = CreateWindowExW(0, kWindowClass, L"Champions of Norrath - PC Port (Launcher)",
                                  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                  CW_USEDEFAULT, CW_USEDEFAULT, 820, 660, nullptr, nullptr, instance, nullptr);
    if (!window)
        return 1;
    g_main = window;
    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
