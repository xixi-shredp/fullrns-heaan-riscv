#import "@preview/cheq:0.2.2": checklist
#show: checklist

#import "@preview/grape-suite:1.0.0": exercise
#import exercise: project, task, subtask

#show: project.with(
  title: "Readme",

  show-outline: true,

  show-solutions: false,
)

= src
仓库地址：https://github.com/xixi-shredp/fullrns-heaan-riscv.git
== 使用Barrett单变量模乘
=== 数据预处理
+ 在Context中添加成员变量和成员函数
  ```cpp
    uint64_t **s4ntt_rowBarPres;    // 行NTT预处理数
    uint64_t **s4ntt_colBarPres;    // 列NTT预处理数
    uint64_t **s4ntt_wmatrixBarPres;// 乘系数预处理数
    uint64_t **nttBarPres;          // 直接NTT预处理数

    // Barrett单变量模乘直接NTT
    void origin_qiNTTAndEqual_withBar(uint64_t *a, long index);
    // Montgomeny模乘直接NTT
    void origin_qiNTTAndEqual_withMont(uint64_t *a, long index);
    // Barrett单变量模乘4步NTT
    void step4_qiNTTAndEqual_withBar(uint64_t *a, long index);
    // Montgomeny模乘4步NTT
    void step4_qiNTTAndEqual_withMont(uint64_t *a, long index);
  ```

+ 在Context构造函数中做数据预处理
  ```cpp
    nttBarPres           = cus_alloc(uint64_t *, L);
    s4ntt_wmatrixBarPres = new uint64_t *[L];
    s4ntt_rowBarPres     = cus_alloc(uint64_t *, L);
    if (N1 == N2) {
        s4ntt_colBarPres = s4ntt_rowBarPres;
    } else {
        s4ntt_colBarPres = cus_alloc(uint64_t *, L);
    }

    for (long i = 0; i < L; ++i) {
      ...
      nttBarPres[i] = new uint64_t[N]();
      calBarPre(N, qRootPows[i], nttBarPres[i], qVec[i]);
      s4ntt_colBarPres[i] = new uint64_t[N1];
      calBarPre(N1, s4ntt_col_qRootPows[i], s4ntt_colBarPres[i], qVec[i]);
      if (N1 != N2) {
          s4ntt_rowBarPres[i] = new uint64_t[N2];
          calBarPre(N2, s4ntt_row_qRootPows[i], s4ntt_rowBarPres[i],
                    qVec[i]);
      }
    }
    ...
    /// 乘系数用预处理器计算
    for (int i = 0; i < L; ++i) {
        for (int j = 0; j < N1; ++j) {
            s4ntt_wmatrixBarPres[i] = cus_alloc(uint64_t, N);
            calBarPre(N, wmatrix_pows[i], s4ntt_wmatrixBarPres[i], qVec[i]);
        }
    }
  ```
  - cus_alloc是个宏定义，本质是new，之前为了封装大页分配的时候写的，后来一直没改过来(utils.h)
  - calBarPre用于计算Barrett模乘预处理数（preprocess.h）

=== Barrett模乘直接NTT
在Context.cpp中的`origin_qiNTTAndEqual_withBar`实现。

蝶形运算循环部分：
```cpp
  uint64_t Wori = qRootPows[index][m + i];
  for (long j = j1; j <= j2; j++) {
      uint64_t T = a[j + t];
      uint64_t V = barrett_modMul_singalVal(
          T, Wori, q, nttBarPres[index][m + i]);
      barmul_record;
      a[j + t] = a[j] < V ? a[j] + q - V : a[j] - V;
      a[j] += V;
      if (a[j] > q) a[j] -= q;
      submod_record;
      addmod_record;
  }
```
- `barrett_modMul_singalVal` 为计算函数(utils.h)
- `*_record`用于记录模运算信息，仿真记录数据时需要用到，通过宏关闭记录

== 使用模运算指令扩展
使用函数定义在ExtUtil.h

=== 标量指令扩展直接NTT
在ExtUtil.cpp中的`ext_qiNTTAndEqual_withBar`和`ext_qiNTTAndEqual_withMont`实现。

每次切换模数之前，需要调用setmod设置模数q和qInv

添加时直接修改蝶形运算里面的模乘模加减即可：
```cpp
  uint64_t W = qRootScalePows[index][m + i];
  uint64_t Wori = qRootPows[index][m + i];
  for (long j = j1; j <= j2; j++) {
    uint64_t T = a[j + t];
    uint64_t V = bar_mulmod_s(T, Wori, nttBarPres[index][m + i]);
    a[j + t] = submod(a[j], V);
    a[j] = addmod(a[j], V);
  }
```

=== 标量指令扩展4步NTT
在Ext_Step4_Util.cpp中实现：

```cpp
STEP4NTT_TEMPLATE_IMPLEMENT(ext_step4_qiNTTAndEqual_withBar, ROW_MUL_Barrett,
                            ROW_NTT_Barrett, SET_MOD);

STEP4NTT_TEMPLATE_IMPLEMENT(ext_step4_qiNTTAndEqual_withMont,
                            ROW_MUL_Montgomeny, ROW_NTT_Montgomeny, SET_MOD);
```
- `STEP4NTT_TEMPLATE_IMPLEMENT`是一个4步NTT实现的宏，定义在Step4NTTUtil.h
- `SET_MOD`也是宏，用于在执行NTT之前设置模数

添加时也只需直接修改行NTT里面的模乘模加减和乘系数时的模乘

=== 向量指令扩展直接NTT
在RVV_Ext_NTT.cpp中实现：

```cpp
void Context::rvv_ext_ori_qiNTTAndEqual_withBar(uint64_t *a, long index);
void Context::rvv_ext_ori_qiNTTAndEqual_withMont(uint64_t *a, long index);
```

- 这里借用4步NTT时为行NTT实现的函数来实现直接NTT


=== 向量指令扩展4步NTT
在RVV_Ext_NTT.cpp中实现：

```cpp
STEP4NTT_TEMPLATE_IMPLEMENT(rvv_ext_step4_qiNTTAndEqual_withBar,
                            RVV_ROW_MUL_Barrett, RVV_EXT_ROW_NTT_Barrett,
                            SET_MOD);
STEP4NTT_TEMPLATE_IMPLEMENT(rvv_ext_step4_qiNTTAndEqual_withMont,
                            RVV_ROW_MUL_Montgomeny, RVV_EXT_ROW_NTT_Montgomeny,
                            SET_MOD);
```
- `STEP4NTT_TEMPLATE_IMPLEMENT`是一个4步NTT实现的宏，定义在Step4NTTUtil.h
- `SET_MOD`也是宏，用于在执行NTT之前设置模数

行NTT优化(rvv_ext_rowNTT_withBar)时，根据蝶形运算组数来分别优化：
```cpp
if (t >= 8) {
    rvv_ext_mth_op1_rowNTTWithBar(m, a, t, logt1, q, qRootPows,
} else {
    rvv_ext_mth_op2_rowNTTWithBar(m, a, t, logt1, q, qRootPows,
}
```

我在实现只实现了LMUL=4的情况，其他情况的实现类似：

使用向量间运算的指令：(此时蝶形因子和预处理数都是向量)
```cpp
  vbarmodmuls_vv(v12, v4, v8);
  vsubmod_vv(v16, v24, v12);
  vaddmod_vv(v24, v24, v12);
```

使用向量标量运算的指令：(此时蝶形因子和预处理数都是标量)
```cpp
  vbarmodmuls_vxx(v4, W, R);
  vsubmod_vv(v8, v16, v4);
  vaddmod_vv(v16, v16, v4);
```
- Barrett向量标量模乘比较特殊，第二和第三操作数都是标量
- Montgomeny 模乘只有2个操作数，就是一般的RVV运算格式

== 仿真时记录运算和数据
在OpRecorder.h中实现，主要用于记录数据来驱动VCS仿真
通过宏CONFIG_OP_RECORD开关

= scripts

- regress.py 用于回归测试
- 其他 py 文件用于解析stdout和gem5的stats.txt

= run/gem5

- config下是配置脚本
- split_stat.py用于把stats.txt分割成多个
  - 因为代码里面使用到了m5ops.h的m5_dump_reset_stats，将统计信息分成了3个部分，第二部分是测试用例部分

= Gem5源码修改
仓库地址：https://github.com/xixi-shredp/xixi-gem5.git

主要是src/arch/riscv/isa
+ 添加指令在decoder.isa
+ 添加一些format:format/fhe.isa
  - FHEOp：本质就是ROp，标量扩展用
  - VectorVXXFormatFHEOp：
    - 为了使用vbarmulmod.vxx添加
    - 在VectorIntFormat的基础上修改了操作数：
      - src1和src2固定为标量操作数
      - src3固定为向量操作数
