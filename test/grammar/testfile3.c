#include <stdio.h>
int getint() {
    int a;
    scanf("%d", &a);
    return a;
}
const int c1 = 1;
const int cc1 = 1, cc2 = 1;
const int ccc1 = 1, ccc2 = 1, ccc3 = 1;
const int ca1[2] = {1, 2};
const int cca1[2] = {1, 2}, cca2[2] = {1, 2};
const int caa1[2][2] = {{1, 2}, {3, 4}};
const int ccaa1[2][2] = {{1, 2}, {3, 4}}, ccaa2[2][2] = {{1, 2}, {3, 4}};

int v1 = 1;
int vc1 = 1, vc2 = 1;
int vcc1 = 1, vcc2 = 1, vcc3 = 1;
int va1[2] = {1, 2};
int vca1[2] = {1, 2}, vca2[2] = {1, 2};
int vaa1[2][2] = {{1, 2}, {3, 4}};
int vcaa1[2][2] = {{1, 2}, {3, 4}}, vcaa2[2][2] = {{1, 2}, {3, 4}};
int without_para() {
    return 5;
}

int with_para_1(int p) {
    return p * p;
}

int with_para_2(int p1, int p2) {
    return p1 * p2;
}

int with_array_para_1(int a[]) {
    return a[0] * a[0];
}

int with_array_para_2(int a[], int b[]) {
    return a[0] * b[0];
}

int with_double_array_para_1(int a[][2]) {
    return a[0][0] * a[0][0];
}

int with_double_array_para_2(int a[][2], int b[][2]) {
    return a[0][0] * b[0][0];
}

int without_para_void() {
    return 1;
}

void with_para_1_void(int p) {
    int p1 = p * p;
}

void with_para_2_void(int p1, int p2) {
    int p = p1 * p2;
}

void with_array_para_1_void(int a[]) {
    int p = a[0] * a[0];
}

void with_array_para_2_void(int a[], int b[]) {
    int p = a[0] * b[0];
}

void with_double_array_para_1_void(int a[][10]) {
    int p = a[0][0] * a[0][0];
}

void with_double_array_para_2_void(int a[][2], int b[][2]) {
    int p = a[0][0] * b[0][0];
}

int count_odd(int a[], int n) {
    int cnt = 0, i = 0;
    while (i < n) {
        if (a[i] % 2 == 1) {
            cnt = cnt + 1;
        }
        i = i + 1;
    }
    return cnt;
}

int main() {
    int n = 0, m = 0;
    int i = 0;
    int a = 1, b = 2;
    int value = 5;
    int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    n = getint();
    m = getint();
    printf("19373700\n");
    printf("%d\n", n);
    n = n * m;
    m = (n + m) * n;
    {
    }
    if (0) {
        printf("false\n");
    } else {
        printf("true\n");
    }

    if (!0) {
        printf("true\n");
    } else {
        printf("false\n");
    }

    if (a < b) {
        printf("a<b\n");
    }
    if (a > b) {
        printf("a>b\n");
    }
    if (a <= b) {
        printf("a<=b\n");
    }
    if (a >= b) {
        printf("a>=b\n");
    }
    if (a == b) {
        printf("a==b\n");
    }
    if (a != b) {
        printf("a!=b\n");
    }
    while (1) {
        value = value - 1;
        if (value < 0) {
            break;
        } else {
            continue;
        }
    }
    value = 5;
    value = -+-5;
    value = (value + a) * b;
    value = -value;
    value = without_para();
    value = with_para_1(value);
    value = with_para_2(a, b);
    with_array_para_1_void(vcaa1[0]);
    with_array_para_2_void(vcaa1[0], vcaa2[0]);
    array[0] = with_array_para_1(va1);
    array[0] = with_array_para_2(vca1, vca2);
    vaa1[0][0] = 1;
    vaa1[0][0] = with_double_array_para_1(vcaa1);
    vaa1[0][0] = with_double_array_para_2(vcaa1, vcaa2);
    value = (value + array[0]) * (array[1] - vaa1[0][0]) / 2;
    printf("value = %d\n", value % 2);
    printf("odds = %d\n", count_odd(array, 10));
    return 0;
}