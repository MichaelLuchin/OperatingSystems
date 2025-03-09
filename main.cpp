#include <iostream>
#include <math.h>

using namespace std;

int septToDec(string &sept);

bool isSept(string &sept);

int main() {
    string sept;
    cin >> sept;

    if(isSept(sept)) {
        cout << septToDec(sept);
    }
    return 0;
}

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
