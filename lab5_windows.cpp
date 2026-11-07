#include <iostream>
#include <windows.h>
#include <string>
#include <vector>

using namespace std;

bool isPrime(int n) {
    if (n <= 1) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        
        HANDLE hReadPipe = (HANDLE)stoull(argv[1]);
        HANDLE hWritePipe = (HANDLE)stoull(argv[2]);

        int interval[2];
        DWORD readBytes;
        ReadFile(hReadPipe, interval, sizeof(interval), &readBytes, NULL);

        vector<int> prime;
        for (int i = interval[0]; i <= interval[1]; ++i) {
            if (isPrime(i)) prime.push_back(i);
        }

        int count = prime.size();
        DWORD writtenBytes;
        WriteFile(hWritePipe, &count, sizeof(count), &writtenBytes, NULL);
        if (count > 0) {
            WriteFile(hWritePipe, prime.data(), count * sizeof(int), &writtenBytes, NULL);
        }
        return 0; 
    }

    STARTUPINFOA si[10];
    PROCESS_INFORMATION pi[10];
    HANDLE hParentWrite[10];
    HANDLE hParentRead[10];
    HANDLE handleProcese[10];

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    for (int i = 0; i < 10; ++i) {
        HANDLE hChildRead, hChildWrite;
        
        CreatePipe(&hChildRead, &hParentWrite[i], &sa, 0);
        CreatePipe(&hParentRead[i], &hChildWrite, &sa, 0);

        SetHandleInformation(hParentWrite[i], HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(hParentRead[i], HANDLE_FLAG_INHERIT, 0);

        ZeroMemory(&si[i], sizeof(si[i]));
        si[i].cb = sizeof(si[i]);
        ZeroMemory(&pi[i], sizeof(pi[i]));

        char cmdArgs[256];
        snprintf(cmdArgs, sizeof(cmdArgs), "\"%s\" %llu %llu", argv[0], (unsigned long long)hChildRead, (unsigned long long)hChildWrite);

        CreateProcessA(NULL, cmdArgs, NULL, NULL, TRUE, 0, NULL, NULL, &si[i], &pi[i]);
        handleProcese[i] = pi[i].hProcess;

        CloseHandle(hChildRead);
        CloseHandle(hChildWrite);
    }

    for (int i = 0; i < 10; ++i) {
        int start = (i == 0) ? 0 : i * 1000 + 1;
        int end = (i + 1) * 1000;
        int interval[2] = {start, end};
        
        DWORD bytesWritten;
        WriteFile(hParentWrite[i], interval, sizeof(interval), &bytesWritten, NULL);
        CloseHandle(hParentWrite[i]); // Închidem scrierea
    }

    for (int i = 0; i < 10; ++i) {
        int count;
        DWORD bytesRead;
        ReadFile(hParentRead[i], &count, sizeof(count), &bytesRead, NULL);
        
        cout << "Procesul " << i + 1 << " a gasit " << count << " numere prime:" << endl;
        if (count > 0) {
            vector<int> prime(count);
            ReadFile(hParentRead[i], prime.data(), count * sizeof(int), &bytesRead, NULL);
            for(int p : prime) cout << p << " ";
            cout << "\n\n";
        }
        CloseHandle(hParentRead[i]);
    }

    WaitForMultipleObjects(10, handleProcese, TRUE, INFINITE);
    for (int i = 0; i < 10; ++i) {
        CloseHandle(pi[i].hProcess);
        CloseHandle(pi[i].hThread);
    }

    return 0;
}