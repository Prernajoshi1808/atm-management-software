#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <stdexcept>

struct CardInfo {
    std::string cardNumber;
    std::string cardHolder;
    int accountId;
}; 

struct AccountInfo {
    int accountId;
    std::string holderName;
    double balance;
};

struct TransactionRecord {
    int transactionId;
    std::string transactionType;
    double amount;
    std::string transactionDate;
    double balanceAfter;
};

class Database {
public:
    Database();
    ~Database();

    bool connect(const std::string& connectionString);
    void disconnect();
    bool isConnected() const;

    // Card operations
    bool cardExists(const std::string& cardNumber);
    bool validatePIN(const std::string& cardNumber, const std::string& pin);
    CardInfo getCardInfo(const std::string& cardNumber);

    // Account operations
    AccountInfo getAccountInfo(int accountId);
    double getBalance(int accountId);
    bool withdraw(int accountId, double amount);
    bool changePIN(const std::string& cardNumber, const std::string& newPin);

    // Transaction operations
    std::vector<TransactionRecord> getTransactionHistory(int accountId, int limit = 10);
    bool logTransaction(int accountId, const std::string& type, double amount, double balanceAfter);

private:
    SQLHENV hEnv;
    SQLHDBC hDbc;
    bool connected;

    SQLHSTMT createStatement();
    void freeStatement(SQLHSTMT hStmt);
    std::string getSQLError(SQLHSTMT hStmt);
};
