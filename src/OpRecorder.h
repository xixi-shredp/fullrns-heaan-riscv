#ifndef __OP_RECORDER_HH__
#define __OP_RECORDER_HH__

#include <cassert>
#include <cstdint>
#include <iostream>

typedef uint64_t OpEleType;

#ifdef CONFIG_OP_RECORD
    #define __Op2FuncDefine(name)                                           \
        void name(OpEleType op1, OpEleType op2, OpEleType q)                \
        {                                                                   \
            fprintf(fp, #name " %lu %lu %lu\n", op1, op2, q);               \
        }                                                                   \
        void name(OpEleType op1, OpEleType op2, OpEleType q, OpEleType res) \
        {                                                                   \
            fprintf(fp, #name " %lu %lu %lu =%lu\n", op1, op2, q, res);     \
        }

    #define __Op4FuncDefine(name)                                         \
        void name(OpEleType op1, OpEleType op2, OpEleType q, OpEleType r) \
        {                                                                 \
            fprintf(fp, #name " %lu %lu %lu %lu\n", op1, op2, q, r);      \
        }                                                                 \
        void name(OpEleType op1, OpEleType op2, OpEleType q, OpEleType r, \
                  OpEleType res)                                          \
        {                                                                 \
            fprintf(fp, #name " %lu %lu %lu %lu =%lu\n", op1, op2, q, r,  \
                    res);                                                 \
        }
#else
    #define __Op2FuncDefine(name)                               \
        void name(OpEleType op1, OpEleType op2, OpEleType q) {} \
        void name(OpEleType op1, OpEleType op2, OpEleType q, OpEleType res) {}

    #define __Op4FuncDefine(name)                                            \
        void name(OpEleType op1, OpEleType op2, OpEleType q, OpEleType r) {} \
        void name(OpEleType op1, OpEleType op2, OpEleType q, OpEleType r,    \
                  OpEleType res)                                             \
        {                                                                    \
        }
#endif

class OpRecorder
{
  private:
    FILE *fp;

  public:
    OpRecorder(const char *path) : fp(nullptr)
    {
#ifdef CONFIG_OP_RECORD
        fp = fopen(path, "w");
        assert(fp);
        if (fp == nullptr) {
            fprintf(stderr, "file `%s` open failed for OpRecorder\n", path);
            exit(1);
        }
#endif
    }
    ~OpRecorder()
    {
        if (fp != nullptr) fclose(fp);
    }
    __Op2FuncDefine(rd_addmod);
    __Op2FuncDefine(rd_submod);
    __Op4FuncDefine(rd_montredc);
    __Op4FuncDefine(rd_barmulmod)
};

#endif // __OP_RECORDER_HH__
