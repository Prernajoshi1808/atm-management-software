#include "ATM.h"

ATM::ATM(Database& db)
    : db(db), state(ATMState::IDLE), currentAccountId(-1), pinAttempts(0) {}

bool ATM::insertCard(const std::string& cardNumber) {
    if (state != ATMState::IDLE) return false;
    if (!db.cardExists(cardNumber)) return false;
    currentCard = cardNumber;
    pinAttempts = 0;
    state = ATMState::CARD_INSERTED;
    return true;
}

bool ATM::validatePIN(const std::string& pin) {
    if (state != ATMState::CARD_INSERTED) return false;
    if (!db.validatePIN(currentCard, pin)) {
        pinAttempts++;
        if (pinAttempts >= MAX_PIN_ATTEMPTS) ejectCard();
        return false;
    }
    CardInfo info = db.getCardInfo(currentCard);
    currentAccountId = info.accountId;
    cardHolderName = info.cardHolder;
    state = ATMState::PIN_VALIDATED;
    return true;
}

void ATM::ejectCard() {
    currentCard.clear();
    currentAccountId = -1;
    cardHolderName.clear();
    pinAttempts = 0;
    state = ATMState::IDLE;
}

bool ATM::isSessionActive() const {
    return state == ATMState::PIN_VALIDATED;
}

std::string ATM::getCurrentCardNumber() const { return currentCard; }
int ATM::getCurrentAccountId() const { return currentAccountId; }
std::string ATM::getCardHolderName() const { return cardHolderName; }

double ATM::checkBalance() {
    if (!isSessionActive()) return -1;
    return db.getBalance(currentAccountId);
}

bool ATM::withdraw(double amount, std::string& message) {
    if (!isSessionActive()) { message = "No active session."; return false; }
    if (amount <= 0) { message = "Amount must be greater than zero."; return false; }
    if ((int)amount % 100 != 0) { message = "Amount must be in multiples of 100."; return false; }
    double balance = db.getBalance(currentAccountId);
    if (amount > balance) { message = "Insufficient balance."; return false; }
    if (!db.withdraw(currentAccountId, amount)) { message = "Transaction failed. Please try again."; return false; }
    double newBalance = db.getBalance(currentAccountId);
    db.logTransaction(currentAccountId, "WITHDRAWAL", amount, newBalance);
    message = "Successfully withdrew Rs. " + std::to_string((int)amount);
    return true;
}

bool ATM::changePIN(const std::string& oldPin, const std::string& newPin, std::string& message) {
    if (!isSessionActive()) { message = "No active session."; return false; }
    if (!db.validatePIN(currentCard, oldPin)) { message = "Current PIN is incorrect."; return false; }
    if (newPin.length() != 4) { message = "New PIN must be 4 digits."; return false; }
    for (char c : newPin) {
        if (!isdigit(c)) { message = "PIN must contain only digits."; return false; }
    }
    if (oldPin == newPin) { message = "New PIN must be different from current PIN."; return false; }
    if (!db.changePIN(currentCard, newPin)) { message = "Failed to change PIN."; return false; }
    db.logTransaction(currentAccountId, "PIN_CHANGE", 0, db.getBalance(currentAccountId));
    message = "PIN changed successfully!";
    return true;
}

std::vector<TransactionRecord> ATM::getHistory() {
    if (!isSessionActive()) return {};
    return db.getTransactionHistory(currentAccountId);
}

ATMState ATM::getState() const { return state; }
