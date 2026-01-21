#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ===============================================
//                   CONFIGURATION & CONSTANTS
// ===============================================
#define USER_FILE "users.dat"
#define LEDGER_FILE "ledger.csv"
#define ADMIN_ID 0
#define TRANSACTION_FEE_RATE 0.01 // 1% Fee
#define ADMIN_PASSWORD "admin123" // Default admin password

// ANSI Colors for Console UI
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define CYAN "\033[0;36m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"

// =======================================
//                   DATA STRUCTURES
// =======================================

typedef struct {
    int id;
    char name[50];
    char password[20];
    double balance;
} User;

typedef struct {
    char txId[20];
    int senderId;
    int receiverId;
    double amount;
    char type[10]; // "DEPOSIT", "TRANSFER", "FEE"
    char timestamp[30];
} Transaction;

// ===========================================
//                   HELPER FUNCTIONS
// ===========================================

void getCurrentTime(char *buffer) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", tm_info);
}

void generateTxID(char *buffer) {
    // Generates a pseudo-random TXID like "TX-8921"
    srand(time(NULL) + rand()); // Seed with time + random
    sprintf(buffer, "TX-%d", rand() % 9000 + 1000);
}

// Check if a User ID exists in the binary file
int userExists(int id) {
    FILE *file = fopen(USER_FILE, "rb");
    if (!file) return 0;

    User u;
    while (fread(&u, sizeof(User), 1, file)) {
        if (u.id == id) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

// Log to CSV Ledger (Append Only)
void logTransactionToCSV(int senderId, int receiverId, double amount, const char *type) {
    FILE *file = fopen(LEDGER_FILE, "a");
    if (!file) return;

    char txId[20], timeStr[30];
    generateTxID(txId);
    getCurrentTime(timeStr);

    // CSV Format: TXID, Sender, Receiver, Amount, Type, Date
    fprintf(file, "%s,%d,%d,%.2f,%s,%s\n", txId, senderId, receiverId, amount, type, timeStr);
    fclose(file);
}

// Initialize System (Create Admin if not exists)
void initializeSystem() {
    FILE *file = fopen(USER_FILE, "rb");
    if (file) {
        // Check if file is empty
        fseek(file, 0, SEEK_END);
        if (ftell(file) > 0) {
            fclose(file);
            return; // Admin likely exists
        }
        fclose(file);
    }

    // Create Admin User
    file = fopen(USER_FILE, "wb");
    User admin;
    admin.id = ADMIN_ID;
    strcpy(admin.name, "System Admin");
    strcpy(admin.password, ADMIN_PASSWORD);
    admin.balance = 0.0;
    fwrite(&admin, sizeof(User), 1, file);
    fclose(file);
    
    // Create Header for Ledger
    FILE *csv = fopen(LEDGER_FILE, "w");
    fprintf(csv, "TXID,SenderID,ReceiverID,Amount,Type,Timestamp\n");
    fclose(csv);
    
    printf(GREEN "[SYSTEM] System Initialized. Admin ID: 0 Created.\n" RESET);
}

// ============================================
//                   CORE FOUNDER FEATURES
// ============================================

void generateStatement(int currentUserID) {
    FILE *file = fopen(LEDGER_FILE, "r");
    if (file == NULL) {
        printf(RED "Error: Transaction history not found.\n" RESET);
        return;
    }

    char line[256];
    char txId[20], type[10], date[30];
    int sender, receiver;
    double amount;

    printf("\n=================================================================\n");
    printf(CYAN "                  ACCOUNT STATEMENT: USER ID %d\n" RESET, currentUserID);
    printf("=================================================================\n");
    printf("%-10s | %-10s | %-12s | %s\n", "TX ID", "TYPE", "AMOUNT", "DATE");
    printf("-----------------------------------------------------------------\n");

    // Skip Header
    fgets(line, sizeof(line), file); 

    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        // Clean trailing newline if present
        line[strcspn(line, "\n")] = 0;
        
        sscanf(line, "%[^,],%d,%d,%lf,%[^,],%[^\n]", txId, &sender, &receiver, &amount, type, date);

        if (sender == currentUserID || receiver == currentUserID) {
            found = 1;
            if (sender == currentUserID) {
                // Sent Money (Red)
                printf("%-10s | %-10s | " RED "-$%-10.2f" RESET " | %s\n", txId, "SENT", amount, date);
            } else {
                // Received Money (Green)
                printf("%-10s | %-10s | " GREEN "+$%-10.2f" RESET " | %s\n", txId, "RECEIVED", amount, date);
            }
        }
    }

    if (!found) printf("\t\tNo transactions found.\n");
    printf("=================================================================\n");
    fclose(file);
}

void adminDashboard(int currentUserID) {
    if (currentUserID != ADMIN_ID) {
        printf(RED "\n[ACCESS DENIED] Admin privileges required.\n" RESET);
        return;
    }

    FILE *file = fopen(USER_FILE, "rb");
    if (!file) return;

    User u;
    double totalLiquidity = 0.0;
    double totalRevenue = 0.0;
    int totalUsers = 0;

    printf("\n" YELLOW "#####################################################\n");
    printf("             QUANTUM PARK - SYSTEM ADMIN             \n");
    printf("#####################################################" RESET "\n");

    while (fread(&u, sizeof(User), 1, file)) {
        if (u.id == ADMIN_ID) {
            totalRevenue = u.balance;
        } else {
            totalLiquidity += u.balance;
            totalUsers++;
        }
    }

    printf(" SYSTEM HEALTH METRICS:\n");
    printf(" ----------------------\n");
    printf(" [👥] Active Users:      %d\n", totalUsers);
    printf(" [💰] Total Liquidity:   $%.2f (User Assets)\n", totalLiquidity);
    printf(" [📈] Total Revenue:     " GREEN "$%.2f" RESET " (Collected Fees)\n", totalRevenue);
    
    if (totalLiquidity > 0)
        printf(" [📊] Capital Ratio:     %.2f%%\n", (totalRevenue / totalLiquidity) * 100);
    else 
        printf(" [📊] Capital Ratio:     N/A (No Liquidity)\n");

    printf("#####################################################\n");
    fclose(file);
}

// ======================================
//                   TRANSACTION LOGIC
// ======================================

void processTransfer(int senderId, int receiverId, double amount) {
    FILE *file = fopen(USER_FILE, "rb+");
    if (!file) {
        printf(RED "Database Error.\n" RESET);
        return;
    }

    User u;
    long senderPos = -1, receiverPos = -1, adminPos = -1;
    User sender, receiver, admin;
    int foundCount = 0;

    // 1. Locate all involved users in the file
    while (fread(&u, sizeof(User), 1, file)) {
        if (u.id == senderId) { sender = u; senderPos = ftell(file) - sizeof(User); foundCount++; }
        else if (u.id == receiverId) { receiver = u; receiverPos = ftell(file) - sizeof(User); foundCount++; }
        else if (u.id == ADMIN_ID) { admin = u; adminPos = ftell(file) - sizeof(User); foundCount++; }
    }

    if (senderPos == -1 || receiverPos == -1 || adminPos == -1) {
        printf(RED "Error: User(s) not found.\n" RESET);
        fclose(file);
        return;
    }

    // 2. Calculate Fees
    double fee = amount * TRANSACTION_FEE_RATE;
    double totalDeduction = amount + fee;

    // 3. Validation
    if (sender.balance < totalDeduction) {
        printf(RED "Insufficient Funds! You need $%.2f (incl. fee).\n" RESET, totalDeduction);
        fclose(file);
        return;
    }

    // 4. Atomic Updates (In Memory)
    sender.balance -= totalDeduction;
    receiver.balance += amount;
    admin.balance += fee;

    // 5. Atomic Writes (To File)
    fseek(file, senderPos, SEEK_SET);
    fwrite(&sender, sizeof(User), 1, file);

    fseek(file, receiverPos, SEEK_SET);
    fwrite(&receiver, sizeof(User), 1, file);

    fseek(file, adminPos, SEEK_SET);
    fwrite(&admin, sizeof(User), 1, file);

    fclose(file);

    // 6. Log to CSV
    logTransactionToCSV(senderId, receiverId, amount, "TRANSFER");
    logTransactionToCSV(senderId, ADMIN_ID, fee, "FEE");

    printf(GREEN "\n[SUCCESS] Sent $%.2f to User %d.\n" RESET, amount, receiverId);
    printf(YELLOW "[INFO] Fee Deducted: $%.2f\n" RESET, fee);
}

void depositMoney(int userId, double amount) {
    FILE *file = fopen(USER_FILE, "rb+");
    if (!file) return;

    User u;
    while (fread(&u, sizeof(User), 1, file)) {
        if (u.id == userId) {
            u.balance += amount;
            fseek(file, -sizeof(User), SEEK_CUR);
            fwrite(&u, sizeof(User), 1, file);
            fclose(file);
            logTransactionToCSV(userId, userId, amount, "DEPOSIT");
            printf(GREEN "\n[SUCCESS] Deposited $%.2f. New Balance: $%.2f\n" RESET, amount, u.balance);
            return;
        }
    }
    fclose(file);
}

// ===================================
//                   AUTH LOGIC
// ===================================

void registerUser() {
    FILE *file = fopen(USER_FILE, "ab");
    User u;
    
    printf("\n--- REGISTRATION ---\n");
    printf("Enter new User ID: ");
    scanf("%d", &u.id);

    if (userExists(u.id)) {
        printf(RED "Error: User ID already exists.\n" RESET);
        fclose(file);
        return;
    }

    printf("Enter Name: ");
    scanf(" %[^\n]", u.name);
    printf("Enter Password: ");
    scanf("%s", u.password);
    u.balance = 0.0; // Start with 0 balance

    fwrite(&u, sizeof(User), 1, file);
    fclose(file);
    printf(GREEN "User Registered Successfully!\n" RESET);
}

int loginUser() {
    int id;
    char password[20];
    FILE *file = fopen(USER_FILE, "rb");
    if (!file) return -1;

    printf("\n--- LOGIN ---\n");
    printf("User ID: ");
    scanf("%d", &id);
    printf("Password: ");
    scanf("%s", password);

    User u;
    while (fread(&u, sizeof(User), 1, file)) {
        if (u.id == id && strcmp(u.password, password) == 0) {
            printf(GREEN "\nWelcome back, %s!\n" RESET, u.name);
            printf("Current Balance: " CYAN "$%.2f\n" RESET, u.balance);
            fclose(file);
            return id;
        }
    }

    printf(RED "Invalid Credentials.\n" RESET);
    fclose(file);
    return -1;
}

// =======================================
//                   MAIN MENU
// =======================================

int main() {
    initializeSystem();
    int choice, loggedInId = -1;

    while (1) {
        if (loggedInId == -1) {
            printf("\n--- LEDGER CORE SYSTEM ---\n");
            printf("1. Login\n");
            printf("2. Register\n");
            printf("3. Exit\n");
            printf("Select: ");
            scanf("%d", &choice);

            if (choice == 1) loggedInId = loginUser();
            else if (choice == 2) registerUser();
            else if (choice == 3) exit(0);
            else printf(RED "Invalid choice.\n" RESET);

        } else {
            // User Dashboard
            printf("\n--- DASHBOARD (ID: %d) ---\n", loggedInId);
            printf("1. Deposit Money\n");
            printf("2. Transfer Money\n");
            printf("3. Account Statement\n");
            if (loggedInId == ADMIN_ID) printf(YELLOW "4. Admin Dashboard\n" RESET);
            printf("5. Logout\n");
            printf("Select: ");
            scanf("%d", &choice);

            if (choice == 1) {
                double amt;
                printf("Enter Amount: ");
                scanf("%lf", &amt);
                depositMoney(loggedInId, amt);
            } else if (choice == 2) {
                int receiver;
                double amt;
                printf("Enter Receiver ID: ");
                scanf("%d", &receiver);
                printf("Enter Amount: ");
                scanf("%lf", &amt);
                processTransfer(loggedInId, receiver, amt);
            } else if (choice == 3) {
                generateStatement(loggedInId);
            } else if (choice == 4 && loggedInId == ADMIN_ID) {
                adminDashboard(loggedInId);
            } else if (choice == 5) {
                loggedInId = -1;
                printf("Logged out.\n");
            }
        }
    }
    return 0;
}

