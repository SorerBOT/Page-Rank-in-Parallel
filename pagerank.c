#include "pagerank.h"
#include <stdlib.h>

// #ifdef _PAGE_RANK_IMPLEMENTATION


typedef struct
{
    node** root_nodes;
    size_t* outlinks;
    size_t root_nodes_num;

} outlinks_count_args;

outlinks_count_args* outlinks_count_args_make(node** root_nodes, size_t* outlinks, size_t root_nodes_num)
{
    outlinks_count_args* args = PR_MALLOC(sizeof(outlinks_count_args));
    *args = (outlinks_count_args)
    {
        .root_nodes     = root_nodes,
        .root_nodes_num = root_nodes_num,
        .outlinks       = outlinks
    };
    return args;
}
void* outlinks_count(void* _args)
{
    outlinks_count_args* args = _args;
    size_t outlinks = 0;
    for (size_t i = 0; i < args->root_nodes_num; ++i)
    {
        node* node_current = args->root_nodes[i];
        while (node_current != NULL)
        {
            ++outlinks;
            node_current = node_current->next;
        }
        args->outlinks[i] = outlinks;
    }
    return NULL;
}
void compute_outlinks_parallel(Graph* graph, size_t* node_outlinks, size_t node_num, long cores_num)
{
    thr_pool_t* pool = thr_pool_create(cores_num, cores_num, 0, NULL);
    if (pool == NULL)
    {
#ifdef _PR_PRINT
        fprintf(stderr, "Error: thr_pool_create()");
#endif
        exit(EXIT_FAILURE);
    }

    for (long i = 0; i < node_num; ++i)
    {
        int res = thr_pool_queue(pool, outlinks_count,
                outlinks_count_args_make(&graph->adjacencyLists[i], &node_outlinks[i], 1));
        if (res == -1)
        {
#ifdef _PR_PRINT
            fprintf(stderr, "Error: thr_pool_queue()");
#endif
            exit(EXIT_FAILURE);
        }
    }
    thr_pool_wait(pool);
    thr_pool_destroy(pool);
}

typedef struct
{
    float* arr;
    size_t arr_len;
    float value;
} fill_arr_sequential_args;

fill_arr_sequential_args* fill_arr_sequential_args_make(float* arr, size_t arr_len, float value)
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
        pthread_create(&t_ids[i], NULL, fill_arr_sequential, fill_arr_sequential_args_make(arr, chunk_size, value));
        arr += chunk_size;
    }
    pthread_create(&t_ids[cores_num-1], NULL, fill_arr_sequential, fill_arr_sequential_args_make(arr, arr_len % (cores_num-1), value));

    for (size_t i = 0; i < cores_num; ++i)
    {
        pthread_join(t_ids[i], NULL);
    }
}

void perform_iteration(Graph* graph, float* new_pageranks, float* old_pageranks, size_t* outlinks)
{
    const size_t N = graph->numVertices;
    float page_rank_sum_no_outlinks = 0;
    for (size_t i = 0; i < N; ++i)
    {
        node* outlink = graph->adjacencyLists[i];
        if (outlink == NULL)
        {
            page_rank_sum_no_outlinks += old_pageranks[i] / N;
        }
        else
        {
            while (outlink != NULL)
            {
                new_pageranks[outlink->v] += PR_DAMPING_FACTOR * (old_pageranks[outlink->v] + old_pageranks[i] / outlinks[i]);
                outlink = outlink->next;
            }
        }
    }
    for (size_t i = 0; i < N; ++i)
    {
        new_pageranks[i] += PR_DAMPING_FACTOR * (page_rank_sum_no_outlinks);
    }

}

void PageRank(Graph *graph, int n, float* ranks)
{
    const long cores_num  = sysconf(_SC_NPROCESSORS_ONLN);
    if (cores_num == -1)
    {
#ifdef _PR_PRINT
        fprintf(stderr, "Error: sysconf");
#endif
        exit(EXIT_FAILURE);
    }

    const size_t N          = graph->numVertices;

    fill_arr_parallel(ranks, N, (1-PR_DAMPING_FACTOR)/N, cores_num);

    /*
     * 1. Compute outlinks
     * 2. Parallelise perform_iteration
     * 3. Call perform_iteration n times
     */

    /* 1. Compute Outlinks */
    size_t* outlinks  = PR_MALLOC(N * sizeof(size_t));
    compute_outlinks_parallel(graph, outlinks, N, cores_num);

    for (size_t i = 0; i < N; ++i)
    {
        printf("Node %lu has %lu outlinks\n", i, outlinks[i]);
    }

    float* new_ranks = PR_MALLOC(N * sizeof(float));
    float* ptr_temp;
    for (size_t i = 0; i < n; ++i)
    {
        perform_iteration(graph, new_ranks, ranks, outlinks);
        ptr_temp = new_ranks;
        new_ranks = ranks;
        ranks = ptr_temp;
    }

    free(new_ranks);
    free(outlinks);
}

// #endif /* _PAGE_RANK_IMPLEMENTATION */
