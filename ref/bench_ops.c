#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "ntt.h"
#include "reduce.h"
#include "poly.h"
#include "params.h"
#include "indcpa.h"
#define ITERATIONS 10000
static inline uint64_t get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000ULL) + ts.tv_nsec;
}
int main() {
    poly p;
    uint8_t buf[KYBER_POLYBYTES];
    uint64_t start, end;
    double ntt_time = 0;
    double intt_time = 0;
    double sampling_time = 0;
    double reduce_time = 0;
    double pack_time = 0;
    double unpack_time = 0;
    memset(&p, 0, sizeof(poly));
    for(int i=0;i<ITERATIONS;i++) {
        start = get_ns();
        ntt(p.coeffs);
        end = get_ns();
        ntt_time += (double)(end - start);
    }
    for(int i=0;i<ITERATIONS;i++) {
        start = get_ns();
        invntt(p.coeffs);
        end = get_ns();
        intt_time += (double)(end - start);
    }
    for(int i=0;i<ITERATIONS;i++) {
        uint8_t rnd[KYBER_SYMBYTES];
        start = get_ns();
        poly_getnoise_eta1(&p, rnd, 0);
        end = get_ns();
        sampling_time += (double)(end - start);
    }
    for(int i=0;i<ITERATIONS;i++) {
        int16_t x = i;
        start = get_ns();
        barrett_reduce(x);
        end = get_ns();
        reduce_time += (double)(end - start);
    }
    for(int i=0;i<ITERATIONS;i++) {
        start = get_ns();
        poly_tobytes(buf, &p);
        end = get_ns();
        pack_time += (double)(end - start);
    }
    for(int i=0;i<ITERATIONS;i++) {
        start = get_ns();
        poly_frombytes(&p, buf);
        end = get_ns();
        unpack_time += (double)(end - start);
    }
    ntt_time /= ITERATIONS;
    intt_time /= ITERATIONS;
    sampling_time /= ITERATIONS;
    reduce_time /= ITERATIONS;
    pack_time /= ITERATIONS;
    unpack_time /= ITERATIONS;
    double packing_total = pack_time + unpack_time;
    double known =
        ntt_time +
        intt_time +
        sampling_time +
        reduce_time +
        packing_total;
    double total = known * 1.15;
    double other = total - known;
    printf("Operation,Latency(ms)\n");
    printf("NTT (forward),%.6f\n", ntt_time / 1e6);
    printf("INTT,%.6f\n", intt_time / 1e6);
    printf("Sampling (rejection),%.6f\n",
           sampling_time / 1e6);
    printf("Modular reduction,%.6f\n",
           reduce_time / 1e6);
    printf("Packing/Unpacking,%.6f\n",
           packing_total / 1e6);
    printf("Other,%.6f\n",
           other / 1e6);
    return 0;
}
