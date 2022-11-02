#include <stdio.h>
int a = 1;
const int c = 2;
int arr[2][2][2] = {{{1, 2}, {3, 4}}, {{1, 2}, {3, 4}}};

int add(int a, int b, int c[]) {
    int d = a + b;
    c[0] = d;
    return d;
}

int main() {
    int var = add(1, 2, arr[1][1]);
    printf("%d", var);
}