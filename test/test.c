#include <stdio.h>
int add(int a, int b, int c[][2]) {
    return a + b;
}

int main() {
    int cnt = 0;
    while (cnt < 100) {
        cnt = cnt + 1;
    }
    printf("cnt = %d\n", cnt);
    return 0;
}