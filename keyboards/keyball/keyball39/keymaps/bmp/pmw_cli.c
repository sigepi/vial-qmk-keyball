// Copyright 2026 sigepi
/* SPDX-License-Identifier: GPL-2.0-or-later */

// CLI command "pmw": show PMW3360 state (product, SROM, CPI register).
// "pmw <n>" writes n (0..119 = 100..12000 CPI) to Config1 and reads it back,
// without touching the keyball CPI setting (test only; a CPI key or reset restores it).

#include <stdlib.h>
#include "quantum.h"
#include "printf.h"
#include "lib/keyball/keyball.h"
#include "drivers/pmw3360/bmp_pmw3360.h"
#include "microshell/util/msopt.h"
#include "microshell/util/mscmd.h"

static void print_cpi_reg(void) {
    uint8_t c1 = pmw3360_reg_read(pmw3360_Config1);
    printf("config1: %u (= %u cpi)\n", c1, (unsigned)(c1 + 1) * 100);
}

MSCMD_USER_RESULT usrcmd_pmw(MSOPT *msopt, MSCMD_USER_OBJECT usrobj) {
    if (!keyball.this_have_ball) {
        printf("no sensor on this half\n");
        return 1;
    }

    if (msopt->argc >= 2) {
        char arg[8];
        msopt_get_argv(msopt, 1, arg, sizeof(arg));
        int n = atoi(arg);
        if (n < 0 || n > pmw3360_MAXCPI) {
            printf("pmw <0..%u>\n", pmw3360_MAXCPI);
            return 1;
        }
        printf("write config1: %d (= %d cpi)\n", n, (n + 1) * 100);
        pmw3360_reg_write(pmw3360_Config1, (uint8_t)n);
        print_cpi_reg();
        return 0;
    }

    uint8_t pid = pmw3360_reg_read(pmw3360_Product_ID);
    uint8_t rev = pmw3360_reg_read(pmw3360_Revision_ID);
    uint8_t sid = pmw3360_reg_read(pmw3360_SROM_ID);
    printf("product: 0x%02x rev: 0x%02x\n", pid, rev);
    printf("srom_id reg: 0x%02x, uploaded: 0x%02x (0 = no upload)\n", sid, pmw3360_srom_id);
    print_cpi_reg();
    printf("keyball cpi: %u (= %u cpi)\n", keyball_get_cpi(), (unsigned)keyball_get_cpi() * 100);
    return 0;
}
