#include <iostream>
#include <math.h>
#include "unixfilework.cpp"

using namespace std;

int septToDec(string &sept){
    int dec = 0;

    for(int i = sept.size()-1; i >= 0; i--){
        dec += (sept[i] - 48) * pow(7, sept.size()-i-1);
    }
    return dec;
}

bool isSept(string &sept){
    for(char s : sept) {
        if (s - 48 > 6) {
            cout << "Not septenary or numeric";
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    unix_file_work(argc, argv);
    return 0;
}


