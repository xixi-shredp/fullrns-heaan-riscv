#include "CounterUtils.h"
#include <inttypes.h>

#ifdef CONFIG_XUANTIE_RISCV
#define BASIC_IDXES \
    CYCLE, TIME, INSTRET

#define CACHE_IDXES \
    L1I_ACCESS, L1I_MISS,\
    L1D_READ_ACCESS, L1D_READ_MISS, L1D_WRITE_ACCESS, L1D_WRITE_MISS, \
    L2_READ_ACCESS, L2_READ_MISS, L2_WRITE_ACCESS, L2_WRITE_MISS 

#define TLB_IDXES \
    I_UTLB_MISS, D_UTLB_MISS, JTLB_MISS 

#define BRANCH_IDXES \
    COND_BRANCH_MISS, IND_BRANCH_MISS, IND_BRANCH_INS

#define LSU_IDXES \
    LSU_SPEC_FAIL, STORE_INS, LSU_CRS_4K_STALL, \
    LSU_OTHER_STALL, LSU_SQ_DISC, LSU_SQ_DATA_DISC

#define RF_IDXES \
    RF_LNCH_FAIL, RF_REG_LNCH_FAIL, RF_INST

const vector<user_counter_t> CounterReader::basic_idx = { BASIC_IDXES };

const vector<user_counter_t> CounterReader::cache_idx = { CACHE_IDXES };

const vector<user_counter_t> CounterReader::tlb_idx = { TLB_IDXES };

const vector<user_counter_t> CounterReader::branch_idx = { BRANCH_IDXES };

const vector<user_counter_t> CounterReader::lsu_idx = { LSU_IDXES };

const vector<user_counter_t> CounterReader::rf_idx = { RF_IDXES };

const vector<user_counter_t> CounterReader::all_idx = {
    BASIC_IDXES,
    CACHE_IDXES,
    TLB_IDXES,
    BRANCH_IDXES,
    LSU_IDXES,
    RF_IDXES
};

const vector<string> CounterReader::counter_name={
    "cycle", "time", "instret",
    "l1 icache access", "l1 icache miss",
    "i-utlb miss", "d-utlb miss", "jtlb miss",
    "cond branch miss", "reserved", "ind branch miss", "ind branch",
    "lsu spec fail", "store inst", 
    "l1 dcache read access", "l1 dcache read miss",
    "l1 dcache write access", "l1 dcache write miss",
    "l2 cache read access", "l2 cache read miss",
    "l2 cache write access", "l2 cache write miss",
    "rf launch fail", "rf reg launch fail", "rf inst",
    "lsu cross 4k stall", "lsu other stall", 
    "lsu sq discard", "lsu sq data discard"
};

void CounterUtils::start_record_basic(){
    start_recorder.record_basic();
}
void CounterUtils::start_record_cache(){
    start_recorder.record_basic();
    start_recorder.record_cache();
}
void CounterUtils::start_record_tlb(){
    start_recorder.record_basic();
    start_recorder.record_tlb();
}
void CounterUtils::start_record_branch(){
    start_recorder.record_basic();
    start_recorder.record_branch();
}
void CounterUtils::start_record_lsu(){
    start_recorder.record_basic();
    start_recorder.record_lsu();
}
void CounterUtils::start_record_rf(){
    start_recorder.record_basic();
    start_recorder.record_rf();
}
void CounterUtils::start_all(){
    start_recorder.record_all();
}


void CounterUtils::end_record_basic(){
    end_recorder.record_basic();
}

void CounterUtils::end_record_cache(){
    end_recorder.record_cache();
}

void CounterUtils::end_record_tlb(){
    end_recorder.record_tlb();
}

void CounterUtils::end_record_branch(){
    end_recorder.record_branch();
}

void CounterUtils::end_record_lsu(){
    end_recorder.record_lsu();
}

void CounterUtils::end_record_rf(){
    end_recorder.record_rf();
}

void CounterUtils::end_all(){
    end_recorder.record_all();
}

void CounterUtils::print_record_basic(){
    for( auto i: CounterReader::basic_idx ){
        printf("%s: %" PRIu64 "\n", CounterReader::counter_name[i].c_str(), end_minus_start(i) );
    }
}

void CounterUtils::print_record_cache(){
    for( auto i: CounterReader::cache_idx ){
        printf("%s: %" PRIu64 "\n", CounterReader::counter_name[i].c_str(), end_minus_start(i) );
    }
    printf("l1 icache miss rate: %lf%%\n", 100*(double)end_minus_start(L1I_MISS)/end_minus_start(L1I_ACCESS) );
    printf("l1 dcache read miss rate: %lf%%\n", 100*(double)end_minus_start(L1D_READ_MISS)/end_minus_start(L1D_READ_ACCESS) );
    printf("l1 dcache write miss rate: %lf%%\n", 100*(double)end_minus_start(L1D_WRITE_MISS)/end_minus_start(L1D_WRITE_ACCESS) );
    printf("l2 cache read miss rate: %lf%%\n", 100*(double)end_minus_start(L2_READ_MISS)/end_minus_start(L2_READ_ACCESS) );
    printf("l2 cache write miss rate: %lf%%\n", 100*(double)end_minus_start(L2_WRITE_MISS)/end_minus_start(L2_WRITE_ACCESS) );
}

void CounterUtils::print_record_tlb(){
    for( auto i: CounterReader::tlb_idx ){
        printf("%s: %" PRIu64 "\n", CounterReader::counter_name[i].c_str(), end_minus_start(i) );
    }
    printf("i-utlb miss rate: %lf%%\n", 100*(double)end_minus_start(I_UTLB_MISS)/end_minus_start(L1I_ACCESS) );
    printf("d-utlb miss rate: %lf%%\n", 100*(double)end_minus_start(D_UTLB_MISS)/(end_minus_start(L1D_READ_ACCESS) + end_minus_start(L1D_WRITE_ACCESS)) );
    printf("jtlb miss rate: %lf%%\n", 100*(double)end_minus_start(JTLB_MISS)/( end_minus_start(I_UTLB_MISS) + end_minus_start(D_UTLB_MISS) ) );
}

void CounterUtils::print_record_branch(){
    for( auto i: CounterReader::branch_idx ){
        printf("%s: %" PRIu64 "\n", CounterReader::counter_name[i].c_str(), end_minus_start(i) );
    }
}

void CounterUtils::print_record_lsu(){
    for( auto i: CounterReader::lsu_idx ){
        printf("%s: %" PRIu64 "\n", CounterReader::counter_name[i].c_str(), end_minus_start(i) );
    }
}

void CounterUtils::print_record_rf(){
    for( auto i: CounterReader::rf_idx ){
        printf("%s: %" PRIu64 "\n", CounterReader::counter_name[i].c_str(), end_minus_start(i) );
    }
}

void CounterUtils::print_all(){
    print_record_basic();
    print_record_cache();
    print_record_tlb();
    print_record_branch();
    print_record_lsu();
    print_record_rf();
}

#else

void CounterUtils::start_record_basic(){}
void CounterUtils::start_record_cache(){}
void CounterUtils::start_record_tlb(){}
void CounterUtils::start_record_branch(){}
void CounterUtils::start_record_lsu(){}
void CounterUtils::start_record_rf(){}
void CounterUtils::start_all(){}
void CounterUtils::end_record_basic(){}
void CounterUtils::end_record_cache(){}
void CounterUtils::end_record_tlb(){}
void CounterUtils::end_record_branch(){}
void CounterUtils::end_record_lsu(){}
void CounterUtils::end_record_rf(){}
void CounterUtils::end_all(){}
void CounterUtils::print_record_basic(){}
void CounterUtils::print_record_cache(){}
void CounterUtils::print_record_tlb(){}
void CounterUtils::print_record_branch(){}
void CounterUtils::print_record_lsu(){}
void CounterUtils::print_record_rf(){}
void CounterUtils::print_all(){}
#endif
