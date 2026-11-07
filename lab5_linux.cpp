#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <cmath>

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

int main() {
    int pipes_ParinteCatreCopil[10][2];
    int pipes_CopilCatreParinte[10][2];
    pid_t pids[10];

    for (int i = 0; i < 10; ++i) {
        pipe(pipes_ParinteCatreCopil[i]);
        pipe(pipes_CopilCatreParinte[i]);

        pids[i] = fork();

        if (pids[i] == 0) {
            close(pipes_ParinteCatreCopil[i][1]); 
            close(pipes_CopilCatreParinte[i][0]); 

            int interval[2];
            read(pipes_ParinteCatreCopil[i][0], interval, sizeof(interval));

            vector<int> prime;
            for (int num = interval[0]; num <= interval[1]; ++num) {
                if (isPrime(num)) prime.push_back(num);
            }

            int count = prime.size();
            write(pipes_CopilCatreParinte[i][1], &count, sizeof(count));
            
            if (count > 0) {
                write(pipes_CopilCatreParinte[i][1], prime.data(), count * sizeof(int));
            }

            close(pipes_ParinteCatreCopil[i][0]);
            close(pipes_CopilCatreParinte[i][1]);
            return 0;
        } else {
            close(pipes_ParinteCatreCopil[i][0]);
            close(pipes_CopilCatreParinte[i][1]);
        }
    }

    for (int i = 0; i < 10; ++i) {
        int start = i * 1000 + 1;
        if (i == 0) start = 0; 
        int end = (i + 1) * 1000;
        
        int interval[2] = {start, end};
        write(pipes_ParinteCatreCopil[i][1], interval, sizeof(interval));
        close(pipes_ParinteCatreCopil[i][1]); 
    }

    for (int i = 0; i < 10; ++i) {
        int count;
        read(pipes_CopilCatreParinte[i][0], &count, sizeof(count));
        
        cout << "Procesul " << i + 1 << " a gasit " << count << " numere prime:" << endl;
        if (count > 0) {
            vector<int> prime(count);
            read(pipes_CopilCatreParinte[i][0], prime.data(), count * sizeof(int));
            for (int p : prime) cout << p << " ";
            cout << "\n\n";
        }
        close(pipes_CopilCatreParinte[i][0]);
    }

    for (int i = 0; i < 10; ++i) {
        waitpid(pids[i], NULL, 0);
    }

    return 0;
}