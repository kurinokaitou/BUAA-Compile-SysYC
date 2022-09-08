#include <stdio.h>
int getint() {
    int a;
    scanf("%d", &a);
    return a;
}

void getNum(int b[]) {
    b[0] = getint();
    b[1] = getint();
}

int main() {
    int n = 0, m = 0;
    int a[3][3] = {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
    };
    int b[2] = {0, 0};
    getNum(b);
    int i = 0, j = 0;
    n = b[0];
    m = b[1];
    printf("19373700\n");
    while (i != n) {
        j = 0;
        while (j != m) {
            printf("%d,%d ", i, j);
            j = j + 1;
        }
        i = i + 1;
        printf("\n");
    }
    return 0;
}