#include "ATMGui.h"
#include "ATM.h"
#include <sstream>
#include <iomanip>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#define _WIN32_WINNT 0x0A00
#define _WIN32_IE 0x0A00

ATMGui* ATMGui::instance = nullptr;
LRESULT CALLBACK PanelProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_COMMAND || msg == WM_CTLCOLORSTATIC || msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORBTN) {
        return SendMessage(GetParent(hWnd), msg, wParam, lParam);
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

ATMGui::ATMGui(HINSTANCE hInstance, ATM& atm)
    : hInstance(hInstance), atm(atm), hMainWnd(NULL), currentPanel(PANEL_CARD) {
    instance = this;
    for (int i = 0; i < 6; i++) hPanels[i] = NULL;
}

void ATMGui::createFonts() {
    hTitleFont = CreateFontW(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    hNormalFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    hSmallFont = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    hBoldFont = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
}

void ATMGui::createBrushes() {
    hBgBrush = CreateSolidBrush(RGB(15, 40, 80));    // Dark navy
    hPanelBrush = CreateSolidBrush(RGB(25, 60, 110)); // Medium navy
    hBtnBrush = CreateSolidBrush(RGB(0, 120, 215));  // Blue button
    hEditBrush = CreateSolidBrush(RGB(200, 220, 255)); // White edit box
}

HWND MakeLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h, HFONT font, COLORREF color = RGB(255,255,255)) {
    HWND lbl = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_CENTER,
        x, y, w, h, parent, NULL, GetModuleHandle(NULL), NULL);
    SendMessage(lbl, WM_SETFONT, (WPARAM)font, TRUE);
    return lbl;
}

HWND MakeEdit(HWND parent, int id, int x, int y, int w, int h, HFONT font, bool password = false) {
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER;
    if (password) style |= ES_PASSWORD;
    HWND edit = CreateWindowW(L"EDIT", L"", style, x, y, w, h, parent, (HMENU)(UINT_PTR)id, GetModuleHandle(NULL), NULL);
    SendMessage(edit, WM_SETFONT, (WPARAM)font, TRUE);
    return edit;
}

HWND MakeButton(HWND parent, const wchar_t* text, int id, int x, int y, int w, int h, HFONT font) {
    HWND btn = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, w, h, parent, (HMENU)(UINT_PTR)id, GetModuleHandle(NULL), NULL);
    SendMessage(btn, WM_SETFONT, (WPARAM)font, TRUE);
    return btn;
}

HWND MakePanel(HWND parent, int x, int y, int w, int h) {
    return CreateWindowW(L"STATIC", L"", WS_CHILD | SS_BLACKRECT,
        x, y, w, h, parent, NULL, GetModuleHandle(NULL), NULL);
}

bool ATMGui::create() {
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASSW wc = {};

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ATMWindow";
    wc.hbrBackground = CreateSolidBrush(RGB(15, 40, 80));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassW(&wc);
    WNDCLASSW panelWc = {};
    panelWc.lpfnWndProc = PanelProc;
    panelWc.hInstance = hInstance;
    panelWc.lpszClassName = L"ATMPanel";
    panelWc.hbrBackground = CreateSolidBrush(RGB(15, 40, 80));
    RegisterClassW(&panelWc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int winW = 520, winH = 680;
    int startX = (screenW - winW) / 2;
    int startY = (screenH - winH) / 2;

    hMainWnd = CreateWindowExW(0, L"ATMWindow", L"ATM Management System",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        startX, startY, winW, winH,
        NULL, NULL, hInstance, NULL);
    if (!hMainWnd) return false;

    createFonts();
    createBrushes();
    createHeader(hMainWnd);
    createCardPanel();
    createPINPanel();
    createMenuPanel();
    createWithdrawPanel();
    createChangePINPanel();
    createHistoryPanel();

    showPanel(PANEL_CARD);
    ShowWindow(hMainWnd, SW_SHOW);
    UpdateWindow(hMainWnd);
    SetTimer(hMainWnd, 1, 1000, NULL);
    return true;
}

void ATMGui::createHeader(HWND parent) {
    // Header background (drawn via WM_CTLCOLORSTATIC via WndProc)
    hHeaderLabel = CreateWindowW(L"STATIC", L"🏦  ATM Management System",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        0, 0, 504, 70, parent, NULL, hInstance, NULL);
    SendMessage(hHeaderLabel, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

    hStatusLabel = CreateWindowW(L"STATIC", L"Please insert your card to begin.",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        10, 75, 484, 25, parent, (HMENU)IDC_STATUS_LABEL, hInstance, NULL);
    SendMessage(hStatusLabel, WM_SETFONT, (WPARAM)hSmallFont, TRUE);
}

void ATMGui::createCardPanel() {
    hPanels[PANEL_CARD] = CreateWindowW(L"ATMPanel", L"", WS_CHILD,
        30, 115, 444, 450, hMainWnd, NULL, hInstance, NULL);
    HWND p = hPanels[PANEL_CARD];

    MakeLabel(p, L"Insert Your Card", 0, 30, 444, 35, hBoldFont);
    MakeLabel(p, L"Enter your 16-digit card number:", 40, 90, 364, 22, hNormalFont);
    hCardInput = MakeEdit(p, IDC_CARD_INPUT, 80, 120, 284, 36, hNormalFont);
    SendMessage(hCardInput, EM_SETLIMITTEXT, 16, 0);
    hBtnInsert = MakeButton(p, L"  INSERT CARD  ", IDC_BTN_INSERT, 132, 185, 180, 44, hBoldFont);
    MakeLabel(p, L"─────────────────────────────────────", 0, 255, 444, 22, hSmallFont);
    hTimeLabel = MakeLabel(p, L"", 0, 300, 444, 25, hBoldFont);
}

void ATMGui::createPINPanel() {
    hPanels[PANEL_PIN] = CreateWindowW(L"ATMPanel", L"", WS_CHILD,
        30, 115, 444, 450, hMainWnd, NULL, hInstance, NULL);
    HWND p = hPanels[PANEL_PIN];

    MakeLabel(p, L"Enter Your PIN", 0, 30, 444, 35, hBoldFont);
    MakeLabel(p, L"Please enter your 4-digit PIN:", 40, 100, 364, 22, hNormalFont);
    hPinInput = MakeEdit(p, IDC_PIN_INPUT, 132, 130, 180, 36, hNormalFont, true);
    SendMessage(hPinInput, EM_SETLIMITTEXT, 4, 0);
    hBtnValidate = MakeButton(p, L"  VALIDATE PIN  ", IDC_BTN_VALIDATE, 132, 195, 180, 44, hBoldFont);
    MakeButton(p, L"Cancel / Eject Card", IDC_BTN_EJECT, 132, 260, 180, 38, hNormalFont);
}

void ATMGui::createMenuPanel() {
    hPanels[PANEL_MENU] = CreateWindowW(L"ATMPanel", L"", WS_CHILD,
        30, 115, 444, 450, hMainWnd, NULL, hInstance, NULL);
    HWND p = hPanels[PANEL_MENU];

    hWelcomeLabel = MakeLabel(p, L"Welcome!", 0, 20, 444, 30, hBoldFont);
    hMenuBalance = MakeLabel(p, L"", 0, 55, 444, 26, hNormalFont);
    MakeLabel(p, L"Select an option:", 30, 100, 384, 22, hNormalFont);
    MakeButton(p, L"  Check Balance  ", IDC_BTN_BALANCE, 82, 140, 280, 48, hBoldFont);
    MakeButton(p, L"  Withdraw Money  ", IDC_BTN_WITHDRAW, 82, 205, 280, 48, hBoldFont);
    MakeButton(p, L"  Change PIN  ", IDC_BTN_CHANGE_PIN, 82, 270, 280, 48, hBoldFont);
    MakeButton(p, L"  Transaction History  ", IDC_BTN_HISTORY, 82, 335, 280, 48, hBoldFont);
    MakeButton(p, L"  Eject Card / Exit  ", IDC_BTN_EJECT, 82, 400, 280, 40, hNormalFont);
}

void ATMGui::createWithdrawPanel() {
    hPanels[PANEL_WITHDRAW] = CreateWindowW(L"ATMPanel", L"", WS_CHILD,
        30, 115, 444, 450, hMainWnd, NULL, hInstance, NULL);
    HWND p = hPanels[PANEL_WITHDRAW];

    MakeLabel(p, L"Withdraw Money", 0, 30, 444, 35, hBoldFont);
    hWithdrawBalance = MakeLabel(p, L"", 0, 75, 444, 22, hNormalFont);
    MakeLabel(p, L"Quick Amounts:", 40, 120, 200, 22, hNormalFont);
    MakeButton(p, L"Rs. 500", 3001, 30, 150, 90, 38, hNormalFont);
    MakeButton(p, L"Rs. 1000", 3002, 135, 150, 90, 38, hNormalFont);
    MakeButton(p, L"Rs. 2000", 3003, 240, 150, 90, 38, hNormalFont);
    MakeButton(p, L"Rs. 5000", 3004, 345, 150, 90, 38, hNormalFont);
    MakeLabel(p, L"Or enter custom amount:", 40, 210, 364, 22, hNormalFont);
    hAmountInput = MakeEdit(p, IDC_AMOUNT_INPUT, 132, 240, 180, 36, hNormalFont);
    MakeButton(p, L"  WITHDRAW  ", IDC_BTN_WITHDRAW, 132, 300, 180, 44, hBoldFont);
    MakeButton(p, L"Back to Menu", IDC_BTN_BALANCE, 132, 365, 180, 36, hNormalFont);
}

void ATMGui::createChangePINPanel() {
    hPanels[PANEL_CHANGE_PIN] = CreateWindowW(L"ATMPanel", L"", WS_CHILD,
        30, 115, 444, 450, hMainWnd, NULL, hInstance, NULL);
    HWND p = hPanels[PANEL_CHANGE_PIN];

    MakeLabel(p, L"Change PIN", 0, 30, 444, 35, hBoldFont);
    MakeLabel(p, L"Current PIN:", 80, 100, 180, 22, hNormalFont);
    hOldPinInput = MakeEdit(p, IDC_PIN_INPUT, 132, 128, 180, 34, hNormalFont, true);
    SendMessage(hPinInput, EM_SETLIMITTEXT, 4, 0);
    MakeLabel(p, L"New PIN (4 digits):", 80, 185, 200, 22, hNormalFont);
    hNewPinInput = MakeEdit(p, IDC_NEW_PIN_INPUT, 132, 213, 180, 34, hNormalFont, true);
    SendMessage(hNewPinInput, EM_SETLIMITTEXT, 4, 0);
    MakeLabel(p, L"Confirm New PIN:", 80, 270, 200, 22, hNormalFont);
    hConfirmPinInput = MakeEdit(p, IDC_CONFIRM_PIN, 132, 298, 180, 34, hNormalFont, true);
    SendMessage(hConfirmPinInput, EM_SETLIMITTEXT, 4, 0);
    MakeButton(p, L"  CHANGE PIN  ", IDC_BTN_CONFIRM_PIN, 132, 360, 180, 44, hBoldFont);
    MakeButton(p, L"Back to Menu", IDC_BTN_BALANCE, 132, 415, 180, 36, hNormalFont);
}

void ATMGui::createHistoryPanel() {
    hPanels[PANEL_HISTORY] = CreateWindowW(L"ATMPanel", L"", WS_CHILD,
        30, 115, 444, 450, hMainWnd, NULL, hInstance, NULL);
    HWND p = hPanels[PANEL_HISTORY];

    MakeLabel(p, L"Transaction History", 0, 15, 444, 30, hBoldFont);
    hTransactionList = CreateWindowExW(0, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
        10, 55, 424, 340, p, (HMENU)IDC_TRANSACTION_LIST, hInstance, NULL);
    SendMessage(hTransactionList, WM_SETFONT, (WPARAM)hSmallFont, TRUE);
    ListView_SetExtendedListViewStyle(hTransactionList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LVCOLUMNW col = {};
    col.mask = LVCF_TEXT | LVCF_WIDTH;
    col.cx = 75; col.pszText = (LPWSTR)L"Type"; ListView_InsertColumn(hTransactionList, 0, &col);
    col.cx = 90; col.pszText = (LPWSTR)L"Amount"; ListView_InsertColumn(hTransactionList, 1, &col);
    col.cx = 145; col.pszText = (LPWSTR)L"Date"; ListView_InsertColumn(hTransactionList, 2, &col);
    col.cx = 100; col.pszText = (LPWSTR)L"Balance"; ListView_InsertColumn(hTransactionList, 3, &col);

    MakeButton(p, L"Back to Menu", IDC_BTN_BALANCE, 132, 410, 180, 36, hNormalFont);
}

void ATMGui::showPanel(int panelId) {
    for (int i = 0; i < 6; i++) {
        if (hPanels[i]) ShowWindow(hPanels[i], SW_HIDE);
    }
    if (hPanels[panelId]) ShowWindow(hPanels[panelId], SW_SHOW);
    currentPanel = panelId;
    InvalidateRect(hMainWnd, NULL, TRUE);
}

void ATMGui::setStatus(const std::string& msg, bool isError) {
    std::wstring wmsg(msg.begin(), msg.end());
    SetWindowTextW(hStatusLabel, wmsg.c_str());
}

void ATMGui::clearInputs() {
    if (hCardInput) SetWindowTextW(hCardInput, L"");
    if (hPinInput) SetWindowTextW(hPinInput, L"");
    if (hAmountInput) SetWindowTextW(hAmountInput, L"");
    if (hNewPinInput) SetWindowTextW(hNewPinInput, L"");
    if (hConfirmPinInput) SetWindowTextW(hConfirmPinInput, L"");
    if (hOldPinInput) SetWindowTextW(hOldPinInput, L"");
}

void ATMGui::updateBalance() {
    if (atm.isSessionActive()) {
        double bal = atm.checkBalance();
        std::ostringstream oss;
        oss << "Balance: Rs. " << std::fixed << std::setprecision(2) << bal;
        std::wstring wbal(oss.str().begin(), oss.str().end());
        if (hMenuBalance) SetWindowTextW(hMenuBalance, wbal.c_str());
        if (hWithdrawBalance) SetWindowTextW(hWithdrawBalance, wbal.c_str());
    }
}

void ATMGui::loadHistory() {
    if (!hTransactionList) return;
    ListView_DeleteAllItems(hTransactionList);
    auto records = atm.getHistory();
    int row = 0;
    for (auto& r : records) {
        std::wstring wType(r.transactionType.begin(), r.transactionType.end());
        std::wostringstream wAmt, wBal;
        wAmt << L"Rs. " << std::fixed << std::setprecision(2) << r.amount;
        wBal << L"Rs. " << std::fixed << std::setprecision(2) << r.balanceAfter;
        std::wstring wDate(r.transactionDate.begin(), r.transactionDate.end());

        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = row;
        item.pszText = (LPWSTR)wType.c_str();
        ListView_InsertItem(hTransactionList, &item);
        ListView_SetItemText(hTransactionList, row, 1, (LPWSTR)wAmt.str().c_str());
        ListView_SetItemText(hTransactionList, row, 2, (LPWSTR)wDate.c_str());
        ListView_SetItemText(hTransactionList, row, 3, (LPWSTR)wBal.str().c_str());
        row++;
    }
    if (records.empty()) {
        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = 0;
        item.pszText = (LPWSTR)L"No transactions found.";
        ListView_InsertItem(hTransactionList, &item);
    }
}

void ATMGui::onInsertCard() {
    wchar_t buf[32] = {};
    GetWindowTextW(hCardInput, buf, 32);
    std::wstring wcard(buf);
    std::string card(wcard.begin(), wcard.end());
    if (card.empty()) { setStatus("Please enter a card number.", true); return; }
    if (card.length() != 16) { setStatus("Card number must be 16 digits.", true); return; }
    if (atm.insertCard(card)) {
        setStatus("Card accepted. Please enter your PIN.");
        clearInputs();
        showPanel(PANEL_PIN);
    } else {
        setStatus("Card not found. Please try again.", true);
    }
}

void ATMGui::onValidatePIN() {
    wchar_t buf[8] = {};
    GetWindowTextW(hPinInput, buf, 8);
    std::wstring wpin(buf);
    std::string pin(wpin.begin(), wpin.end());
    if (pin.length() != 4) { setStatus("PIN must be 4 digits.", true); return; }
    if (atm.validatePIN(pin)) {
        std::string welcome = "Welcome, " + atm.getCardHolderName() + "!";
        setStatus(welcome);
        std::wstring ww(welcome.begin(), welcome.end());
        if (hWelcomeLabel) SetWindowTextW(hWelcomeLabel, ww.c_str());
        updateBalance();
        clearInputs();
        showPanel(PANEL_MENU);
    } else {
        if (atm.getState() == ATMState::IDLE) {
            setStatus("Too many wrong attempts. Card ejected.", true);
            clearInputs();
            showPanel(PANEL_CARD);
        } else {
            setStatus("Incorrect PIN. Please try again.", true);
            SetWindowTextW(hPinInput, L"");
        }
    }
}

void ATMGui::onCheckBalance() {
    if (currentPanel == PANEL_WITHDRAW || currentPanel == PANEL_CHANGE_PIN || currentPanel == PANEL_HISTORY) {
        updateBalance();
        clearInputs();
        showPanel(PANEL_MENU);
        return;
    }
    double bal = atm.checkBalance();
    std::ostringstream oss;
    oss << "Your balance is: Rs. " << std::fixed << std::setprecision(2) << bal;
    setStatus(oss.str());
    updateBalance();
}

void ATMGui::onWithdraw() {
    if (currentPanel == PANEL_MENU) {
        updateBalance();
        showPanel(PANEL_WITHDRAW);
        return;
    }
    wchar_t buf[32] = {};
    GetWindowTextW(hAmountInput, buf, 32);
    std::wstring wamt(buf);
    if (wamt.empty()) { setStatus("Please enter an amount.", true); return; }
    double amount = 0;
    try { amount = std::stod(std::string(wamt.begin(), wamt.end())); }
    catch (...) { setStatus("Invalid amount entered.", true); return; }
    std::string msg;
    if (atm.withdraw(amount, msg)) {
        setStatus(msg);
        updateBalance();
        SetWindowTextW(hAmountInput, L"");
        showPanel(PANEL_MENU);
    } else {
        setStatus(msg, true);
    }
}

void ATMGui::onShowChangePIN() {
    clearInputs();
    showPanel(PANEL_CHANGE_PIN);
    setStatus("Enter current PIN and choose a new PIN.");
}

void ATMGui::onConfirmChangePIN() {
    wchar_t p1[8] = {}, p2[8] = {}, p3[8] = {};
    GetWindowTextW(hOldPinInput, p1, 8);
    GetWindowTextW(hNewPinInput, p2, 8);
    GetWindowTextW(hConfirmPinInput, p3, 8);
    std::wstring wp1(p1), wp2(p2), wp3(p3);
    if (wp2 != wp3) { setStatus("New PINs do not match.", true); return; }
    std::string oldPin(wp1.begin(), wp1.end());
    std::string newPin(wp2.begin(), wp2.end());
    std::string msg;
    if (atm.changePIN(oldPin, newPin, msg)) {
        setStatus(msg);
        clearInputs();
        showPanel(PANEL_MENU);
    } else {
        setStatus(msg, true);
    }
}

void ATMGui::onShowHistory() {
    loadHistory();
    showPanel(PANEL_HISTORY);
    setStatus("Showing last 10 transactions.");
}

void ATMGui::onEjectCard() {
    atm.ejectCard();
    clearInputs();
    setStatus("Card ejected. Thank you for using our ATM.");
    showPanel(PANEL_CARD);
}

LRESULT CALLBACK ATMGui::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    ATMGui* self = instance;
    switch (msg) {
        case WM_TIMER: {
       
        if (wParam == 1 && self && self->hTimeLabel) {
            SYSTEMTIME st;
            GetLocalTime(&st); // Grab the computer's live time
            
            wchar_t timeStr[100];
        
            wsprintfW(timeStr, L"Date: %02d/%02d/%04d   Time: %02d:%02d:%02d", 
                st.wDay, st.wMonth, st.wYear, 
                st.wHour, st.wMinute, st.wSecond);
                
            SetWindowTextW(self->hTimeLabel, timeStr);
        }
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (self) {
            if (id == IDC_BTN_INSERT) self->onInsertCard();
            else if (id == IDC_BTN_VALIDATE) self->onValidatePIN();
            else if (id == IDC_BTN_BALANCE) self->onCheckBalance();
            else if (id == IDC_BTN_WITHDRAW) self->onWithdraw();
            else if (id == IDC_BTN_CHANGE_PIN) self->onShowChangePIN();
            else if (id == IDC_BTN_CONFIRM_PIN) self->onConfirmChangePIN();
            else if (id == IDC_BTN_HISTORY) self->onShowHistory();
            else if (id == IDC_BTN_EJECT) self->onEjectCard();
            else if (id == 3001) { SetWindowTextW(self->hAmountInput, L"500"); }
            else if (id == 3002) { SetWindowTextW(self->hAmountInput, L"1000"); }
            else if (id == 3003) { SetWindowTextW(self->hAmountInput, L"2000"); }
            else if (id == 3004) { SetWindowTextW(self->hAmountInput, L"5000"); }
        }
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(220, 235, 255));
        return (LRESULT)self->hBgBrush;
    }
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(200, 220, 255));
        SetTextColor(hdc, RGB(10, 30, 70));
        return (LRESULT)self->hEditBrush;
    }
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(0, 100, 200));
        SetTextColor(hdc, RGB(255, 255, 255));
        return (LRESULT)self->hBtnBrush;
    }
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, self->hBgBrush);
        // Draw header bar
        RECT header = { 0, 0, rc.right, 70 };
        HBRUSH hHeaderBrush = CreateSolidBrush(RGB(5, 25, 60));
        FillRect(hdc, &header, hHeaderBrush);
        DeleteObject(hHeaderBrush);
        // Draw status bar
        RECT statusBar = { 0, 70, rc.right, 108 };
        HBRUSH hStatusBrush = CreateSolidBrush(RGB(0, 80, 160));
        FillRect(hdc, &statusBar, hStatusBrush);
        DeleteObject(hStatusBrush);
        return 1;
    }
    case WM_DESTROY:
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int ATMGui::run() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
