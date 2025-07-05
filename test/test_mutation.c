#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char **argv)
{
    int *arr = malloc(10 * sizeof(int));
    int i, idx = 8;

    for (i = 0; i < 10; i++)
        arr[i] = i;

    int next_slot = idx + 1;
    while (1)
    {
        /*next slot is empty*/
        if (arr[next_slot % (10)] == 0)
        {
            arr[next_slot % 10] = arr[idx];
            arr[idx] = 0;
            break;
        }
        else
            next_slot++;
    }

    printf("%d -> %d\n", arr[0], arr[8]);

    assert(arr[8] == 0);
    assert(arr[0] == 8);

    free(arr);

    return 0;
}