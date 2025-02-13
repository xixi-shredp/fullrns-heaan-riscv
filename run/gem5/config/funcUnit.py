from m5.objects.FuncUnit import *
from m5.objects import *
from m5.params import *
from m5.SimObject import SimObject

class BJU(FUDesc):
    opList = [
        OpDesc(opClass="Branch"),
    ]
    count = 1

class IntALUMult(FUDesc):
    opList = [
        OpDesc(opClass="IntAlu"),
        OpDesc(opClass="IntMult", opLat=3)
    ]
    count = 1

class IntALUDiv(FUDesc):
    opList = [
        OpDesc(opClass="IntAlu"),
        OpDesc(opClass="IntDiv", opLat=20, pipelined=False),
    ]
    count = 1

class IntMod(FUDesc):
    opList = [
        OpDesc(opClass="FHEMod"),
        OpDesc(opClass="FHEAddMod", opLat=2),
        OpDesc(opClass="FHEMontMulMod", opLat=8),
        OpDesc(opClass="FHEBarMulMod", opLat=6),
    ]
    count = 1

class VFPU(FUDesc):
    opList = [
        # float
        OpDesc(opClass="FloatAdd", opLat=2),
        OpDesc(opClass="FloatCmp", opLat=2),
        OpDesc(opClass="FloatCvt", opLat=2),
        OpDesc(opClass="FloatMult", opLat=4),
        OpDesc(opClass="FloatMultAcc", opLat=5),
        OpDesc(opClass="FloatMisc", opLat=3),
        OpDesc(opClass="FloatDiv", opLat=12, pipelined=False),
        OpDesc(opClass="FloatSqrt", opLat=24, pipelined=False),
        # Simd FHE 
        OpDesc(opClass="SimdFHEMod", opLat=5),
        OpDesc(opClass="SimdFHEMontMulMod", opLat=11),
        OpDesc(opClass="SimdFHEBarMulMod", opLat=9),
        # Simd
        OpDesc(opClass="SimdAdd", opLat=4),
        OpDesc(opClass="SimdAddAcc", opLat=4),
        OpDesc(opClass="SimdAlu", opLat=4),
        OpDesc(opClass="SimdCmp", opLat=4),
        OpDesc(opClass="SimdCvt", opLat=4),
        OpDesc(opClass="SimdMisc", opLat=4),
        OpDesc(opClass="SimdMult", opLat=4),
        OpDesc(opClass="SimdMultAcc", opLat=4),
        OpDesc(opClass="SimdMatMultAcc", opLat=4),
        OpDesc(opClass="SimdShift", opLat=4),
        OpDesc(opClass="SimdShiftAcc", opLat=4),
        OpDesc(opClass="SimdDiv", opLat=25),
        OpDesc(opClass="SimdSqrt", opLat=25),
        OpDesc(opClass="SimdFloatAdd", opLat=4),
        OpDesc(opClass="SimdFloatAlu", opLat=4),
        OpDesc(opClass="SimdFloatCmp", opLat=4),
        OpDesc(opClass="SimdFloatCvt", opLat=4),
        OpDesc(opClass="SimdFloatDiv", opLat=25),
        OpDesc(opClass="SimdFloatMisc", opLat=4),
        OpDesc(opClass="SimdFloatMult", opLat=5),
        OpDesc(opClass="SimdFloatMultAcc", opLat=5),
        OpDesc(opClass="SimdFloatMatMultAcc", opLat=5),
        OpDesc(opClass="SimdFloatSqrt", opLat=25),
        OpDesc(opClass="SimdReduceAdd", opLat=4),
        OpDesc(opClass="SimdReduceAlu", opLat=4),
        OpDesc(opClass="SimdReduceCmp", opLat=4),
        OpDesc(opClass="SimdFloatReduceAdd", opLat=4),
        OpDesc(opClass="SimdFloatReduceCmp", opLat=4),
        OpDesc(opClass="SimdExt", opLat=4),
        OpDesc(opClass="SimdFloatExt", opLat=4),
        OpDesc(opClass="SimdConfig"),
    ]
    count = 2

class LoadPipe(FUDesc):
    opList = [
        OpDesc(opClass="MemRead"),
        OpDesc(opClass="FloatMemRead"),
        OpDesc(opClass="SimdUnitStrideLoad"),
        OpDesc(opClass="SimdUnitStrideMaskLoad"),
        OpDesc(opClass="SimdStridedLoad"),
        OpDesc(opClass="SimdIndexedLoad"),
        OpDesc(opClass="SimdUnitStrideFaultOnlyFirstLoad"),
        OpDesc(opClass="SimdWholeRegisterLoad"),
    ]
    count = 1

class StorePipe(FUDesc):
    opList = [
        OpDesc(opClass="MemWrite"),
        OpDesc(opClass="FloatMemWrite"),
        OpDesc(opClass="SimdUnitStrideStore"),
        OpDesc(opClass="SimdUnitStrideMaskStore"),
        OpDesc(opClass="SimdStridedStore"),
        OpDesc(opClass="SimdIndexedStore"),
        OpDesc(opClass="SimdWholeRegisterStore"),
    ]
    count = 2

class C910FUPool(FUPool):
    FUList = [
        LoadPipe(),
        StorePipe(),
        BJU(),
        IntALUMult(),
        IntALUDiv(),
        IntMod(),
        VFPU(),
    ]

