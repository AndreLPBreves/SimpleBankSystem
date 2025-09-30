#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "limits.h"
#include "stdlib.h"

typedef struct BankAccount
{
    char Username[255];
    char Password[255];

    double Balance;
} BankAccount;

typedef struct Transaction
{
    BankAccount* Source;
    BankAccount* Dest;
    double Value;
} Transaction;


bool custom_strcmp(const char* str1, const char* str2)
{
    for (;(*str1 == *str2 && *str1 != '\0'); str1++, str2++) {}
    return *str1 == *str2;
}

char* custom_strcpy(char* dest, int destMaxLen, const char* source, int sourceLen)
{
    char* ret = dest;
    if (sourceLen >= destMaxLen) return NULL;
    while ((*dest++ = *source++) != '\0') {}
    return ret;
}

int custom_atoi(const char* str)
{
    int result = 0;
    int sign = 1;
    
    for (;*str == ' '; str++) {}

    if (*str == '-')
    {
        sign = -1;
        str++;
    } else if (*str == '+') str++;

    for (;(*str >= '0' && *str <= '9'); str++)
    {
        int digit = *str - '0';
        if (result < INT_MIN/10) return (sign == 1) ? INT_MAX : INT_MIN;
        if (result == INT_MIN/10 && -digit < (INT_MIN % 10)) return (sign == 1) ? INT_MAX : INT_MIN;
        result = (result * 10) - digit;
    }
    
    if (sign > -1)
    {
        if (result == INT_MIN) return INT_MAX;
        return -result;
    }

    return result;
}

void getInput(const char* message, char* buffer, size_t bufferMaxLen)
{
    printf(message, "\n");
    fgets(buffer, bufferMaxLen, stdin);
    size_t len = custom_strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = '\0';
}

int custom_strlen(const char* str)
{
    int charCount = 0;
    for(; str[charCount] != '\0'; charCount++) {}
    return charCount;
}

BankAccount* findAccount(const char* accountName, BankAccount* bankAccounts, int numBankAccounts)
{
    BankAccount* bankAccount = NULL;

    for(int i = 0; i < numBankAccounts; i++)
    {
        if (custom_strcmp(accountName, bankAccounts[i].Username))
        {
            bankAccount = &bankAccounts[i];
            break;
        }
    }
    
    return bankAccount;
}

BankAccount* login(char* userInputBuffer, size_t inputMaxLen, BankAccount* bankAccounts, int numBankAccounts)
{
    getInput("Nome de usuario: ", userInputBuffer, inputMaxLen);
    if (custom_strlen(userInputBuffer) < 1) return NULL;

    BankAccount* bankAccount = findAccount(userInputBuffer, bankAccounts, numBankAccounts);

    if (bankAccount == NULL) return NULL;
    
    getInput("Senha: ", userInputBuffer, inputMaxLen);
    if (custom_strlen(userInputBuffer) < 1 || !custom_strcmp(bankAccount->Password, userInputBuffer)) return NULL;
    
    return bankAccount;
}

bool newAccount(char* userInputBuffer, size_t inputMaxLen, BankAccount* bankAccounts, int* numBankAccounts)
{
    if (*numBankAccounts >= 10)
    {
        printf("Numero maximo de contas atingido.\n");
        return false;
    }

    getInput("Nome de usuario: ", userInputBuffer, inputMaxLen);
    if (custom_strlen(userInputBuffer) < 1) return false;
    if (!custom_strcpy(bankAccounts[*numBankAccounts].Username, sizeof(bankAccounts[*numBankAccounts].Username), userInputBuffer, custom_strlen(userInputBuffer)))
    {
        return false;
    }

    getInput("Senha: ", userInputBuffer, inputMaxLen);
    if (custom_strlen(userInputBuffer) < 1) return false;
    if (!custom_strcpy(bankAccounts[*numBankAccounts].Password, sizeof(bankAccounts[*numBankAccounts].Password), userInputBuffer, custom_strlen(userInputBuffer)))
    {
        memset(&bankAccounts[*numBankAccounts], 0, sizeof(BankAccount));
        return false;
    }

    (*numBankAccounts)++;
    return true;
}

void deposit(BankAccount* bankAccount, char* userInputBuffer, const int inputMaxLen)
{
    getInput("Valor para depositar: ", userInputBuffer, inputMaxLen);
    double value = strtod(userInputBuffer, NULL);

    if (value <= 0.0) {
        printf("Valor de deposito invalido. Deve ser positivo.\n");
        return;
    }

    bankAccount->Balance += value;
    printf("Deposito de R$%.2f realizado. Saldo atual: R$%.2f\n", value, bankAccount->Balance);
}

void transfer(BankAccount* src,
              BankAccount* bankAccounts,
              int numBankAccounts,
              Transaction* transactions,
              int* numTransactions,
              char* userInputBuffer,
              const int inputMaxLen)
{
    getInput("Nome de usuario de destino: ", userInputBuffer, inputMaxLen);
    if (custom_strlen(userInputBuffer) < 1)
    {
        printf("Nome de usuario invalido.\n");
        return;
    }
    
    BankAccount* dest = findAccount(userInputBuffer, bankAccounts, numBankAccounts);
    
    if (dest == NULL)
    {
        printf("Erro: Conta de destino nao encontrada.\n");
        return;
    }

    getInput("Valor para transferir: ", userInputBuffer, inputMaxLen);
    double value = strtod(userInputBuffer, NULL);
    
    if (value <= 0.0)
    {
        printf("Valor de transferencia invalido. Deve ser positivo.\n");
        return;
    }

    if (src->Balance >= value)
    {
        if (*numTransactions >= 255)
        {
            printf("Erro: Limite maximo de transacoes atingido. Transferencia nao realizada.\n");
            return;
        }

        src->Balance -= value;
        dest->Balance += value;
        
        Transaction* currTransaction = &transactions[*numTransactions];
        currTransaction->Source = src;
        currTransaction->Dest = dest;
        currTransaction->Value = value;
        
        (*numTransactions)++;
        
        printf("Transferencia de R$%.2f para %s realizada com sucesso. Saldo atual: R$%.2f\n", value, dest->Username, src->Balance);
    }
    else
    {
        printf("Erro: Saldo insuficiente (R$%.2f) para transferir R$%.2f.\n", src->Balance, value);
    }
}

void withdraw(BankAccount* bankAccount, char* userInputBuffer, const int inputMaxLen)
{
    getInput("Valor para sacar: ", userInputBuffer, inputMaxLen);
    double value = strtod(userInputBuffer, NULL);

    if (value <= 0.0)
    {
        printf("Valor de saque invalido. Deve ser positivo.\n");
        return;
    }
    
    if (bankAccount->Balance < value)
    {
        printf("Erro: Saldo insuficiente (R$%.2f).\n", bankAccount->Balance);
        return;
    }
    
    bankAccount->Balance -= value;
    printf("Saque de R$%.2f realizado. Saldo atual: R$%.2f\n", value, bankAccount->Balance);
}

void statement(BankAccount* currentAccount, Transaction* transactions, int numTransactions)
{
    printf("\n--- EXTRATO DE TRANSACOES DE %s ---\n", currentAccount->Username);
    printf("Saldo Atual: R$%.2f\n", currentAccount->Balance);
    printf("----------------------------------\n");
    
    int found_transactions = 0;

    for (int i = 0; i < numTransactions; i++)
    {
        if (transactions[i].Source == currentAccount)
        {
            printf("[DEBITO] Transferencia enviada para %s: -R$%.2f\n", 
                   transactions[i].Dest->Username, 
                   transactions[i].Value);
            found_transactions = 1;
        }
        else if (transactions[i].Dest == currentAccount)
        {
            printf("[CREDITO] Transferencia recebida de %s: +R$%.2f\n", 
                   transactions[i].Source->Username, 
                   transactions[i].Value);
            found_transactions = 1;
        }
    }
    
    if (!found_transactions)
    {
         printf("Nenhuma transacao de transferencia registrada.\n");
         printf("(Depositos e Saques nao geram entradas no extrato.)\n");
    }
    
    printf("----------------------------------\n\n");
}

int main()
{
    char userInputBuffer[255] = {0};
    int option = 0;
    bool isRunning = true;
    BankAccount bankAccounts[10] = {0};
    int numBankAccounts = 0;
    BankAccount* currentBankAccount = NULL;
    Transaction transactions[255] = {0};
    int numTransactions = 0;
    
    printf("Bem-vindo ao sistema bancario!\n"
           "Para iniciar, escolha uma das opcoes abaixo:\n");

    while (isRunning)
    {
        getInput("(1) - Conectar\n"
                 "(2) - Registrar\n"
                 "(3) - Sair\n", userInputBuffer, sizeof(userInputBuffer));
        option = custom_atoi(userInputBuffer);
        switch (option)
        {
        case 1:
            if (!(currentBankAccount = login(userInputBuffer, sizeof(userInputBuffer), bankAccounts, numBankAccounts)))
            {
                printf("Credenciais invalidas\n");
                currentBankAccount = NULL;
                break;
            }
            printf("Bem-vindo %s!\n", currentBankAccount->Username);
            printf("Qual operacao vai realizar hoje?\n");
            while (currentBankAccount != NULL)
            {
                getInput("(1) - Depositar\n"
                         "(2) - Transferir\n"
                         "(3) - Sacar\n"
                         "(4) - Ver Extrato\n"
                         "(5) - Desconectar\n", userInputBuffer, sizeof(userInputBuffer));
                option = custom_atoi(userInputBuffer);
                switch (option)
                {
                case 1:
                    deposit(currentBankAccount, userInputBuffer, sizeof(userInputBuffer));
                    break;
                case 2:
                    transfer(currentBankAccount, bankAccounts, numBankAccounts, transactions, &numTransactions, userInputBuffer, sizeof(userInputBuffer));
                    break;
                case 3:
                    withdraw(currentBankAccount, userInputBuffer, sizeof(userInputBuffer));
                    break;
                case 4:
                    statement(currentBankAccount, transactions, numTransactions);
                    break;
                case 5:
                    printf("Desconectando...\n");
                    currentBankAccount = NULL;
                    break;
                default:
                    printf("\033[2J\033[H");
                    fflush(stdout);
                    printf("opcao %s invalida.\n"
                           "Escolha uma das opcoes abaixo:\n", userInputBuffer);
                }
            }
            break;
        case 2:
            newAccount(userInputBuffer, sizeof(userInputBuffer), bankAccounts, &numBankAccounts);
            break;
        case 3:
            printf("Saindo...\n");
            isRunning = false;
            break;
        default:
            printf("\033[2J\033[H");
            fflush(stdout);
            printf("opcao %s invalida.\n"
                   "Escolha uma das opcoes abaixo:\n", userInputBuffer);
        }
    }
           
    return 0;
}