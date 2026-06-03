-- ============================================
-- ATM Management System - Database Setup Script
-- Run this in SQL Server Management Studio
-- ============================================

-- Create Database
IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = 'ATM_DB')
    CREATE DATABASE ATM_DB;
GO

USE ATM_DB;
GO

-- ============================================
-- DROP EXISTING TABLES (Reverse Dependency Order)
-- Drop children first, then parents
-- ============================================
IF OBJECT_ID('TransactionHistory', 'U') IS NOT NULL
    DROP TABLE TransactionHistory;
GO

IF OBJECT_ID('CardDetails', 'U') IS NOT NULL
    DROP TABLE CardDetails;
GO

IF OBJECT_ID('AccountDetails', 'U') IS NOT NULL
    DROP TABLE AccountDetails;
GO

-- ============================================
-- TABLE: AccountDetails (PARENT)
-- Stores account balance information
-- ============================================
CREATE TABLE AccountDetails (
    AccountID   INT IDENTITY(1,1) PRIMARY KEY,
    HolderName  NVARCHAR(100) NOT NULL,
    Balance     DECIMAL(18,2) NOT NULL DEFAULT 0.00,
    AccountType NVARCHAR(20) NOT NULL DEFAULT 'SAVINGS',
    CreatedAt   DATETIME NOT NULL DEFAULT GETDATE()
);
GO

-- ============================================
-- TABLE: CardDetails (CHILD of Account)
-- Stores card and PIN information
-- ============================================
CREATE TABLE CardDetails (
    CardID      INT IDENTITY(1,1) PRIMARY KEY,
    CardNumber  NVARCHAR(16) NOT NULL UNIQUE,
    CardHolder  NVARCHAR(100) NOT NULL,
    PIN         NVARCHAR(4)  NOT NULL,
    AccountID   INT NOT NULL,
    IsActive    BIT NOT NULL DEFAULT 1,
    CreatedAt   DATETIME NOT NULL DEFAULT GETDATE()
);
GO

-- ============================================
-- TABLE: TransactionHistory (CHILD of Account)
-- Stores all transactions
-- ============================================
CREATE TABLE TransactionHistory (
    TransactionID   INT IDENTITY(1,1) PRIMARY KEY,
    AccountID       INT NOT NULL,
    TransactionType NVARCHAR(20) NOT NULL, -- 'WITHDRAWAL', 'PIN_CHANGE'
    Amount          DECIMAL(18,2) NOT NULL DEFAULT 0.00,
    TransactionDate DATETIME NOT NULL DEFAULT GETDATE(),
    BalanceAfter    DECIMAL(18,2) NOT NULL,
    FOREIGN KEY (AccountID) REFERENCES AccountDetails(AccountID)
);
GO

-- ============================================
-- INSERT SAMPLE DATA
-- ============================================

-- Insert Accounts first (Parent data must exist first)
INSERT INTO AccountDetails (HolderName, Balance, AccountType) VALUES
    ('Prerna Sharma',  50000.00, 'SAVINGS'),
    ('Rahul Verma',    25000.00, 'SAVINGS'),
    ('Anjali Singh',   75000.00, 'CURRENT');
GO

-- Insert Cards 
INSERT INTO CardDetails (CardNumber, CardHolder, PIN, AccountID) VALUES
    ('1234567890123456', 'Prerna Sharma', '1234', 1),
    ('9876543210987654', 'Rahul Verma',   '5678', 2),
    ('1111222233334444', 'Anjali Singh',  '9999', 3);
GO

-- Insert some sample transactions
INSERT INTO TransactionHistory (AccountID, TransactionType, Amount, BalanceAfter) VALUES
    (1, 'WITHDRAWAL', 2000.00, 48000.00),
    (1, 'WITHDRAWAL', 1000.00, 47000.00),
    (2, 'WITHDRAWAL', 500.00,  24500.00),
    (1, 'PIN_CHANGE',    0.00, 47000.00);
GO

-- ============================================
-- VERIFY DATA
-- ============================================
SELECT 'CardDetails' AS TableName, COUNT(*) AS [RowCount] FROM CardDetails
UNION ALL
SELECT 'AccountDetails', COUNT(*) FROM AccountDetails
UNION ALL
SELECT 'TransactionHistory', COUNT(*) FROM TransactionHistory;
GO

PRINT 'Database setup complete! ATM_DB is ready.';
GO