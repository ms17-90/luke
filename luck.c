#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>
#include <sys/time.h>

typedef struct {
    int total_requests;
    int success_count;
    int failure_count;
    double total_response_time;
    pthread_mutex_t lock;
} Stats;

typedef struct {
    char url[256];
    int requests_per_thread;
    Stats *stats;
} ThreadArgs;

void init_stats(Stats *stats) {
    stats->total_requests = 0;
    stats->success_count = 0;
    stats->failure_count = 0;
    stats->total_response_time = 0.0;
    pthread_mutex_init(&stats->lock, NULL);
}

void* thread_task(void *arg) {
    ThreadArgs *args = (ThreadArgs*)arg;
    CURL *curl = curl_easy_init();
    
    if(!curl) {
        fprintf(stderr, "Failed to initialize CURL\n");
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, args->url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // HEAD请求
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    for(int i = 0; i < args->requests_per_thread; i++) {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        
        CURLcode res = curl_easy_perform(curl);
        
        gettimeofday(&end, NULL);
        double response_time = (end.tv_sec - start.tv_sec) * 1000.0 +
                             (end.tv_usec - start.tv_usec) / 1000.0;

        pthread_mutex_lock(&args->stats->lock);
        args->stats->total_requests++;
        if(res == CURLE_OK) {
            args->stats->success_count++;
        } else {
            args->stats->failure_count++;
        }
        args->stats->total_response_time += response_time;
        pthread_mutex_unlock(&args->stats->lock);
    }

    curl_easy_cleanup(curl);
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 4) {
        printf("Usage: %s <url> <ddos> <ddos2>\n", argv[0]);
        return 1;
    }

    char *url = argv[1];
    int thread_count = atoi(argv[2]);
    int requests_per_thread = atoi(argv[3]);

    if(thread_count <= 0 || requests_per_thread <= 0) {
        printf("Invalid parameters\n");
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    Stats stats;
    init_stats(&stats);

    pthread_t threads[thread_count];
    ThreadArgs args = { .requests_per_thread = requests_per_thread, .stats = &stats };
    strncpy(args.url, url, sizeof(args.url)-1);

    struct timeval program_start, program_end;
    gettimeofday(&program_start, NULL);

    // 创建线程
    for(int i = 0; i < thread_count; i++) {
        if(pthread_create(&threads[i], NULL, thread_task, &args) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            return 1;
        }
    }

    // 等待所有线程完成
    for(int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&program_end, NULL);
    double total_time = (program_end.tv_sec - program_start.tv_sec) * 1000.0 +
                      (program_end.tv_usec - program_start.tv_usec) / 1000.0;

    // 输出结果
    printf("\n=== Stress Test Results ===\n");
    printf("URL:               %s\n", url);
    printf("Total Requests:    %d\n", stats.total_requests);
    printf("Successful:        %d\n", stats.success_count);
    printf("Failed:            %d\n", stats.failure_count);
    printf("Total Time:        %.2f ms\n", total_time);
    printf("Requests/sec:      %.2f\n", stats.total_requests / (total_time / 1000.0));
    printf("Average Time:      %.2f ms\n", stats.total_response_time / stats.total_requests);

    pthread_mutex_destroy(&stats.lock);
    curl_global_cleanup();
    return 0;
}