#include <stdio.h>
int getint() {
    int a;
    scanf("%d", &a);
    return a;
}

const int maxSize = 10000;
void merge(int p, int q, int r, int A[]) {
    int left[maxSize], right[maxSize];
    for (int i = 0; i < q - p + 1; ++i)
        left[i] = A[p + i];
    left[q - p + 1] = maxSize;
    right[r - q] = maxSize;
    for (int i = 0; i < r - q; ++i)
        right[i] = A[q + i + 1];
    int i = 0, j = 0;

    for (int k = p; k < r + 1; ++k) {
        if (left[i] <= right[j])
            A[k] = left[i++];
        else
            A[k] = right[j++];
    }
}

void mergeSort(int p, int r, int A[]) {
    if (p < r) {
        int q = (p + r) / 2;
        mergeSort(p, q, A);
        mergeSort(q + 1, r, A);
        merge(p, q, r, A);
    }
}

int g = 0;
int add_g() {
    g = g + 1;
    return g;
}

int main() {
    int n;
    int A[maxSize];
    n = getint();
    for (int i = 0; i < n; ++i) {
        A[i] = getint();
    }
    printf("19373700\n");
    mergeSort(0, n - 1, A);
    for (int i = 0; i < n; ++i) {
        if (A[i] > 100 && add_g()) {
            printf("%d ", A[i]);
        }
    }
    printf("\n%d\n", g);
    return 0;
}