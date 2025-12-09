#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

#define _PR_PRINT 1
#define _PR_SAFE_ALLOC 1
#include "pagerank.h"

#define D 0.15
static void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s <num_vertices> <num_iterations> < input_edges.txt\n",
            prog);
}

void initializeRanks(float *ranks, int N) {
    for (int i = 0; i < N; i++) {
        ranks[i] = 1.0 / N;
    }
}
void PageRankSequential(Graph *graph, int iterations, float* ranks) {
    int N = graph->numVertices;
    float *newRanks = (float *)malloc(N * sizeof(float));
    int* outlinkes = (int*)malloc(N* sizeof(int));

    initializeRanks(ranks, N);
    
    //outlinks calculations
    
    for (int i = 0; i < N; i++)
    {
        node* v = graph->adjacencyLists[i];
        while (v != NULL)
        {
            outlinkes[i]++;
            v = v->next;
        }
    }
    
    for (int iter = 0; iter < iterations; iter++) {
        //calculate nodes with outlinks to i
        for (int i = 0; i < N; i++) {
            vertex* out2i = (vertex*)malloc(N * sizeof(vertex));
            
            for(int j = 0; j < N; j++) {
                if( j == i) continue;
                node* v = graph->adjacencyLists[j];
                while (v != NULL) {
                    if (v->v == i) {
                        out2i[j] = 1;
                        break;
                    }
                    v = v->next;
                }
            }

            //calculate i rank
            double sumA = 0.0;
            double sumB = 0.0;
            for (int j = 0 ; j < N; j++)
            {
                if(out2i[j] == 1)
                {
                    sumA += ranks[j]/outlinkes[j];
                } else if(outlinkes[j] == 0)
                {
                    sumB += ranks[j]/N;
                }
            }
            newRanks[i] = D/N +(1-D)*(sumA+sumB);
        }

        for (int i = 0; i < N; i++) {
            ranks[i] = newRanks[i];
        }
    }

    free(newRanks);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        usage(argv[0]);
        return 1;
    }

    char *endptr = NULL;
    long n_vertices = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || n_vertices <= 0) {
        fprintf(stderr, "Invalid num_vertices: %s\n", argv[1]);
        return 1;
    }

    long n_iters = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || n_iters <= 0) {
        fprintf(stderr, "Invalid num_iterations: %s\n", argv[2]);
        return 1;
    }

    Graph *g = createGraph((int)n_vertices);
    if (!g) {
        fprintf(stderr, "Failed to create graph\n");
        return 1;
    }
    
    // Read directed edges from stdin: "u v" per line.
    int u, v;
    while (scanf("%d %d", &u, &v) == 2) {
        if (u < 0 || u >= n_vertices || v < 0 || v >= n_vertices) {
            fprintf(stderr,
                    "Warning: ignoring out-of-range edge %d -> %d\n", u, v);
            continue;
        }
        addEdge(g, u, v);
    }

    // Allocate rank array and run PageRank.
    float *rank = (float *)malloc(sizeof(float) * n_vertices);
    if (!rank) {
        fprintf(stderr, "Failed to allocate rank array\n");
        return 1;
    }
    float *rank_sequential = (float *)malloc(sizeof(float) * n_vertices);
    if (!rank_sequential) {
        fprintf(stderr, "Failed to allocate rank array\n");
        return 1;
    }

    PageRank(g, (int)n_iters, rank);
    PageRankSequential(g, (int)n_iters, rank_sequential);

    // Print final ranks, one per line: "vertex rank"
    double sum = 0.f, sum_sequential = 0.f;
    for (int i = 0; i < n_vertices; i++) {
        printf("%d %.6f-%.6f=%.6f\n", i, rank[i], rank_sequential[i], rank[i] - rank_sequential[i]);
        sum += rank[i];
        sum_sequential += rank_sequential[i];
    }
    fprintf(stderr, "Sum of ranks = %.6f, sum sequential = %.6f, diff = %.6f\n", sum, sum_sequential, sum - sum_sequential);

    free(rank);
    // We skip freeing the graph (simple test driver); OS will reclaim it.

    return 0;
}
