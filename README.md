# ATM Management Application

## Overview

ATM Management is a Windows desktop application built in C++ using Win32 APIs and SQL Server ODBC connectivity. The project simulates a simple ATM system with card authentication, PIN validation, balance inquiry, cash withdrawal, PIN change, and transaction history display.

This application is designed for demonstration and educational purposes, showing how a native Win32 GUI application can integrate with a relational database backend through ODBC.

## Key Features

- Card-based authentication with PIN validation
- Secure session flow with card insertion, PIN entry, and eject
- Balance inquiry for the authenticated account
- Cash withdrawal with:
  - transfer-safe amount validation
  - multiple-of-100 restriction
  - sufficient balance checks
- PIN change process with current PIN verification and 4-digit validation
- Transaction history display for the current account
- SQL Server database integration using ODBC Driver 17

## Architecture

- `src/main.cpp` — Application entry point and database connection initialization
- `src/Database.cpp` — SQL Server data access layer using ODBC
- `src/ATM.cpp` — Core ATM business logic and session state management
- `src/ATMGui.cpp` — Win32 user interface and event handling
- `include/` — Public header files for each major module
- `sql/setup.sql` — SQL Server database schema creation and sample seed data

## Prerequisites

- Windows 10 or later
- CMake 3.15 or newer
- Visual Studio with C++ workload, or another compatible C++ build toolchain
- ODBC Driver 17 for SQL Server
- SQL Server instance accessible from the development machine

## Database Setup

1. Open SQL Server Management Studio (SSMS) or another SQL client.
2. Execute `sql/setup.sql` to create the database schema and sample data.
3. Confirm the database was created as `ATM_DB` and contains the following tables:
   - `AccountDetails`
   - `CardDetails`
   - `TransactionHistory`

### Sample Accounts

The provided script seeds sample accounts and cards, including:

- Card `1234567890123456` / PIN `1234`
- Card `9876543210987654` / PIN `5678`
- Card `1111222233334444` / PIN `9999`

## Build Instructions

Open a command prompt in the repository root and run:

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

If you are using Visual Studio, you can also open the generated solution from the `build` folder and build the `ATM_Management` target.

## Run Instructions

1. Ensure SQL Server is running and the `ATM_DB` database is available.
2. Verify the ODBC connection string in `src/main.cpp` matches your SQL Server instance:

```cpp
const std::string CONNECTION_STRING =
    "DRIVER={ODBC Driver 17 for SQL Server};"
    "SERVER=localhost;"
    "DATABASE=ATM_DB;"
    "Trusted_Connection=yes;";
```

3. Launch the compiled executable from the `build` directory:

```powershell
.\Release\ATM_Management.exe
```

## Usage Guide

1. Enter a valid card number in the card input field.
2. Click `Insert Card`.
3. Enter the 4-digit PIN and click `Validate`.
4. Use the menu options to:
   - Check current balance
   - Withdraw cash
   - Change PIN
   - View recent transactions
   - Eject the card

### Withdrawal Rules

- Withdrawal amounts must be in multiples of `100`
- The requested amount must not exceed the available balance

### PIN Change Rules

- The new PIN must be exactly 4 digits
- The new PIN must differ from the old PIN
- The current PIN must be validated before the change is applied

## Project Structure

- `CMakeLists.txt` — Build configuration for the application
- `include/ATM.h` — ATM session and transaction API
- `include/ATMGui.h` — GUI definitions and control IDs
- `include/Database.h` — Database interface and ODBC helper definitions
- `src/main.cpp` — Program entry and application startup logic
- `src/Database.cpp` — ODBC-based SQL Server access implementation
- `src/ATM.cpp` — ATM use-case implementation and validation logic
- `src/ATMGui.cpp` — Win32 UI layout, event handling, and screen transitions
- `sql/setup.sql` — Database creation script and sample data

## Troubleshooting

- If the application fails to connect, verify SQL Server is running and the connection string is correct.
- Ensure `ODBC Driver 17 for SQL Server` is installed on the machine.
- If the UI fails to open, confirm the build target is `WIN32` and the executable is launched from a supported Windows environment.

## Notes

- This project is intended for demonstration and learning; it is not production-ready.
- Database credentials are handled through trusted connection configuration in `main.cpp`.
- You can extend the application by adding deposit support, full transaction filtering, account creation, and card deactivation logic.

## Advantages

- Native Windows application with a lightweight Win32 GUI.
- Clear separation between UI, business logic, and database access.
- Uses ODBC for database portability across SQL Server environments.
- Simple, easy-to-follow flow for ATM operations.
- Includes sample SQL schema and seeded data for fast setup.

## Disadvantages

- No encryption or secure PIN storage; PINs are stored as plain text for demo purposes.
- Limited error handling and no advanced logging.
- No support for deposits, transfers, or multi-currency accounts.
- Not audited for production-grade security or transaction safety.
- User interface is basic and not responsive to modern UI standards.

## Future Upgrades

- Add deposit and fund transfer functionality.
- Introduce secure PIN hashing and token-based authentication.
- Support account creation, card issuance, and admin management.
- Add full transaction filtering, pagination, and export options.
- Improve the UI with modern frameworks or a web-based frontend.
- Add unit tests, integration tests, and database migration scripts.

## License

This repository does not include a license file. We can add a license if we plan to distribute or share the code.