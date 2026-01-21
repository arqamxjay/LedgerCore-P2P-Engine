<img width="846" height="373" alt="Screenshot 2026-01-21 at 7 55 12 PM" src="https://github.com/user-attachments/assets/4a466e11-99a5-4bbb-984a-74a3338bb3f6" />
# LedgerCore-P2P-Engine 💸

**A Secure C-Based P2P Payment Ledger Prototype.**

LedgerCore is a backend simulation of a digital wallet system, engineered to demonstrate **financial data integrity**, **atomic transaction logic**, and **automated audit trails**.

## 🏦 Core FinTech Features
* **Atomic P2P Transfers:** Implements logic to ensure money is only deducted if the receiver exists and funds are sufficient.
* **Immutable Ledger:** Every transaction is logged to `ledger.csv` in an append-only format to simulate a real-world audit trail.
* **Revenue Generation:** A built-in 1% platform fee system that funnels revenue to a secure Admin ID (0).
* **System Analytics:** A hidden Admin Dashboard that calculates **Total Liquidity** and **Capital Reserve Ratios**.
* **Account Statements:** Dynamic parsing of the CSV ledger to provide users with a formatted transaction history.

## 🛠️ Technical Implementation
* **Language:** C
* **Data Models:** Structured using complex `structs` for Users and Transactions.
* **Storage:** Binary file handling (`.dat`) for fast user lookups and CSV streams for logs.

## 🚀 How to Run
1. Clone the repo: `git clone https://github.com/arqamxjay/LedgerCore-P2P-Engine.git`
2. Compile: `gcc src/main.c -o ledgercore`
3. Execute: `./ledgercore`

---
*Developed as a Academic-level Portfolio Project by Mohammad Arqam Javed*
