#include <stdio.h>
int a = 1;
const int c = 2;
int arr[8] = {1, 2, 3, 4, 1, 2, 3, 4};
int getint() {
    int n;
    scanf("%d", &n);
    return n;
}

void putint(int n) {
    printf("%d", n);
}

int add(int a, int b, int c[]) {
    int d = a + b;
    c[0] = d;
    return d;
}

int main() {
    int i = getint();
    int cnt = 0;
    while (i < 10 && cnt < 100) {
        cnt = cnt + 1;
    }
    putint(cnt);
    return 0;
}
