#include <cstdint>
#include <stdlib.h>
#include <getopt.h>

#include "../src/TimeUtils.h"
#include "../src/Numb.h"
#include "../src/ExtUtil.h"

void *
aligned_malloc(size_t size, int alignment)
{
    const int pointerSize = sizeof(void *);

    const int requestedSize = size + alignment - 1 + pointerSize;

#ifdef CONFIG_BIGPAGE
    void *raw = big_page_alloc(requestedSize);
#else
    void *raw = malloc(requestedSize);
#endif

    uintptr_t start = (uintptr_t)raw + pointerSize;
    void *aligned   = (void *)((start + alignment - 1) & ~(alignment - 1));

    *(void **)((uintptr_t)aligned - pointerSize) = raw;

    return aligned;
}

template <typename T>
void
aligned_free(T *aligned_ptr)
{
    if (aligned_ptr) {
        free(((T **)aligned_ptr)[-1]);
    }
}

bool
isAligned(void *data, int alignment)
{
    return ((uintptr_t)data & (alignment - 1)) == 0;
}

class ModTest
{
  private:
    long logN, N;
    uint64_t *ax, *bx, *br, *c1, *c2;
    uint64_t q, qInv;

  public:
    ModTest(long logN) : logN(logN)
    {
        srand(time(NULL));
        N = 1 << logN;

        ax = (uint64_t *)aligned_malloc(sizeof(uint64_t) << logN, 64);
        bx = (uint64_t *)aligned_malloc(sizeof(uint64_t) << logN, 64);
        br = (uint64_t *)aligned_malloc(sizeof(uint64_t) << logN, 64);

        c1 = (uint64_t *)aligned_malloc(sizeof(uint64_t) << logN, 64);
        c2 = (uint64_t *)aligned_malloc(sizeof(uint64_t) << logN, 64);

        q    = (1l << 32) + rand() % (1l << 32);
        qInv = inv(q);
        set_mod(q, qInv);

        if (!isAligned(ax, 64)) {
            printf("not aligned\n");
            exit(1);
        }
        if (!isAligned(bx, 64)) {
            printf("not aligned\n");
            exit(1);
        }
        if (!isAligned(br, 64)) {
            printf("not aligned\n");
            exit(1);
        }

        srand(time(NULL));
        for (int i = 0; i < N; ++i) {
            ax[i] = rand() % q;
            bx[i] = rand() % q;
            br[i] = rand() % q;
        }
    }

    inline uint64_t
    soft_addmod(uint64_t a, uint64_t b)
    {
        uint64_t c = a + b;
        if (c > q)
            return c - q;
        else
            return c;
    }

    inline uint64_t
    hard_addmod(uint64_t a, uint64_t b)
    {
        return addmod(a, b);
    }

    inline uint64_t
    soft_montredc(uint64_t a, uint64_t b)
    {
        unsigned __int128 U  = static_cast<unsigned __int128>(a) * b;
        uint64_t U0          = static_cast<uint64_t>(U);
        uint64_t U1          = static_cast<uint64_t>(U >> 64);
        uint64_t Q           = U0 * qInv;
        unsigned __int128 Hx = static_cast<unsigned __int128>(Q) * q;
        uint64_t H           = static_cast<uint64_t>(Hx >> 64);
        uint64_t V           = U1 < H ? U1 + q - H : U1 - H;
        return V;
    }

    inline uint64_t
    hard_montredc(uint64_t a, uint64_t b)
    {
        return mont_redc(a, b);
    }

    inline uint64_t
    soft_barmulmod(uint64_t a, uint64_t b, uint64_t r)
    {
        uint64_t z1 = a * b;
        uint64_t t  = (static_cast<__uint128_t>(a) * r) >> 64;
        uint64_t t1 = q * t;
        uint64_t z  = z1 - t1;
        return z > q ? z - q : z;
    }

    inline uint64_t
    hard_barmulmod(uint64_t a, uint64_t b, uint64_t r)
    {
        return bar_mulmod_s(a, b, r);
    }

    void
    check_result()
    {
        for (int i = 0; i < N; i++) {
            if (c1[i] != c2[i]) {
                printf("error in idx: %d.\n", i);
                exit(1);
            }
        }
    }

    void
    addmod_test()
    {
        TimeUtils time_log;
        time_log.start("soft addmod");
        STAT_RESET;
        for (int i = 0; i < N; i++) {
            c1[i] = soft_addmod(ax[i], bx[i]);
        }
        STAT_RESET;
        time_log.stop("soft addmod");

        time_log.start("hard addmod");
        STAT_RESET;
        for (int i = 0; i < N; i++) {
            c2[i] = hard_addmod(ax[i], bx[i]);
        }
        STAT_RESET;
        time_log.stop("hard addmod");

        check_result();
    }

    void
    montredc_test()
    {
        TimeUtils time_log;
        time_log.start("soft Montgomeny REDC");
        STAT_RESET;
        for (int i = 0; i < N; i++) {
            c1[i] = soft_montredc(ax[i], bx[i]);
        }
        STAT_RESET;
        time_log.stop("soft Montgomeny REDC");

        time_log.start("hard Montgomeny REDC");
        STAT_RESET;
        for (int i = 0; i < N; i++) {
            c2[i] = hard_montredc(ax[i], bx[i]);
        }
        STAT_RESET;
        time_log.stop("hard Montgomeny REDC");

        check_result();
    }

    void
    barmulmod_test()
    {
        TimeUtils time_log;
        time_log.start("soft Barrett MulMod");
        STAT_RESET;
        for (int i = 0; i < N; i++) {
            c1[i] = soft_barmulmod(ax[i], bx[i], br[i]);
        }
        STAT_RESET;
        time_log.stop("soft Barrett MulMod");

        time_log.start("hard Barrett MulMod");
        STAT_RESET;
        for (int i = 0; i < N; i++) {
            c2[i] = hard_barmulmod(ax[i], bx[i], br[i]);
        }
        STAT_RESET;
        time_log.stop("hard Barrett MulMod");

        check_result();
    }
};

int
main(int argc, const char *argv[])
{

    int opt;
    int optind;

    struct option longopts[] = {{"case", 1, NULL, 'k'},
                                {"logN", 1, NULL, 'N'}};
    const char *optstring    = "ck:N:";

    long logN = -1;
    string case_name;

    while ((opt = getopt_long(argc, (char *const *)argv, optstring, longopts,
                              &optind)) != -1)
    {
        switch (opt) {
            case 'N': {
                logN = strtol(optarg, NULL, 10);
                break;
            }
            case 'k': {
                case_name = string(optarg);
                break;
            }
            default: {
                printf("invalid option %c", opt);
                exit(1);
            }
        }
    }

    cout << "case: " << case_name << endl << "logN: " << logN << endl;
    ModTest test(logN);
    if (case_name == "addmod") {
        test.addmod_test();
    }
    if (case_name == "barmulmod") {
        test.barmulmod_test();
    }
    if (case_name == "montredc") {
        test.montredc_test();
    }
}
