from m5.objects.FuncUnit import *
from m5.objects import *
from m5.params import *
from m5.SimObject import SimObject

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

class FHEMod(FUDesc):
    opList = [
        OpDesc(opClass="FHEAddMod", opLat=2),
        OpDesc(opClass="FHEMulMod", opLat=8),
    ]
    count = 1


class FP_ALU(FUDesc):
    opList = [
        OpDesc(opClass="FloatAdd", opLat=2),
        OpDesc(opClass="FloatCmp", opLat=2),
        OpDesc(opClass="FloatCvt", opLat=2),
    ]
    count = 1


class FP_MultDiv(FUDesc):
    opList = [
        OpDesc(opClass="FloatMult", opLat=4),
        OpDesc(opClass="FloatMultAcc", opLat=5),
        OpDesc(opClass="FloatMisc", opLat=3),
        OpDesc(opClass="FloatDiv", opLat=12, pipelined=False),
        OpDesc(opClass="FloatSqrt", opLat=24, pipelined=False),
    ]
    count = 1


class SIMD_Unit(FUDesc):
    opList = [
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


# class PredALU(FUDesc):
#     opList = [OpDesc(opClass="SimdPredAlu")]
#     count = 1


class ReadPort(FUDesc):
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

class WritePort(FUDesc):
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

# class IprPort(FUDesc):
#     opList = [OpDesc(opClass="IprAccess", opLat=3, pipelined=False)]
#     count = 1

class C910FUPool(FUPool):
    FUList = [
        IntALUMult(),
        IntALUDiv(),
        FHEMod(),
        FP_ALU(),
        FP_MultDiv(),
        ReadPort(),
        SIMD_Unit(),
        WritePort(),
    ]

