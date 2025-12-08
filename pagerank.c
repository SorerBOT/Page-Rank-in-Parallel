#include "pagerank.h"

// #ifdef _PAGE_RANK_IMPLEMENTATION

typedef struct
{
    float* arr;
    size_t arr_len;
    float value;
} fill_arr_sequential_args;

fill_arr_sequential_args* fill_arr_sequential_make_args(float* arr, size_t arr_len, float value)
{
    fill_arr_sequential_args* args = PR_MALLOC(sizeof(fill_arr_sequential_args));
    *args = (fill_arr_sequential_args)
    {
        .arr = arr,
        .arr_len = arr_len,
        .value = value
    };
    return args;
}
void* fill_arr_sequential(void* _args)
{
    fill_arr_sequential_args* args = _args;
    for (size_t i = 0; i < args->arr_len; ++i)
    {
        args->arr[i] = args->value;
    }

    free(args);
    return NULL;
}

void fill_arr_parallel(float* arr, size_t arr_len, float value, size_t cores_num)
{
    pthread_t* t_ids = PR_MALLOC(cores_num * sizeof(pthread_t));
    size_t chunk_size = arr_len / (cores_num-1);

    for (size_t i = 0; i < cores_num-1; ++i)
    {
        pthread_create(&t_ids[i], NULL, fill_arr_sequential, fill_arr_sequential_make_args(arr, chunk_size, value));
        arr += chunk_size;
    }
    pthread_create(&t_ids[cores_num-1], NULL, fill_arr_sequential, fill_arr_sequential_make_args(arr, arr_len % (cores_num-1), value));

    for (size_t i = 0; i < cores_num; ++i)
    {
        pthread_join(t_ids[i], NULL);
    }
}

void PageRank(Graph *graph, int n, float* ranks)
{
    const size_t cores_num  = sysconf(_SC_NPROCESSORS_ONLN);
    const size_t N          = graph->numVertices;

    fill_arr_parallel(ranks, N, 1.0/N, cores_num);

    printf("[");
    for (size_t i = 0; i < N; ++i)
    {
        printf("%f, ", ranks[i]);
    }
    printf("]\n");
}

// #endif /* _PAGE_RANK_IMPLEMENTATION */
