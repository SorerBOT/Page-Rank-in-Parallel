#include "pagerank.h"
#include "graph.h"
#include <pthread.h>
#include <stdlib.h>

// #ifdef _PAGE_RANK_IMPLEMENTATION

typedef struct
{
    float* src;
    float* dest;
    size_t arr_len;
} copy_arr_sequential_args;
copy_arr_sequential_args* copy_arr_sequential_args_make(float* src, float* dest, size_t arr_len)
{
    copy_arr_sequential_args* args = PR_MALLOC(sizeof(copy_arr_sequential_args));
    *args = (copy_arr_sequential_args)
    {
        .src = src,
        .dest = dest,
        .arr_len = arr_len
    };
    return args;
}
void* copy_arr_sequential(void* _args)
{
    copy_arr_sequential_args* args = _args;
    for (size_t i = 0; i < args->arr_len; ++i)
    {
        args->src[i] = args->dest[i];
    }
    free(args);
    return NULL;
}
void copy_arr_parallel(float* src, float* dest, size_t arr_len, long cores_num)
{
    pthread_t* t_ids = PR_MALLOC(cores_num * sizeof(pthread_t));
    size_t chunk_size = arr_len / (cores_num-1);

    for (size_t i = 0; i < cores_num-1; ++i)
    {
        pthread_create(&t_ids[i], NULL, copy_arr_sequential, copy_arr_sequential_args_make(src, dest, chunk_size));
        src += chunk_size;
        dest += chunk_size;
    }
    pthread_create(&t_ids[cores_num-1], NULL, copy_arr_sequential, copy_arr_sequential_args_make(src, dest, arr_len % (cores_num-1)));

    for (size_t i = 0; i < cores_num; ++i)
    {
        pthread_join(t_ids[i], NULL);
    }
}

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
} add_arr_sequential_args;

add_arr_sequential_args* add_arr_sequential_args_make(float* arr, size_t arr_len, float value)
{
    add_arr_sequential_args* args = PR_MALLOC(sizeof(add_arr_sequential_args));
    *args = (add_arr_sequential_args)
    {
        .arr = arr,
        .arr_len = arr_len,
        .value = value
    };
    return args;
}
void* add_arr_sequential(void* _args)
{
    add_arr_sequential_args* args = _args;
    for (size_t i = 0; i < args->arr_len; ++i)
    {
        args->arr[i] += args->value;
    }
    free(args);
    return NULL;
}

void add_arr_parallel(float* arr, size_t arr_len, float value, size_t cores_num)
{
    pthread_t* t_ids = PR_MALLOC(cores_num * sizeof(pthread_t));
    size_t chunk_size = arr_len / (cores_num-1);

    for (size_t i = 0; i < cores_num-1; ++i)
    {
        pthread_create(&t_ids[i], NULL, add_arr_sequential, add_arr_sequential_args_make(arr, chunk_size, value));
        arr += chunk_size;
    }
    pthread_create(&t_ids[cores_num-1], NULL, add_arr_sequential, add_arr_sequential_args_make(arr, arr_len % (cores_num-1), value));

    for (size_t i = 0; i < cores_num; ++i)
    {
        pthread_join(t_ids[i], NULL);
    }
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

typedef struct {
    mutex* page_rank_sum_no_outlinks_mutex;
    Graph* graph;
    vertex v;
    node* outlinks_root;
    size_t outlinks_num;
    float old_pagerank;
    float* new_pageranks;
    float* page_rank_sum_no_outlinks_ptr;
} update_all_linked_pages_args;
update_all_linked_pages_args* update_all_linked_pages_args_make(mutex* page_rank_sum_no_outlinks_mutex, Graph* graph, vertex v, node* outlinks_root, size_t outlinks_num,
        float old_pagerank, float* new_pageranks,
        float* page_rank_sum_no_outlinks_ptr)
{
    update_all_linked_pages_args* args = PR_MALLOC(sizeof(update_all_linked_pages_args));
    *args = (update_all_linked_pages_args)
    {
        .page_rank_sum_no_outlinks_mutex = page_rank_sum_no_outlinks_mutex,
        .graph = graph,
        .v = v,
        .outlinks_root = outlinks_root,
        .outlinks_num = outlinks_num,
        .old_pagerank = old_pagerank,
        .new_pageranks = new_pageranks,
        .page_rank_sum_no_outlinks_ptr = page_rank_sum_no_outlinks_ptr
    };
    return args;
}
void* update_all_linked_pages(void* _args)
{
    update_all_linked_pages_args* args = _args;
    if (args->outlinks_root == NULL)
    {
        pthread_mutex_lock(args->page_rank_sum_no_outlinks_mutex);
        *args->page_rank_sum_no_outlinks_ptr += args->old_pagerank;
        pthread_mutex_unlock(args->page_rank_sum_no_outlinks_mutex);
    }
    else
    {
        while (args->outlinks_root != NULL)
        {
            if (args->outlinks_root->v != args->v)
            {
                pthread_mutex_lock(&args->graph->num_visits_mutexes[args->outlinks_root->v]);
                args->new_pageranks[args->outlinks_root->v] += PR_DAMPING_FACTOR * (args->old_pagerank / args->outlinks_num);
                pthread_mutex_unlock(&args->graph->num_visits_mutexes[args->outlinks_root->v]);
            }
            args->outlinks_root = args->outlinks_root->next;
        }
    }

    free(args);
    return NULL;
}

void perform_iteration(thr_pool_t* pool, Graph* graph, float* new_pageranks, float* old_pageranks, size_t* outlinks, long cores_num)
{
    const size_t N = graph->numVertices;
    float page_rank_sum_no_outlinks = 0.f;

    for (size_t i = 0; i < N; ++i)
    {
        mutex page_rank_sum_no_outlinks_mutex;
        pthread_mutex_init(&page_rank_sum_no_outlinks_mutex, NULL);

    int res = thr_pool_queue(pool, update_all_linked_pages, update_all_linked_pages_args_make(&page_rank_sum_no_outlinks_mutex, graph, (vertex)i, graph->adjacencyLists[i], outlinks[i], old_pageranks[i], new_pageranks, &page_rank_sum_no_outlinks));
    if (res == -1)
    {
#ifdef _PR_PRINT
        fprintf(stderr, "Error: thr_pool_queue()");
#endif
        exit(EXIT_FAILURE);
    }

        pthread_mutex_lock(&graph->num_visits_mutexes[i]);
        new_pageranks[i] += (1-PR_DAMPING_FACTOR)/N;
        pthread_mutex_unlock(&graph->num_visits_mutexes[i]);
    }
    thr_pool_wait(pool);
    add_arr_parallel(new_pageranks, N, PR_DAMPING_FACTOR * (page_rank_sum_no_outlinks) / N, cores_num);
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
    fill_arr_parallel(ranks, N, 1.f/N, cores_num);

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
    float* ptr_temp = NULL;
    thr_pool_t* pool = thr_pool_create(cores_num, cores_num, 0, NULL);
    if (pool == NULL)
    {
#ifdef _PR_PRINT
        fprintf(stderr, "Error: thr_pool_create()");
#endif
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < n; ++i)
    {
        float sum_ranks = 0.f;
        for (size_t j = 0; j < N; ++j)
        {
            sum_ranks += ranks[j];
        }
        printf("%lu: sum %.6f\n", i, sum_ranks);
        fill_arr_parallel(new_ranks, N, 0.f, cores_num);
        perform_iteration(pool, graph, new_ranks, ranks, outlinks, cores_num);

        ptr_temp = new_ranks;
        new_ranks = ranks;
        ranks = ptr_temp;
    }
    thr_pool_destroy(pool);

    if (n % 2 == 1)
    {
        copy_arr_parallel(ranks, new_ranks, N, cores_num);
    }
    for (size_t j = 0; j < N; ++j)
    {
        printf("Element %lu in ranks is: %.2f\n", j, ranks[j]);
    }

    free(new_ranks);
    free(outlinks);
}

// #endif /* _PAGE_RANK_IMPLEMENTATION */
