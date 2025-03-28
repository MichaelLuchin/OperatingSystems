//
// Created by m_luc on 28.03.2025.
//
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
using namespace std;

struct ConversionData {
    int from_base;
    int to_base;
    string number;
};

int pipe_in[2];
int pipe_out[2];
pid_t pid;

void help() {
    cout << "Справка:\n"
         << "Программа для перевода чисел между системами счисления\n"
         << "Формат ввода: <исходная_система> <целевая_система> <число>\n"
         << "Пример: 2 10 1010 (переведёт 1010 из двоичной в десятичную)\n"
         << "Для десятичной в другую систему введите 10 <целевая_система> <число>\n"
         << "Из другой системы в десятичную введите <исходная_система> 10 <число>\n";
}

int read_base(const string& msg) {
    int result;
    bool flag = true;

    while (flag) {
        cout << msg;
        cin >> result;
        if(result < 2 || result > 36 || cin.fail() || (cin.peek() != '\n')) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << " > Ошибка: система счисления должна быть от 2 до 36\n";
        } else {
            flag = false;
        }
    }
    return result;
}

string read_number(const string& msg) {
    string result;
    cout << msg;
    cin >> result;
    return result;
}

// Проверка корректности числа для заданной системы счисления
bool is_valid_number(const string& num, int base) {
    for (char c : num) {
        int digit;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'A' && c <= 'Z') digit = 10 + (c - 'A');
        else if (c >= 'a' && c <= 'z') digit = 10 + (c - 'a');
        else return false;

        if (digit >= base) return false;
    }
    return true;
}

// Перевод из произвольной системы в десятичную
string arbitrary_to_decimal(const string& num, int from_base) {
    long long result = 0;
    for (char c : num) {
        int digit;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'A' && c <= 'Z') digit = 10 + (c - 'A');
        else digit = 10 + (c - 'a');

        result = result * from_base + digit;
    }
    return to_string(result);
}

// Перевод из десятичной системы в произвольную
string decimal_to_arbitrary(const string& num, int to_base) {
    long long decimal = stoll(num);
    if (decimal == 0) return "0";

    string result;
    while (decimal > 0) {
        int digit = decimal % to_base;
        char c;
        if (digit < 10) c = '0' + digit;
        else c = 'A' + (digit - 10);

        result = c + result;
        decimal /= to_base;
    }
    return result;
}

void frontend() {
    ConversionData data;
    data.from_base = read_base("Введите исходную систему счисления (2-36): ");
    data.to_base = read_base("Введите целевую систему счисления (2-36): ");
    data.number = read_number("Введите число для преобразования: ");

    // Проверка корректности числа
    if (!is_valid_number(data.number, data.from_base)) {
        cerr << "Ошибка: число " << data.number << " недопустимо в системе с основанием " << data.from_base << endl;
        exit(1);
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
    if (data.from_base == 10) {
        result = decimal_to_arbitrary(data.number, data.to_base);
    } else if (data.to_base == 10) {
        result = arbitrary_to_decimal(data.number, data.from_base);
    } else {
        // Двойной перевод: сначала в десятичную, затем в целевую
        string decimal = arbitrary_to_decimal(data.number, data.from_base);
        result = decimal_to_arbitrary(decimal, data.to_base);
    }

    write(pipe_out[1], result.c_str(), result.size() + 1);
}

int do_unnamed_pipes(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    if (argc == 2 && !strcmp(argv[1], "--help")) {
        help();
        exit(0);
    } else if (argc != 1) {
        cout << "Запустите программу с ключом --help для получения справки" << endl;
        exit(1);
    }

    cout << "Программа для перевода чисел между системами счисления (2-36)\n";

    pipe(pipe_in);
    pipe(pipe_out);
    pid = fork();

    if (pid < 0) {
        cerr << "Ошибка: не удалось создать процесс" << endl;
        exit(1);
    } else if (pid > 0) {
        frontend();
    } else {
        backend();
    }

    for (int i = 0; i < 2; ++i) {
        close(pipe_in[i]);
        close(pipe_out[i]);
    }

    return 0;
}