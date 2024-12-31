SYS_CLOCK   = 2GHz
CPU_CLOCK   = 2GHz
SYS_VOLATGE = 0.8


L1ICACHE_FLAGS = --l1i_size 65536 \
								 --l1i_assoc 2 \
								 --l1i-hwp-type StridePrefetcher

L1DCACHE_FLAGS = --l1d_size 65536 \
								 --l1d_assoc 2 \
								 --l1d-hwp-type StridePrefetcher

L2CACHE_FLAGS = --l2cache \
								--l2_size 1MB \
								--l2_assoc 16 \
								--l2-hwp-type StridePrefetcher \
								--num-l2caches 1

CACHE_FLAGS = --caches \
							$(L1ICACHE_FLAGS) \
							$(L1DCACHE_FLAGS) \
							$(L2CACHE_FLAGS) \
							--cacheline_size 64

BP_FLAGS = --bp-type C910BiModeBP

MEM_FLAGS = --mem-type DDR5_8400_4x8 \
						--mem-size 7715MB

SE_FLAGS += -n 4 --sys-voltage 0.8 \
						--sys-clock $(SYS_CLOCK) \
						--cpu-clock $(CPU_CLOCK) \
						--cpu-type RiscvO3CPU \
						$(BP_FLAGS) $(CACHE_FLAGS) $(MEM_FLAGS)
