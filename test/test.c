#include <stdio.h>
int a = 1;
const int c = 2;
int arr[3] = {1, 2, 3};

int main() {
    int n = 0;
    int sum = 0;
    if (n > c && sum < arr[1]) {
        a = a * 2;
    }
    printf("%d\n", sum);
}