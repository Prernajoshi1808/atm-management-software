#pragma once

#define _WIN32_WINNT 0x0A00
#define _WIN32_IE 0x0A00

#include <windows.h>
#include <commctrl.h>
#include "ATMGui.h"
#include <string>
#include "ATM.h"
#include "Database.h"

// Window/Control IDs
#define IDC_CARD_INPUT      1001
#define IDC_PIN_INPUT       1002
#define IDC_AMOUNT_INPUT    1003
#define IDC_NEW_PIN_INPUT   1004
#define IDC_CONFIRM_PIN     1005
#define IDC_STATUS_LABEL    1006
#define IDC_BALANCE_LABEL   1007
#define IDC_TRANSACTION_LIST 1008

#define IDC_BTN_INSERT      2001
#define IDC_BTN_VALIDATE    2002
#define IDC_BTN_BALANCE     2003
#define IDC_BTN_WITHDRAW    2004
#define IDC_BTN_CHANGE_PIN  2005
#define IDC_BTN_HISTORY     2006
#define IDC_BTN_EJECT       2007
#define IDC_BTN_CONFIRM_PIN 2008

// Panel IDs
#define PANEL_CARD          0
#define PANEL_PIN           1
#define PANEL_MENU          2
#define PANEL_WITHDRAW      3
#define PANEL_CHANGE_PIN    4
#define PANEL_HISTORY       5

class ATMGui {
public:
    ATMGui(HINSTANCE hInstance, ATM& atm);
    bool create();
    int run();

private:
    HINSTANCE hInstance;
    HWND hMainWnd;
    ATM& atm;

    // Panel windows
    HWND hPanels[6];
    int currentPanel;

    // Controls
    HWND hCardInput, hPinInput, hAmountInput;
    HWND hOldPinInput, hNewPinInput, hConfirmPinInput;
    HWND hStatusLabel, hMenuBalance, hWithdrawBalance;
    HWND hTransactionList;
    HWND hBtnInsert, hBtnValidate;
    HWND hBtnBalance, hBtnWithdraw;
    HWND hBtnChangePin, hBtnHistory, hBtnEject;
    HWND hBtnConfirmPin;
    HWND hHeaderLabel, hWelcomeLabel;
    HWND hTimeLabel;

    // Colors & fonts
    HFONT hTitleFont, hNormalFont, hSmallFont, hBoldFont;
    HBRUSH hBgBrush, hPanelBrush, hBtnBrush, hEditBrush;

    // Setup
    void createFonts();
    void createBrushes();
    void createHeader(HWND parent);
    void createCardPanel();
    void createPINPanel();
    void createMenuPanel();
    void createWithdrawPanel();
    void createChangePINPanel();
    void createHistoryPanel();

    void showPanel(int panelId);
    void setStatus(const std::string& msg, bool isError = false);
    void updateBalance();
    void loadHistory();
    void clearInputs();

    // Event handlers
    void onInsertCard();
    void onValidatePIN();
    void onCheckBalance();
    void onWithdraw();
    void onShowChangePIN();
    void onConfirmChangePIN();
    void onShowHistory();
    void onEjectCard();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static ATMGui* instance;
};
