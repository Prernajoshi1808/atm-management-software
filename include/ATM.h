#pragma once
#include "Database.h"
#include <string>

enum class ATMState {
    IDLE,
    CARD_INSERTED,
    PIN_VALIDATED,
    MENU,
    TRANSACTION_COMPLETE
};

class ATM {
public:
    ATM(Database& db);

    // Session management
    bool insertCard(const std::string& cardNumber);
    bool validatePIN(const std::string& pin);
    void ejectCard();
    bool isSessionActive() const;
    std::string getCurrentCardNumber() const;
    int getCurrentAccountId() const;
    std::string getCardHolderName() const;

    // Transactions
    double checkBalance();
    bool withdraw(double amount, std::string& message);
    bool changePIN(const std::string& oldPin, const std::string& newPin, std::string& message);
    std::vector<TransactionRecord> getHistory();

    ATMState getState() const;

private:
    Database& db;
    ATMState state;
    std::string currentCard;
    int currentAccountId;
    std::string cardHolderName;
    int pinAttempts;
    static const int MAX_PIN_ATTEMPTS = 3;
};
