/**
* @file bib_map.h
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief
*
* @details define burnin board site map
*
* @date 2022/11/2
*
* @author Jeff.he
*
* @bug
*
* Revisions: MT6757_LPDDR4_MB v01
*
*/

#ifndef __BURNIN_BOARD_MAP_H
#define __BURNIN_BOARD_MAP_H

/* define AT24C64 address */
#define MB_AT24C64_ADDR     (0xA2)

/* define pca9555 address */
#define PCA9555_0_ADDR      (0x40)
#define PCA9555_1_ADDR      (0x42)
#define PCA9555_2_ADDR      (0x44)
#define PCA9555_3_ADDR      (0x4A)
#define PCA9555_4_ADDR      (0x4C)

/* define pca9555 command byte(register) */
#define REG_PORTIN          (0)
#define REG_PORTOUT         (2)
#define REG_PORTPI          (4)
#define REG_PORTCONF        (6)

/* 1b: input; 0b: output */
#define PINOUT              (0)
#define PININ               (7)

/* value */
#define INH                 (9)

/* slave address + command byte + port0 + port1 */
#define CTRL(addr, reg, io_0_3, io_4_7, io_8_11, io_12_15)   (((uint32_t)((io_12_15) & 15) << 28) | \
                                                             ((uint32_t)((io_8_11) & 15) << 24) | \
                                                             ((uint32_t)((io_4_7) & 15) << 20) | \
                                                             ((uint32_t)((io_0_3) & 15) << 16) | \
                                                             ((uint32_t)((reg) & 255) << 8) | \
                                                             ((uint32_t)((addr) & 255) << 0))
																														 


/* cd4051 truth table           */
/* i c b a    bcd channel    cs */
/* 0 0 0 0    0   0             */
/* 0 0 0 1    1   1             */
/* 0 0 1 0    2   2             */
/* 0 0 1 1    3   3             */
/* 0 1 0 0    4   4             */
/* 0 1 0 1    5   5             */
/* 0 1 1 0    6   6             */
/* 0 1 1 1    7   7             */
/* 1 x x x    -   -             */


/* config directions of I/O pins. 16bit : 1b input, 0b output. */
#define PCA9555_0_IO        CTRL(PCA9555_0_ADDR, REG_PORTCONF, PINOUT, PINOUT, PINOUT, PINOUT)
#define PCA9555_1_IO        CTRL(PCA9555_1_ADDR, REG_PORTCONF, PINOUT, PINOUT, PINOUT, PINOUT)
#define PCA9555_2_IO        CTRL(PCA9555_2_ADDR, REG_PORTCONF, PINOUT, PINOUT, PINOUT, PINOUT)
#define PCA9555_3_IO        CTRL(PCA9555_3_ADDR, REG_PORTCONF, PINOUT, PINOUT, PINOUT, PINOUT)
#define PCA9555_4_IO        CTRL(PCA9555_4_ADDR, REG_PORTCONF, PINOUT, PINOUT, PINOUT, PINOUT)


/* define cs from bib schematic diagram */
#define TL_BURNIN_BOARDS    (4)
#define BIB_SKTS            (75)


#define DISABLE_SITE_MARK		CTRL(	            0, 		       0, INH, INH, INH, INH)

/* disable all pca9555 */
#define DIS_PCA9555_0			  CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH, INH, INH)
#define DIS_PCA9555_1			  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH, INH, INH)
#define DIS_PCA9555_2			  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH, INH, INH)
#define DIS_PCA9555_3			  CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH, INH, INH)
#define DIS_PCA9555_4			  CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH, INH, INH)


/* CS & SYSRSB mapping */

/* inh11 a11 b11 c11 bcd ch site */
/*  0    0   0   0   0   0  18   */
/*  0    1   0   0   4   1  17   */
/*  0    0   1   0   2   2  16   */
/*  0    1   1   0   6   3  19   */
/*  0    0   0   1   1   4  1    */
/*  0    1   0   1   5   5  4    */
/*  0    0   1   1   3   6  2    */
/*  0    1   1   1   7   7  3    */
/*  1    x   x   x   -   -  -    */
#define RST18                CTRL(PCA9555_0_ADDR, REG_PORTOUT, 0, INH, INH, INH)
#define RST17                CTRL(PCA9555_0_ADDR, REG_PORTOUT, 4, INH, INH, INH)
#define RST16                CTRL(PCA9555_0_ADDR, REG_PORTOUT, 2, INH, INH, INH)
#define RST19                CTRL(PCA9555_0_ADDR, REG_PORTOUT, 6, INH, INH, INH)
#define RST1                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, 1, INH, INH, INH)
#define RST4                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, 5, INH, INH, INH)
#define RST2                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, 3, INH, INH, INH)
#define RST3                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, 7, INH, INH, INH)


/* inh12 a12 b12 c12 bcd ch site */
/*  0    0   0   0   0   0  22   */
/*  0    1   0   0   4   1  21   */
/*  0    0   1   0   2   2  20   */
/*  0    1   1   0   6   3  23   */
/*  0    0   0   1   1   4  5    */
/*  0    1   0   1   5   5  8    */
/*  0    0   1   1   3   6  6    */
/*  0    1   1   1   7   7  7    */
/*  1    x   x   x   -   -  -    */
#define RST22                CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, 0, INH, INH)
#define RST21                CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, 4, INH, INH)
#define RST20                CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, 2, INH, INH)
#define RST23                CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, 6, INH, INH)
#define RST5                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, 1, INH, INH)
#define RST8                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, 5, INH, INH)
#define RST6                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, 3, INH, INH)
#define RST7                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, 7, INH, INH)


/* c2 b2 a2 inh2 bcd  ch site */
/* 0  0  0   0    0   0  22   */
/* 0  0  1   0    2   1  21   */
/* 0  1  0   0    4   2  20   */
/* 0  1  1   0    6   3  23   */
/* 1  0  0   0    8   4  5    */
/* 1  0  1   0    10  5  8    */
/* 1  1  0   0    12  6  6    */
/* 1  1  1   0    14  7  7    */
/* x  x  x   1    -   -  -    */
#define CS22                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH,  0, INH)
#define CS21                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH,  2, INH)
#define CS20                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH,  4, INH)
#define CS23                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH,  6, INH)
#define CS5                  CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH,  8, INH)
#define CS8                  CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH, 10, INH)
#define CS6                  CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH, 12, INH)
#define CS7                  CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH, 14, INH)


/* c1 b1 a1 i1   bcd  ch cs */
/* 0  0  0  0    0    0  18 */
/* 0  0  1  0    2    1  17 */
/* 0  1  0  0    4    2  16 */
/* 0  1  1  0    6    3  19 */
/* 1  0  0  0    8    4  1  */
/* 1  0  1  0    10   5  4  */
/* 1  1  0  0    12   6  2  */
/* 1  1  1  0    14   7  3  */
/* x  x  x  1    -    -     */
#define CS18                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH, INH,  0)
#define CS17                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH, INH,  2)
#define CS16                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH, INH,  4)
#define CS19                 CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH, INH,  6)
#define CS1                  CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH, INH,  8)
#define CS4                  CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH, INH, 10)
#define CS2                  CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH, INH, 12)
#define CS3                  CTRL(PCA9555_0_ADDR, REG_PORTOUT, INH, INH, INH, 14)


/* c13 b13 a13 i13   bcd  ch cs */
/* 0   0   0   0     0    0  26 */
/* 0   0   1   0     2    1  25 */
/* 0   1   0   0     4    2  24 */
/* 0   1   1   0     6    3  27 */
/* 1   0   0   0     8    4  9  */
/* 1   0   1   0     10   5  12 */
/* 1   1   0   0     12   6  10 */
/* 1   1   1   0     14   7  11 */
/* x   x   x   1     -    -     */
#define RST26                 CTRL(PCA9555_1_ADDR, REG_PORTOUT,  0, INH, INH, INH)
#define RST25                 CTRL(PCA9555_1_ADDR, REG_PORTOUT,  2, INH, INH, INH)
#define RST24                 CTRL(PCA9555_1_ADDR, REG_PORTOUT,  4, INH, INH, INH)
#define RST27                 CTRL(PCA9555_1_ADDR, REG_PORTOUT,  6, INH, INH, INH)
#define RST9                  CTRL(PCA9555_1_ADDR, REG_PORTOUT,  8, INH, INH, INH)
#define RST12                 CTRL(PCA9555_1_ADDR, REG_PORTOUT, 10, INH, INH, INH)
#define RST10                 CTRL(PCA9555_1_ADDR, REG_PORTOUT, 12, INH, INH, INH)
#define RST11                 CTRL(PCA9555_1_ADDR, REG_PORTOUT, 14, INH, INH, INH)


/* c17 b17 a17 i17   bcd  ch cs */
/* 0   0   0   0     0    0  43 */
/* 0   0   1   0     2    1  30 */
/* 0   1   0   0     4    2  29 */
/* 0   1   1   0     6    3  44 */
/* 1   0   0   0     8    4  13 */
/* 1   0   1   0     10   5  28 */
/* 1   1   0   0     12   6  14 */
/* 1   1   1   0     14   7  15 */
/* x   x   x   1     -    -     */
#define RST43                 CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH,  0, INH, INH)
#define RST30                 CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH,  2, INH, INH)
#define RST29                 CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH,  4, INH, INH)
#define RST44                 CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH,  6, INH, INH)
#define RST13                 CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH,  8, INH, INH)
#define RST28                 CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, 10, INH, INH)
#define RST14                 CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, 12, INH, INH)
#define RST15                 CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, 14, INH, INH)


/* a7 b7 c7 i7  bcd  ch cs */
/* 0  0  0  0    0   0  43 */
/* 1  0  0  0    8   1  30 */
/* 0  1  0  0    4   2  29 */
/* 1  1  0  0    12  3  44 */
/* 0  0  1  0    2   4  13 */
/* 1  0  1  0    10  5  28 */
/* 0  1  1  0    6   6  14 */
/* 1  1  1  0    14  7  15 */
/* x  x  x  1    -   -     */
#define CS43                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH,  0, INH)
#define CS30                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH,  8, INH)
#define CS29                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH,  4, INH)
#define CS44                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH, 12, INH)
#define CS13                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH,  2, INH)
#define CS28                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH, 10, INH)
#define CS14                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH,  6, INH)
#define CS15                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH, 14, INH)


/* c3 b3 a3 i3  bcd  ch cs */
/* 0  0  0  0    0   0  26 */
/* 0  0  1  0    2   1  25 */
/* 0  1  0  0    4   2  24 */
/* 0  1  1  0    6   3  27 */
/* 1  0  0  0    8   4  9  */
/* 1  0  1  0    10  5  12 */
/* 1  1  0  0    12  6  10 */
/* 1  1  1  0    14  7  11 */
/* x  x  x  1    -   -     */
#define CS26                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH, INH,  0)
#define CS25                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH, INH,  2)
#define CS24                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH, INH,  4)
#define CS27                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH, INH,  6)
#define CS9                   CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH, INH,  8)
#define CS12                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH, INH, 10)
#define CS10                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH, INH, 12)
#define CS11                  CTRL(PCA9555_1_ADDR, REG_PORTOUT, INH, INH, INH, 14)


/* i14 c14 b14 a14   bcd ch    cs */
/* 0   0   0   0     0   0     48 */
/* 0   0   0   1     1   1     47 */
/* 0   0   1   0     2   2     46 */
/* 0   0   1   1     3   3     49 */
/* 0   1   0   0     4   4     31 */
/* 0   1   0   1     5   5     34 */
/* 0   1   1   0     6   6     32 */
/* 0   1   1   1     7   7     33 */
/* 1   x   x   x     -   -        */
#define RST48                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, 0, INH, INH, INH)
#define RST47                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, 1, INH, INH, INH)
#define RST46                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, 2, INH, INH, INH)
#define RST49                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, 3, INH, INH, INH)
#define RST31                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, 4, INH, INH, INH)
#define RST34                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, 5, INH, INH, INH)
#define RST32                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, 6, INH, INH, INH)
#define RST33                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, 7, INH, INH, INH)


/* a15 b15 c15 i15 bcd  ch cs */
/* 0   0   0   0   0    0  52 */
/* 1   0   0   0   8    1  51 */
/* 0   1   0   0   4    2  50 */
/* 1   1   0   0   12   3  53 */
/* 0   0   1   0   2    4  35 */
/* 1   0   1   0   10   5  38 */
/* 0   1   1   0   6    6  36 */
/* 1   1   1   0   14   7  37 */
/* x   x   x   1   -    -     */
#define RST52                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH,  0, INH, INH)
#define RST51                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH,  8, INH, INH)
#define RST50                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH,  4, INH, INH)
#define RST53                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, 12, INH, INH)
#define RST35                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH,  2, INH, INH)
#define RST38                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, 10, INH, INH)
#define RST36                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH,  6, INH, INH)
#define RST37                 CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, 14, INH, INH)


/* a5 b5 c5 i5 bcd  ch cs */
/* 0  0  0  0   0   0  52 */
/* 1  0  0  0   8   1  51 */
/* 0  1  0  0   4   2  50 */
/* 1  1  0  0   12  3  53 */
/* 0  0  1  0   2   4  35 */
/* 1  0  1  0   10  5  38 */
/* 0  1  1  0   6   6  36 */
/* 1  1  1  0   14  7  37 */
/* x  x  x  1   -   -     */
#define CS52                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH,  0, INH)
#define CS51                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH,  8, INH)
#define CS50                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH,  4, INH)
#define CS53                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH, 12, INH)
#define CS35                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH,  2, INH)
#define CS38                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH, 10, INH)
#define CS36                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH,  6, INH)
#define CS37                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH, 14, INH)


/*  i4 c4 b4 a4 bcd  ch cs */
/*  0  0  0  0   0   0  48 */
/*  0  0  0  1   1   1  47 */
/*  0  0  1  0   2   2  46 */
/*  0  0  1  1   3   3  49 */
/*  0  1  0  0   4   4  31 */
/*  0  1  0  1   5   5  34 */
/*  0  1  1  0   6   6  32 */
/*  0  1  1  1   7   7  33 */
/*  1  x  x  x   -   -     */
#define CS48                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH, INH, 0)
#define CS47                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH, INH, 1)
#define CS46                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH, INH, 2)
#define CS49                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH, INH, 3)
#define CS31                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH, INH, 4)
#define CS34                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH, INH, 5)
#define CS32                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH, INH, 6)
#define CS33                  CTRL(PCA9555_2_ADDR, REG_PORTOUT, INH, INH, INH, 7)


/*   c10 b10 a10 i10 bcd  ch cs */
/*   0   0   0   0    0   0  -  */
/*   0   0   1   0    2   1  -  */
/*   0   1   0   0    4   2  -  */
/*   0   1   1   0    6  3  -  */
/*   1   0   0   0    8   4  69 */
/*   1   0   1   0    10  5  -  */
/*   1   1   0   0    12  6  70 */
/*   1   1   1   0    14  7  71 */
/*   x   x   x   1    -   -     */
#define CS69                  CTRL(PCA9555_3_ADDR, REG_PORTOUT,   8, INH, INH, INH)
#define CS70                  CTRL(PCA9555_3_ADDR, REG_PORTOUT,  12, INH, INH, INH)
#define CS71                  CTRL(PCA9555_3_ADDR, REG_PORTOUT,  14, INH, INH, INH)


/*   a20 b20 c20 i20 bcd  ch cs */
/*   0   0   0   0    0   0  -  */
/*   1   0   0   0    8   1  -  */
/*   0   1   0   0    4   2  -  */
/*   1   1   0   0    12  3  -  */
/*   0   0   1   0    2   4  69 */
/*   1   0   1   0    10  5  -  */
/*   0   1   1   0    6   6  70 */
/*   1   1   1   0    14  7  71 */
/*   x   x   x   1    -   -     */
#define RST69                 CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH,  2, INH, INH)
#define RST70                 CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH,  6, INH, INH)
#define RST71                 CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, 14, INH, INH)


/*   a9 b9 c9 i9 bcd  ch cs */
/*   0  0  0  0   0   0  67 */
/*   1  0  0  0   8   1  66 */
/*   0  1  0  0   4   2  65 */
/*   1  1  0  0   12  3  68 */
/*   0  0  1  0   2   4  61 */
/*   1  0  1  0   10  5  64 */
/*   0  1  1  0   6   6  62 */
/*   1  1  1  0   14  7  63 */
/*   x  x  x  1   -   -     */
#define CS67                  CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH,  0, INH)
#define CS66                  CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH,  8, INH)
#define CS65                  CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH,  4, INH)
#define CS68                  CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH, 12, INH)
#define CS61                  CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH,  2, INH)
#define CS64                  CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH, 10, INH)
#define CS62                  CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH,  6, INH)
#define CS63                  CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH, 14, INH)


/*   i19 a19 b19 c19 bcd  ch cs */
/*   0   0   0   0   0    0  67 */
/*   0   1   0   0   4    1  66 */
/*   0   0   1   0   2    2  65 */
/*   0   1   1   0   6    3  68 */
/*   0   0   0   1   1    4  61 */
/*   0   1   0   1   5    5  64 */
/*   0   0   1   1   3    6  62 */
/*   0   1   1   1   7    7  63 */
/*   1   x   x   x   -    -     */
#define RST67                 CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH, INH,  0)
#define RST66                 CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH, INH,  4)
#define RST65                 CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH, INH,  2)
#define RST68                 CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH, INH,  6)
#define RST61                 CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH, INH,  1)
#define RST64                 CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH, INH,  5)
#define RST62                 CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH, INH,  3)
#define RST63                 CTRL(PCA9555_3_ADDR, REG_PORTOUT, INH, INH, INH,  7)


/*  a16 b16 c16 i16 bcd  ch cs */
/*  0   0   0   0   0    0  56 */
/*  1   0   0   0   8    1  55 */
/*  0   1   0   0   4    2  54 */
/*  1   1   0   0   12   3  57 */
/*  0   0   1   0   2    4  39 */
/*  1   0   1   0   10   5  42 */
/*  0   1   1   0   6    6  40 */
/*  1   1   1   0   14   7  41 */
/*  x   x   x   1   -    -     */
#define RST56                 CTRL(PCA9555_4_ADDR, REG_PORTOUT,  0, INH, INH, INH)
#define RST55                 CTRL(PCA9555_4_ADDR, REG_PORTOUT,  8, INH, INH, INH)
#define RST54                 CTRL(PCA9555_4_ADDR, REG_PORTOUT,  4, INH, INH, INH)
#define RST57                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, 12, INH, INH, INH)
#define RST39                 CTRL(PCA9555_4_ADDR, REG_PORTOUT,  2, INH, INH, INH)
#define RST42                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, 10, INH, INH, INH)
#define RST40                 CTRL(PCA9555_4_ADDR, REG_PORTOUT,  6, INH, INH, INH)
#define RST41                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, 14, INH, INH, INH)


/*  a18 b18 c18 i18 bcd  ch cs */
/*  0   0   0   0   0    0  74 */
/*  1   0   0   0   8    1  73 */
/*  0   1   0   0   4    2  72 */
/*  1   1   0   0   12   3  75 */
/*  0   0   1   0   2    4  58 */
/*  1   0   1   0   10   5  60 */
/*  0   1   1   0   6    6  45 */
/*  1   1   1   0   14   7  59 */
/*  x   x   x   1   -    -     */
#define RST74                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH,  0, INH, INH)
#define RST73                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH,  8, INH, INH)
#define RST72                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH,  4, INH, INH)
#define RST75                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, 12, INH, INH)
#define RST58                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH,  2, INH, INH)
#define RST60                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, 10, INH, INH)
#define RST45                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH,  6, INH, INH)
#define RST59                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, 14, INH, INH)


/* a8 b8 c8 i8 bcd ch cs */
/* 0  0  0  0  0   0  74 */
/* 1  0  0  0  8   1  73 */
/* 0  1  0  0  4   2  72 */
/* 1  1  0  0  12  3  75 */
/* 0  0  1  0  2   4  58 */
/* 1  0  1  0  10  5  60 */
/* 0  1  1  0  6   6  45 */
/* 1  1  1  0  14  7  59 */
/* x  x  x  1  -   -     */
#define CS74                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH,  0, INH)
#define CS73                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH,  8, INH)
#define CS72                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH,  4, INH)
#define CS75                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH, 12, INH)
#define CS58                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH,  2, INH)
#define CS60                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH, 10, INH)
#define CS45                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH,  6, INH)
#define CS59                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH, 14, INH)



/*  a6 b6 c6 i6 bcd  ch cs */
/*  0  0  0  0  0    0  56 */
/*  1  0  0  0  8    1  55 */
/*  0  1  0  0  4    2  54 */
/*  1  1  0  0  12   3  57 */
/*  0  0  1  0  2    4  39 */
/*  1  0  1  0  10   5  42 */
/*  0  1  1  0  6    6  40 */
/*  1  1  1  0  14   7  41 */
/*  x  x  x  1  -    -     */
#define CS56                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH, INH,  0)
#define CS55                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH, INH,  8)
#define CS54                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH, INH,  4)
#define CS57                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH, INH, 12)
#define CS39                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH, INH,  2)
#define CS42                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH, INH, 10)
#define CS40                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH, INH,  6)
#define CS41                 CTRL(PCA9555_4_ADDR, REG_PORTOUT, INH, INH, INH, 14)



/* reset the soc */
#define RESET_SOC(n)         (RST##n)

/* chip selection soc */
#define CS_SOC(n)            (CS##n)

#endif
