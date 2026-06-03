#include "Database.h"
#include <sstream>
#include <iostream>

Database::Database() : hEnv(SQL_NULL_HENV), hDbc(SQL_NULL_HDBC), connected(false) {}

Database::~Database() {
    disconnect();
}

bool Database::connect(const std::string& connectionString) {
    // Allocate environment
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) return false;
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    // Allocate connection
    if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) return false;

    // Connect
    std::wstring wcs(connectionString.begin(), connectionString.end());
    SQLWCHAR outConnStr[1024];
    SQLSMALLINT outLen;
    SQLRETURN ret = SQLDriverConnectW(hDbc, NULL,
        (SQLWCHAR*)wcs.c_str(), SQL_NTS,
        outConnStr, sizeof(outConnStr)/sizeof(SQLWCHAR), &outLen,
        SQL_DRIVER_NOPROMPT);

    connected = (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
    return connected;
}

void Database::disconnect() {
    if (hDbc != SQL_NULL_HDBC) {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        hDbc = SQL_NULL_HDBC;
    }
    if (hEnv != SQL_NULL_HENV) {
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        hEnv = SQL_NULL_HENV;
    }
    connected = false;
}

bool Database::isConnected() const { return connected; }

SQLHSTMT Database::createStatement() {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    return hStmt;
}

void Database::freeStatement(SQLHSTMT hStmt) {
    if (hStmt != SQL_NULL_HSTMT)
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

std::string Database::getSQLError(SQLHSTMT hStmt) {
    SQLWCHAR sqlState[6], msg[512];
    SQLINTEGER nativeErr;
    SQLSMALLINT msgLen;
    SQLGetDiagRecW(SQL_HANDLE_STMT, hStmt, 1, sqlState, &nativeErr, msg, 512, &msgLen);
    std::wstring ws(msg);
    return std::string(ws.begin(), ws.end());
}

bool Database::cardExists(const std::string& cardNumber) {
    SQLHSTMT hStmt = createStatement();
    std::wstring query = L"SELECT COUNT(*) FROM CardDetails WHERE CardNumber = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    std::wstring wCard(cardNumber.begin(), cardNumber.end());
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR,
        wCard.size(), 0, (SQLWCHAR*)wCard.c_str(), 0, NULL);
    SQLExecute(hStmt);
    SQLINTEGER count = 0;
    SQLFetch(hStmt);
    SQLGetData(hStmt, 1, SQL_C_LONG, &count, sizeof(count), NULL);
    SQLCloseCursor(hStmt);
    freeStatement(hStmt);
    return count > 0;
}

bool Database::validatePIN(const std::string& cardNumber, const std::string& pin) {
    SQLHSTMT hStmt = createStatement();
    std::wstring query = L"SELECT COUNT(*) FROM CardDetails WHERE CardNumber = ? AND PIN = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    std::wstring wCard(cardNumber.begin(), cardNumber.end());
    std::wstring wPin(pin.begin(), pin.end());
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wCard.size(), 0, (SQLWCHAR*)wCard.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wPin.size(), 0, (SQLWCHAR*)wPin.c_str(), 0, NULL);
    SQLExecute(hStmt);
    SQLINTEGER count = 0;
    SQLFetch(hStmt);
    SQLGetData(hStmt, 1, SQL_C_LONG, &count, sizeof(count), NULL);
    SQLCloseCursor(hStmt);
    freeStatement(hStmt);
    return count > 0;
}

CardInfo Database::getCardInfo(const std::string& cardNumber) {
    CardInfo info;
    info.cardNumber = cardNumber;
    SQLHSTMT hStmt = createStatement();
    std::wstring query = L"SELECT cd.CardHolder, cd.AccountID FROM CardDetails cd WHERE cd.CardNumber = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    std::wstring wCard(cardNumber.begin(), cardNumber.end());
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wCard.size(), 0, (SQLWCHAR*)wCard.c_str(), 0, NULL);
    SQLExecute(hStmt);
    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLWCHAR holder[256]={0};
        SQLLEN ind;
        SQLGetData(hStmt, 1, SQL_C_WCHAR, holder, sizeof(holder), &ind);
        std::wstring wHolder(holder);
        info.cardHolder = std::string(wHolder.begin(), wHolder.end());
        SQLGetData(hStmt, 2, SQL_C_LONG, &info.accountId, sizeof(info.accountId), &ind);
    }
    SQLCloseCursor(hStmt);
    freeStatement(hStmt);
    return info;
}

AccountInfo Database::getAccountInfo(int accountId) {
    AccountInfo info;
    info.accountId = accountId;
    SQLHSTMT hStmt = createStatement();
    std::wstring query = L"SELECT HolderName, Balance FROM AccountDetails WHERE AccountID = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &info.accountId, 0, NULL);
    SQLExecute(hStmt);
    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLWCHAR name[256]={0}; SQLLEN ind;
        SQLGetData(hStmt, 1, SQL_C_WCHAR, name, sizeof(name), &ind);
        std::wstring wn(name); info.holderName = std::string(wn.begin(), wn.end());
        SQLGetData(hStmt, 2, SQL_C_DOUBLE, &info.balance, sizeof(info.balance), &ind);
    }
    SQLCloseCursor(hStmt);
    freeStatement(hStmt);
    return info;
}

double Database::getBalance(int accountId) {
    return getAccountInfo(accountId).balance;
}

bool Database::withdraw(int accountId, double amount) {
    double balance = getBalance(accountId);
    if (balance < amount) return false;
    
    SQLHSTMT hStmt = createStatement();
    std::wstring query = L"UPDATE AccountDetails SET Balance = Balance - ? WHERE AccountID = ? AND Balance >= ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    
   
    double amountParam2 = amount;
    
    
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &amount, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &accountId, 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &amountParam2, 0, NULL);
    
    SQLRETURN ret = SQLExecute(hStmt);
    SQLLEN rows = 0;
    SQLRowCount(hStmt, &rows);
    freeStatement(hStmt);
    
    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) && rows > 0;
}

bool Database::changePIN(const std::string& cardNumber, const std::string& newPin) {
    SQLHSTMT hStmt = createStatement();
    std::wstring query = L"UPDATE CardDetails SET PIN = ? WHERE CardNumber = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    std::wstring wPin(newPin.begin(), newPin.end());
    std::wstring wCard(cardNumber.begin(), cardNumber.end());
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wPin.size(), 0, (SQLWCHAR*)wPin.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wCard.size(), 0, (SQLWCHAR*)wCard.c_str(), 0, NULL);
    SQLRETURN ret = SQLExecute(hStmt);
    freeStatement(hStmt);
    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

std::vector<TransactionRecord> Database::getTransactionHistory(int accountId, int limit) {
    std::vector<TransactionRecord> records;
    SQLHSTMT hStmt = createStatement();
    std::wstring query = L"SELECT TOP (?) TransactionID, TransactionType, Amount, TransactionDate, BalanceAfter FROM TransactionHistory WHERE AccountID = ? ORDER BY TransactionDate DESC";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &limit, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &accountId, 0, NULL);
    SQLExecute(hStmt);
    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        TransactionRecord rec;
        SQLLEN ind;
        SQLGetData(hStmt, 1, SQL_C_LONG, &rec.transactionId, sizeof(rec.transactionId), &ind);
        SQLWCHAR type[64]={0}, date[64]={0};
        SQLGetData(hStmt, 2, SQL_C_WCHAR, type, sizeof(type), &ind);
        std::wstring wt(type); rec.transactionType = std::string(wt.begin(), wt.end());
        SQLGetData(hStmt, 3, SQL_C_DOUBLE, &rec.amount, sizeof(rec.amount), &ind);
        SQLGetData(hStmt, 4, SQL_C_WCHAR, date, sizeof(date), &ind);
        std::wstring wd(date); rec.transactionDate = std::string(wd.begin(), wd.end());
        SQLGetData(hStmt, 5, SQL_C_DOUBLE, &rec.balanceAfter, sizeof(rec.balanceAfter), &ind);
        records.push_back(rec);
    }
    SQLCloseCursor(hStmt);
    freeStatement(hStmt);
    return records;
}

bool Database::logTransaction(int accountId, const std::string& type, double amount, double balanceAfter) {
    SQLHSTMT hStmt = createStatement();
    std::wstring query = L"INSERT INTO TransactionHistory (AccountID, TransactionType, Amount, TransactionDate, BalanceAfter) VALUES (?, ?, ?, GETDATE(), ?)";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    std::wstring wType(type.begin(), type.end());
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &accountId, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wType.size(), 0, (SQLWCHAR*)wType.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &amount, 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &balanceAfter, 0, NULL);
    SQLRETURN ret = SQLExecute(hStmt);
    freeStatement(hStmt);
    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}
