#include <windows.h>
#include <string>
#include "Database.h"
#include "ATM.h"
#include "ATMGui.h"

// Connection string - update Server name as needed
const std::string CONNECTION_STRING =
    "DRIVER={ODBC Driver 17 for SQL Server};"
    "SERVER=localhost;"
    "DATABASE=ATM_DB;"
    "Trusted_Connection=yes;";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    Database db;

    if (!db.connect(CONNECTION_STRING)) {
        MessageBoxW(NULL,
            L"Could not connect to the database.\n\n"
            L"Please ensure:\n"
            L"1. SQL Server is running\n"
            L"2. The ATM_DB database exists\n"
            L"3. ODBC Driver 17 for SQL Server is installed\n"
            L"4. Connection string in main.cpp is correct",
            L"Database Connection Error",
            MB_OK | MB_ICONERROR);
        return 1;
    }

    ATM atm(db);
    ATMGui gui(hInstance, atm);

    if (!gui.create()) {
        MessageBoxW(NULL, L"Failed to create application window.", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    return gui.run();
}
