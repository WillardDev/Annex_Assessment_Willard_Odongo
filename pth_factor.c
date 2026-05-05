#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long pthFactor(long n, long p) {
    long small[100000];
    int count = 0;

    long sq = (long)sqrt((double)n);

    for (long d = 1; d <= sq; d++) {
        if (n % d == 0) {
            small[count++] = d;
        }
    }

    if (p <= count) {
        return small[p - 1];
    }

    p -= count;

    int start = count - 1;
    if (small[start] * small[start] == n) {
        start--;
    }

    if (p > start + 1) {
        return 0;
    }

    return n / small[start - (p - 1)];
}

int main() {
    long n, p;

    printf("Enter n: ");
    scanf("%ld", &n);

    printf("Enter p: ");
    scanf("%ld", &p);

    long result = pthFactor(n, p);
    printf("%ld\n", result);

    return 0;
}