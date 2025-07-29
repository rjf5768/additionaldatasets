#include <klee/klee.h>
#include <assert.h>
#include <stdio.h>

int binarysearch1(const int *arr, int l, int r, int x)
{
    if (r >= l)
    {
        int mid = l + (r - l) / 2;

        // If element is present at middle
        if (arr[mid] == x)
            return mid;

        // If element is smaller than middle
        if (arr[mid] > x)
            return binarysearch1(arr, l, mid - 1, x);

        // Else element is in right subarray
        return binarysearch1(arr, mid + 1, r, x);
    }

    // When element is not present in array
    return -1;
}

int binarysearch2(const int *arr, int l, int r, int x)
{
    int mid = l + (r - l) / 2;

    while (arr[mid] != x)
    {
        if (r <= l || r < 0)
            return -1;

        if (arr[mid] > x)
            // If element is smaller than middle
            r = mid - 1;
        else
            // Else element is in right subarray
            l = mid + 1;

        mid = l + (r - l) / 2;
    }

    // When element is not present in array
    return mid;
}

int main()
{
    int arr[5] = {2, 3, 4, 10, 40};
    int n = 5;
    int x;
    
    // Make search value symbolic
    klee_make_symbolic(&x, sizeof(x), "search_value");
    
    // Add constraints for reasonable search values
    klee_assume(x >= 0 && x <= 50);
    
    // Test both implementations
    int result1 = binarysearch1(arr, 0, n - 1, x);
    int result2 = binarysearch2(arr, 0, n - 1, x);
    
    // Both implementations should return the same result
    klee_assert(result1 == result2);
    
    // If found, verify the value at the returned index
    if (result1 != -1) {
        klee_assert(result1 >= 0 && result1 < n);
        klee_assert(arr[result1] == x);
    }
    
    return 0;
} 