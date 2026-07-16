/**
 * @file      nr_micro_shell_commands.c
 * @author    Nrush
 * @version   V0.1
 * @date      28 Oct 2019
 * *****************************************************************************
 * @attention
 *
 * MIT License
 *
 * Copyright (C) 2019 Nrush. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

 /* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "nr_micro_shell.h"
#include "trace.h"
#include "bsp_usart3.h"
#include "protocol.h"
#include "bsp_burnin_board.h"


extern ITVL_t* gITVL;

/**
 * @brief ls command
 */
static void ls_cmd(char argc, char* argv)
{
    unsigned int i = 0;
    if (argc > 1)
    {
        if (!strcmp("cmd", &argv[argv[1]]))
        {

            for (i = 0; nr_shell.static_cmd[i].fp != NULL; i++)
            {
                INF("%s", nr_shell.static_cmd[i].cmd);
                INF("\n");
            }
        }
        else if (!strcmp("-v", &argv[argv[1]]))
        {
            INF("ls version 1.0.\n");
        }
        else if (!strcmp("-h", &argv[argv[1]]))
        {
            INF("useage: ls [options]\n");
            INF("options: \n");
            INF("\t -h \t: show help\n");
            INF("\t -v \t: show version\n");
            INF("\t cmd \t: show all commands\n");
        }
    }
    else
    {
        INF("ls need more arguments!\n");
    }
}

/**
 * @brief test command
 */
static void test_cmd(char argc, char* argv)
{

    unsigned int i;
    INF("test command:\n");
    for (i = 0; i < argc; i++)
    {
        INF("paras %d: %s\n", i, &(argv[argv[i]]));
    }

#ifdef _SIMULATION_MTK_SOC_

    char* xfer_dir = &argv[argv[1]];
    int  partition = strtol(&argv[argv[2]], NULL, 10);
    uint32_t lba = strtol(&argv[argv[3]], NULL, 10);
    uint32_t len = strtol(&argv[argv[4]], NULL, 10);


    /* request transfer data */
    if (memcmp(xfer_dir, SHELL_XFER_SICO_STR, 4) == 0 && len)
    {
        /* request transfer length */
        WRN("%s %x\r\n", SHELL_XFER_SICO_STR, len);

        /* here clear gDATABUF !!! */
        gITVL->wHead = ITVL_HEAD;
        gITVL->bType = ITVL_HEX_TYPE;
        gITVL->wLength = len;
        gITVL->wID = 0;

        /* last calc crc */
        gITVL->wCheckSum = 0;

        uint16_t* ptr = (uint16_t*)gITVL->value;
        for (i = 0; i < (gITVL->wLength / 2); ++i)
        {
            *ptr++ = i;
        }

        char* ch = (char*)gITVL;
        for (i = 0; i < (sizeof(ITVL_t) + gITVL->wLength); ++i)
        {
            putChar(*ch++);
        }
    }
    else if (memcmp(xfer_dir, SHELL_XFER_SOCI_STR, 4) == 0 && len)
    {
        WRN("%s %x\r\n", SHELL_XFER_SOCI_STR, len);

        while ((gRxParam & RXDONE) == 0);

        /* receive data */
        DUMP("ITVL", gITVL, len);
    }
    else
    {
        WRN("no request data.\r\n");
    }

#endif
}


/**
 * @brief reset slt command
 */
static void reset_slt_cmd(char argc, char* argv)
{
		int bib, site;
	
    bib = strtol(&argv[argv[1]], NULL, 10);
    site = strtol(&argv[argv[2]], NULL, 10);

		reset_soc(bib, site);
}

NR_SHELL_CMD_EXPORT(ls, ls_cmd);
NR_SHELL_CMD_EXPORT(test, test_cmd);
NR_SHELL_CMD_EXPORT(reset_slt, reset_slt_cmd);

/******************* (C) COPYRIGHT 2019 Nrush *****END OF FILE*****************/
