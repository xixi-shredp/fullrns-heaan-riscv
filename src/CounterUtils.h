#include "Common.h"
using namespace std;
#ifdef CONFIG_XUANTIE_RISCV
#define COUNTER_ALL_MAP(f)                                              \
    f( CYCLE )                                                          \
    f( TIME )                                                           \
    f( INSTRET )                                                        \
    f( L1I_ACCESS)                                                      \
    f( L1I_MISS )                                                       \
    f( I_UTLB_MISS )                                                    \
    f( D_UTLB_MISS )                                                    \
    f( JTLB_MISS )                                                      \
    f( COND_BRANCH_MISS )                                               \
    f( COUNTER9_RESERVED)                                               \
    f( IND_BRANCH_MISS )                                                \
    f( IND_BRANCH_INS )                                                 \
    f( LSU_SPEC_FAIL )                                                  \
    f( STORE_INS )                                                      \
    f( L1D_READ_ACCESS )                                                \
    f( L1D_READ_MISS )                                                  \
    f( L1D_WRITE_ACCESS )                                               \
    f( L1D_WRITE_MISS )                                                 \
    f( L2_READ_ACCESS )                                                 \
    f( L2_READ_MISS )                                                   \
    f( L2_WRITE_ACCESS )                                                \
    f( L2_WRITE_MISS )                                                  \
    f( RF_LNCH_FAIL )                                                   \
    f( RF_REG_LNCH_FAIL )                                               \
    f( RF_INST )                                                        \
    f( LSU_CRS_4K_STALL )                                               \
    f( LSU_OTHER_STALL )                                                \
    f( LSU_SQ_DISC )                                                    \
    f( LSU_SQ_DATA_DISC )

#define COUNTER_NUMBERS_MAP(f)                                          \
    f( 3 )                                                              \
    f( 4 )                                                              \
    f( 5 )                                                              \
    f( 6 )                                                              \
    f( 7 )                                                              \
    f( 8 )                                                              \
    f( 9 )                                                              \
    f( 10 )                                                             \
    f( 11 )                                                             \
    f( 12 )                                                             \
    f( 13 )                                                             \
    f( 14 )                                                             \
    f( 15 )                                                             \
    f( 16 )                                                             \
    f( 17 )                                                             \
    f( 18 )                                                             \
    f( 19 )                                                             \
    f( 20 )                                                             \
    f( 21 )                                                             \
    f( 22 )                                                             \
    f( 23 )                                                             \
    f( 24 )                                                             \
    f( 25 )                                                             \
    f( 26 )                                                             \
    f( 27 )                                                             \
    f( 28 )                                                             \


#define dot_suffix( x ) x,
typedef enum user_counter_t{
    COUNTER_ALL_MAP( dot_suffix ) NUM_COUNTERS
} user_counter_t;

class CounterReader{
private:
    static uint64_t read_cycle(){
        uint64_t ret;
        asm volatile (
            "csrr %0, cycle"
            : "=r"(ret)
            :
            :"memory"
        );
        return ret;
    }

    static uint64_t read_time(){
        uint64_t ret;
        asm volatile (
            "csrr %0, time"
            : "=r"(ret)
            :
            :"memory"
        );
        return ret;
    }

    static uint64_t read_instret(){
        uint64_t ret;
        asm volatile (
            "csrr %0, instret"
            : "=r"(ret)
            :
            :"memory"
        );
        return ret;
    }
    #define DEFINE_READ_HPMCOUNTER( n )                                 \
        static uint64_t read_hpmcounter##n(){                           \
            uint64_t ret;                                               \
            asm volatile (                                              \
                "csrr %0, hpmcounter" # n                               \
                "\n"                                                    \
                : "=r"(ret)                                             \
                :                                                       \
                :"memory"                                               \
            );                                                          \
            return ret;                                                 \
        }

    COUNTER_NUMBERS_MAP( DEFINE_READ_HPMCOUNTER )

public:
    static const vector<user_counter_t> basic_idx;
    static const vector<user_counter_t> cache_idx;
    static const vector<user_counter_t> tlb_idx;
    static const vector<user_counter_t> branch_idx;
    static const vector<user_counter_t> lsu_idx;
    static const vector<user_counter_t> rf_idx;
    static const vector<user_counter_t> all_idx;
    static const vector<string> counter_name;

    static uint64_t read_counter( user_counter_t idx ){
        uint64_t ret;

        #define CASE_STATE( x )                                         \
            case x : ret = read_hpmcounter##x(); break;

        switch( idx ){
            case CYCLE: ret = read_cycle(); break;
            case TIME: ret = read_time(); break;
            case INSTRET: ret = read_instret(); break;
            COUNTER_NUMBERS_MAP(CASE_STATE)
            default: printf("[error]: unknown counter\n"); break;
        }
        return ret;
    }
};

class CounterRecoder{
public:
    vector<uint64_t> recoder = vector<uint64_t>(NUM_COUNTERS);

    void record_basic(){
        for( auto i: CounterReader::basic_idx ){
            recoder[i] = CounterReader::read_counter(i);
        }
    }
    void record_cache(){
        for( auto i: CounterReader::cache_idx ){
            recoder[i] = CounterReader::read_counter(i);
        }
    }
    void record_tlb(){
        for( auto i: CounterReader::tlb_idx ){
            recoder[i] = CounterReader::read_counter(i);
        }
    }
    void record_branch(){
        for( auto i: CounterReader::branch_idx ){
            recoder[i] = CounterReader::read_counter(i);
        }
    }
    void record_lsu(){
        for( auto i: CounterReader::lsu_idx ){
            recoder[i] = CounterReader::read_counter(i);
        }
    }
    void record_rf(){
        for( auto i: CounterReader::rf_idx ){
            recoder[i] = CounterReader::read_counter(i);
        }
    }
    void record_all(){
        record_basic();
        record_cache();
        record_tlb();
        record_branch();
        record_lsu();
        record_rf();
    }

};

class CounterUtils{
private:

    CounterRecoder start_recorder, end_recorder;
    inline uint64_t end_minus_start( user_counter_t idx ){
        return end_recorder.recoder[idx] - start_recorder.recoder[idx];
    }
public:
    void start_record_basic();
    void start_record_cache();
    void start_record_tlb();
    void start_record_branch();
    void start_record_lsu();
    void start_record_rf();
    void start_all();

    void end_record_basic();
    void end_record_cache();
    void end_record_tlb();
    void end_record_branch();
    void end_record_lsu();
    void end_record_rf();
    void end_all();

    void print_record_basic();
    void print_record_cache();
    void print_record_tlb();
    void print_record_branch();
    void print_record_lsu();
    void print_record_rf();
    void print_all();
};
#else

typedef enum user_counter_t{
    NUM_COUNTERS
} user_counter_t;

class CounterReader{
public:
    static uint64_t read_counter( user_counter_t idx ){
        return 0;
    }
};

class CounterRecoder{
public:
    void record_basic(){}
    void record_cache(){}
    void record_tlb(){}
    void record_branch(){}
    void record_lsu(){}
    void record_rf(){}
    void record_all(){}
};

class CounterUtils{
private:
    inline uint64_t end_minus_start( user_counter_t idx ){
        return 0;
    }
public:
    void start_record_basic();
    void start_record_cache();
    void start_record_tlb();
    void start_record_branch();
    void start_record_lsu();
    void start_record_rf();
    void start_all();

    void end_record_basic();
    void end_record_cache();
    void end_record_tlb();
    void end_record_branch();
    void end_record_lsu();
    void end_record_rf();
    void end_all();

    void print_record_basic();
    void print_record_cache();
    void print_record_tlb();
    void print_record_branch();
    void print_record_lsu();
    void print_record_rf();
    void print_all();
};

#endif
