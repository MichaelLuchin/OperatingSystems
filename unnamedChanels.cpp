#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
using namespace std;

struct ConversionData {
    int base;
    char number[256];
    bool to_decimal;
};

int pipe_in[2];
int pipe_out[2];
pid_t pid;

void help() {
    cout << "Справка:\n"
         << "Программа для перевода чисел:\n"
         << "1. Из произвольной системы счисления в десятичную\n"
         << "2. Из десятичной системы в произвольную\n"
         << "Системы счисления могут быть от 2 до 36\n";
}

int read_base(const string& msg) {
    int result;
    while (true) {
        cout << msg;
        cin >> result;
        if (cin.fail() || result < 2 || result > 36) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Ошибка: система счисления должна быть от 2 до 36\n";
        } else {
            cin.ignore(1000, '\n');
            return result;
        }
    }
}

void read_number(const string& msg, char* buffer, size_t buffer_size) {
    cout << msg;

    string temp;
    getline(cin, temp);

    if (temp.size() >= buffer_size) {
        cerr << "Ошибка: введённое число слишком длинное (максимум "
             << buffer_size - 1 << " символов)\n";
        exit(1);
    }

    // Копируем и преобразуем в верхний регистр
    for (size_t i = 0; i < temp.size(); ++i) {
        buffer[i] = toupper(temp[i]);
    }
    buffer[temp.size()] = '\0';
}

bool is_valid_number(const string& num, int base) {
    if (num.empty()) return false;

    for (char c : num) {
        int digit;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'A' && c <= 'Z') digit = 10 + (c - 'A');
        else return false;

        if (digit >= base) return false;
    }
    return true;
}

// Перевод из произвольной системы в десятичную
string arbitrary_to_decimal(const string& num, int base) {
    long long result = 0;
    for (char c : num) {
        int digit = (c >= 'A') ? (10 + (c - 'A')) : (c - '0');
        result = result * base + digit;
    }
    return to_string(result);
}

// Перевод из десятичной системы в произвольную
string decimal_to_arbitrary(string num, int base) {
    // Проверяем, что строка содержит только цифры
    for (char c : num) {
        if (!isdigit(c)) {
            return "Ошибка: входное число должно быть десятичным";
        }
    }

    long long decimal = stoll(num);
    if (decimal == 0) return "0";

    string result;
    while (decimal > 0) {
        int digit = decimal % base;
        char c = (digit < 10) ? ('0' + digit) : ('A' + (digit - 10));
        result = c + result;
        decimal /= base;
    }
    return result;
}

void frontend() {
    ConversionData data;
    cout << "Выберите направление перевода:\n"
         << "1. Из произвольной системы в десятичную\n"
         << "2. Из десятичной системы в произвольную\n"
         << "Ваш выбор: ";

    int choice;
    cin >> choice;
    cin.ignore(); // Очищаем буфер после ввода числа

    if (choice == 1) {
        data.to_decimal = true;
        data.base = read_base("Введите исходную систему счисления (2-36): ");
        cout << "Введите число в " << data.base << "-чной системе: ";
    } else if (choice == 2) {
        data.to_decimal = false;
        data.base = read_base("Введите целевую систему счисления (2-36): ");
        cout << "Введите десятичное число: ";
    } else {
        cerr << "Ошибка: неверный выбор\n";
        exit(1);
    }

    read_number("", data.number, sizeof(data.number));

    if (data.to_decimal && !is_valid_number(data.number, data.base)) {
        cerr << "Ошибка: число " << data.number << " недопустимо в системе с основанием " << data.base << endl;
        exit(1);
    }
    if (!data.to_decimal) {
        for (char c : data.number) {
            if (c == '\0') break;
            if (!isdigit(c)) {
                cerr << "Ошибка: введено не десятичное число\n";
                exit(1);
            }
        }
    }

    write(pipe_in[1], &data, sizeof(ConversionData));

    char result[256];
    read(pipe_out[0], result, sizeof(result));
    cout << "Результат: " << result << endl;
    exit(0);
}

void backend() {
    ConversionData data;
    read(pipe_in[0], &data, sizeof(ConversionData));

    string result;
    if (data.to_decimal) {
        result = arbitrary_to_decimal(data.number, data.base);
    } else {
        result = decimal_to_arbitrary(data.number, data.base);
    }

    write(pipe_out[1], result.c_str(), result.size() + 1);
}

int do_unnamed_pipes(int argc, char *argv[]) {
    setlocale(LC_ALL, "ru_RU.UTF-8");

    if (argc == 2 && !strcmp(argv[1], "--help")) {
        help();
        exit(0);
    } else if (argc != 1) {
        cout << "Запустите программу с ключом --help для получения справки" << endl;
        exit(1);
    }

    cout << "Программа для перевода чисел между системами счисления (2-36)\n";

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        perror("pipe");
        exit(1);
    }

    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid > 0) {
        frontend();
    } else {
        backend();
    }

    close(pipe_in[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
    close(pipe_out[1]);

    return 0;
}