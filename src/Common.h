/*
* Copyright (c) by CryptoLab inc.
* This program is licensed under a
* Creative Commons Attribution-NonCommercial 3.0 Unported License.
* You should have received a copy of the license along with this
* work.  If not, see <http://creativecommons.org/licenses/by-nc/3.0/>.
*/

#ifndef HEAANNTT_COMMON_H_
#define HEAANNTT_COMMON_H_

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <stdexcept>
#include <vector>
#include <sys/time.h>
#include <string>
#include <math.h>
# define M_PI		3.14159265358979323846	/* pi */

#include "../config.h"

#ifdef CONFIG_RVV
#include <riscv_vector.h>
#endif

#ifdef CONFIG_FHE_EXT
#include "FHEUtil.h"
#endif

void *big_page_alloc(size_t size);

#endif
