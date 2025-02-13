from m5.objects.ReplacementPolicies import FIFORP
from funcUnit import C910FUPool
from m5.objects import *


def c910CPUConfig(cpu):
    # cpu.forwardComSize = 5
    cpu.backComSize = 7

    ### ============== IFU ===================
    # cpu.fetchTrapLatency = 1 # Seems Unused
    cpu.fetchWidth = 8
    cpu.fetchBufferSize = 16  # (128 bits) 16 Bytes: to buffer the cache line data
    cpu.fetchQueueSize = 2 * 8 + 3
    cpu.fetchToDecodeDelay = 3
    # forward delay should still be 1

    ### ============== IDU ===================
    cpu.decodeWidth = 3
    cpu.renameWidth = 4
    cpu.dispatchWidth = 8
    cpu.issueWidth = 8

    cpu.numIQEntries = 8  # Instruction slots

    cpu.commitWidth = 3
    cpu.wbWidth = 3

    cpu.fuPool = C910FUPool()

    cpu.numPhysIntRegs = 96
    cpu.numPhysFloatRegs = 64
    cpu.numPhysVecRegs = 64
    cpu.numPhysVecPredRegs = 0  # RVV Use VecReg as VecPredReg
    cpu.numPhysMatRegs = 0
    cpu.numROBEntries = 192
    # cpu.squashWidth = 8 # Use for ROB

    # cpu.trapLatency = 13

    ### ============== LSU ===================
    cpu.LQEntries = 16
    cpu.SQEntries = 12
    # cpu.LSQCheckLoads = ?
    # cpu.LSQDepCheckShift = ?
    # cpu.LFSTSize  = ?
    # cpu.SSITSIze  = ?
    # cpu.store_set_clear_period = ?
    cpu.cacheLoadPorts = 1  # the number of cache ports for LSQ Unit to access DCache
    cpu.cacheStorePorts = 1  # maybe only meaningful for non-blocking dcache

    ### ============== BPU ===================
    bpu = cpu.branchPred
    bpu.instShiftAmt = 3
    bpu.globalPredictorSize = 16384
    bpu.globalCtrBits = 2
    bpu.choicePredictorSize = 1024
    bpu.choiceCtrBits = 2
    # FIXME: c910 has also LoopBuffer, which need to be configured.
    # WARN: c910 use 2-level btb, but only SimpleBTB is acquired now.
    # Also the tagBits and instShiftAmt is not known.
    bpu.btb.numEntries = 1024
    bpu.ras.numEntries = 12
    bpu.indirectBranchPred.indirectSets = 256
    bpu.indirectBranchPred.indirectWays = 1
    # WARN: no enough message for IndirectBranchPred in C910.
    # indirectBranchPred:

    ### ============== CACHE ===================
    icache = cpu.icache
    icache.replacement_policy = FIFORP()
    icache.mshrs = 1
    icache.tgts_per_mshr = 1  # targets per mshr
    icache.tag_latency = 1
    icache.data_latency = 1

    dcache = cpu.dcache
    dcache.replacement_policy = FIFORP()
    dcache.mshrs = 8  # non-blocking Read Buffer in C910
    # dcache.tgts_per_mshr = ??? # targets per mshr
    dcache.tag_latency = 1
    dcache.data_latency = 1
    dcache.write_buffers = 8  # wmb in C910

    # In fact, TLB will not be used in SE Mode.
    ### ============== MMU ===================
    # mmu = cpu.mmu
    # mmu.itb.size = 32
    # mmu.dtb.size = 17
    # jtlb = RiscvTLB(size=1024, entry_type="unified")
    # mmu.itb.next_level = jtlb
    # mmu.dtb.next_level = jtlb
    # mmu.pmp.pmp_entries = 8
