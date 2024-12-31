def override_args(args):
    '''
    used to substitude c910.mk
    should be used after argparser
    '''
    args.num_cpus = 4
    args.sys_voltage = 0.8
    args.sys_clock = "2GHz"
    args.cpu_clock = "2GHz"
    args.cpu_type = "RiscvO3CPU"

    args.bp_type = "BiModeBP"

    args.caches = True
    args.cacheline_size = 64

    args.l1i_size = str(65536)
    args.l1i_assoc = 2
    args.l1i_hwp_type = "StridePrefetcher"

    args.l1d_size = str(65536)
    args.l1d_assoc = 2
    args.l1d_hwp_type = "StridePrefetcher"

    args.l2cache = True
    args.l2_size = "1MB"
    args.l2_assoc = 16
    args.l2_hwp_type = "StridePrefetcher"
    args.num_lacaches = 1

    args.mem_type = "DDR5_8400_4x8"
    args.mem_size = "7715MB"
