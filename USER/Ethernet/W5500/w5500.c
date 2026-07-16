//*****************************************************************************
//
//! \file w5500.c
//! \brief W5500 HAL Interface.
//! \version 1.0.0
//! \date 2013/10/21
//! \par  Revision history
//!       <2013/10/21> 1st Release
//! \author MidnightCow
//! \copyright
//!
//! Copyright (c)  2013, WIZnet Co., LTD.
//! All rights reserved.
//! 
//! Redistribution and use in source and binary forms, with or without 
//! modification, are permitted provided that the following conditions 
//! are met: 
//! 
//!     * Redistributions of source code must retain the above copyright 
//! notice, this list of conditions and the following disclaimer. 
//!     * Redistributions in binary form must reproduce the above copyright
//! notice, this list of conditions and the following disclaimer in the
//! documentation and/or other materials provided with the distribution. 
//!     * Neither the name of the <ORGANIZATION> nor the names of its 
//! contributors may be used to endorse or promote products derived 
//! from this software without specific prior written permission. 
//! 
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
//! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
//! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
//! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
//! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
//! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
//! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
//! THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************
#include "trace.h"
#include "w5500.h"


#define _W5500_SPI_VDM_OP_          0x00
#define _W5500_SPI_FDM_OP_LEN1_     0x01
#define _W5500_SPI_FDM_OP_LEN2_     0x02
#define _W5500_SPI_FDM_OP_LEN4_     0x03

////////////////////////////////////////////////////

uint8_t  WIZCHIP_READ(uint32_t AddrSel)
{
    uint8_t ret;

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if( (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_))

#if  ( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_ ) || ( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_DMA_ )
    AddrSel |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);
#elif( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_ )
    AddrSel |= (_W5500_SPI_READ_ | _W5500_SPI_FDM_OP_LEN1_);
#else
#error "Unsupported _WIZCHIP_IO_SPI_ in W5500 !!!"
#endif

#if ( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_DMA_ )
    uint8_t frame[4];
    frame[0] = (AddrSel >> 16) & 0xff;
    frame[1] = (AddrSel >> 8) & 0xff;
    frame[2] = (AddrSel >> 0) & 0xff;
    WIZCHIP.IF.SPI._tx_req(frame, 3);
    ret = WIZCHIP.IF.SPI._read_byte();
#else
    /* VDM Frame MSB first : Address(16bits) + Control(8bits : BSB<<3 | R/W<<2 | OP<<0) + Data(8bits) */
    WIZCHIP.IF.SPI._write_byte((AddrSel & 0x00FF0000) >> 16);
    WIZCHIP.IF.SPI._write_byte((AddrSel & 0x0000FF00) >> 8);
    WIZCHIP.IF.SPI._write_byte((AddrSel & 0x000000FF) >> 0);
    ret = WIZCHIP.IF.SPI._read_byte();
#endif

#elif ( (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_BUS_) )

#if  (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_DIR_)

#elif(_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_)

#else
#error "Unsupported _WIZCHIP_IO_MODE_BUS_ in W5500 !!!"
#endif
#else
#error "Unknown _WIZCHIP_IO_MODE_ in W5000. !!!"   
#endif

    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
    return ret;
}

void     WIZCHIP_WRITE(uint32_t AddrSel, uint8_t wb)
{
    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if( (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_))

#if  ( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_ ) || ( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_DMA_ )
    AddrSel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);
#elif( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_ )
    AddrSel |= (_W5500_SPI_WRITE_ | _W5500_SPI_FDM_OP_LEN1_);
#else
#error "Unsupported _WIZCHIP_IO_SPI_ in W5500 !!!"
#endif
    /* VDM Frame MSB first : Address(16bits) + Control(8bits : BSB<<3 | R/W<<2 | OP<<0) + Data(8bits) */
#if ( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_DMA_ )
    uint8_t frame[4];

    frame[0] = (AddrSel >> 16) & 0xff;
    frame[1] = (AddrSel >> 8) & 0xff;
    frame[2] = (AddrSel >> 0) & 0xff;
    frame[3] = wb;
    WIZCHIP.IF.SPI._tx_req(frame, 4);
#else
    WIZCHIP.IF.SPI._write_byte((AddrSel & 0x00FF0000) >> 16);
    WIZCHIP.IF.SPI._write_byte((AddrSel & 0x0000FF00) >> 8);
    WIZCHIP.IF.SPI._write_byte((AddrSel & 0x000000FF) >> 0);
    WIZCHIP.IF.SPI._write_byte(wb);
#endif

#elif ( (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_BUS_) )

#if  (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_DIR_)

#elif(_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_)

#else
#error "Unsupported _WIZCHIP_IO_MODE_BUS_ in W5500 !!!"
#endif
#else
#error "Unknown _WIZCHIP_IO_MODE_ in W5500. !!!"
#endif

    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
}

void     WIZCHIP_READ_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t len)
{
    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if( (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_))

#if  ( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_ )
    /* sw read 9byte elapsed : 11.16us */
//TRIG_LOW(TRIG6);
    AddrSel |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);
    WIZCHIP.IF.SPI._write_byte((AddrSel & 0x00FF0000) >> 16);
    WIZCHIP.IF.SPI._write_byte((AddrSel & 0x0000FF00) >> 8);
    WIZCHIP.IF.SPI._write_byte((AddrSel & 0x000000FF) >> 0);
    for (int i = 0; i < len; i++)
        pBuf[i] = WIZCHIP.IF.SPI._read_byte();
    //TRIG_HIGH(TRIG6);
#elif( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_ )
    uint16_t i = 0, j = 0;

    AddrSel |= (_W5500_SPI_READ_ | _W5500_SPI_FDM_OP_LEN4_);
    for (i = 0; i < len / 4; i++)
    {
        WIZCHIP.IF.SPI._write_byte((AddrSel & 0x00FF0000) >> 16);
        WIZCHIP.IF.SPI._write_byte((AddrSel & 0x0000FF00) >> 8);
        WIZCHIP.IF.SPI._write_byte((AddrSel & 0x000000FF) >> 0);
        pBuf[i * 4] = WIZCHIP.IF.SPI._read_byte();
        pBuf[i * 4 + 1] = WIZCHIP.IF.SPI._read_byte();
        pBuf[i * 4 + 2] = WIZCHIP.IF.SPI._read_byte();
        pBuf[i * 4 + 3] = WIZCHIP.IF.SPI._read_byte();
        //AddrSel += (4 << 8);    // offset address + 4
        AddrSel = WIZCHIP_OFFSET_INC(AddrSel, 4);
    }
    len %= 4;      // for the rest data
    if (len)
    {
        AddrSel -= 1;  // change _W5500_SPI_FDM_OP_LEN4_ to _W5500_SPI_FDM_OP_LEN2_
        i *= 4;
        for (uint16_t j = 0; j < len / 2; j++)
        {
            WIZCHIP.IF.SPI._write_byte((AddrSel & 0x00FF0000) >> 16);
            WIZCHIP.IF.SPI._write_byte((AddrSel & 0x0000FF00) >> 8);
            WIZCHIP.IF.SPI._write_byte((AddrSel & 0x000000FF) >> 0);
            pBuf[i] = WIZCHIP.IF.SPI._read_byte();
            pBuf[i + 1] = WIZCHIP.IF.SPI._read_byte();
            i += 2;
            //AddrSel += (2 << 8);    // offset address + 2
            AddrSel = WIZCHIP_OFFSET_INC(AddrSel, 2);
        }
        len %= 2;
        if (len)
        {
            AddrSel -= 1;  // change _W5500_SPI_FDM_OP_LEN2_ to _W5500_SPI_FDM_OP_LEN1_
            WIZCHIP.IF.SPI._write_byte((AddrSel & 0x00FF0000) >> 16);
            WIZCHIP.IF.SPI._write_byte((AddrSel & 0x0000FF00) >> 8);
            WIZCHIP.IF.SPI._write_byte((AddrSel & 0x000000FF) >> 0);
            pBuf[i] = WIZCHIP.IF.SPI._read_byte();
        }
    }
#elif ( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_DMA_ ) 
    /* VDM Frame MSB first : Address(16bits) + Control(8bits : BSB<<3 | R/W<<2 | OP<<0) + Data(8bits) */

    uint8_t frame[4];

    /* dma 9 byte elapsed: 8.43us */
    //TRIG_LOW(TRIG6);
    AddrSel |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);
    frame[0] = (AddrSel >> 16) & 0xff;
    frame[1] = (AddrSel >> 8) & 0xff;
    frame[2] = (AddrSel >> 0) & 0xff;
    WIZCHIP.IF.SPI._tx_req(frame, 3);
    WIZCHIP.IF.SPI._rx_req(pBuf, len);
    //TRIG_HIGH(TRIG6);

#else
#error "Unsupported _WIZCHIP_IO_SPI_ in W5500 !!!"
#endif

#elif ( (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_BUS_) )

#if  (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_DIR_)

#elif(_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_)

#else
#error "Unsupported _WIZCHIP_IO_MODE_BUS_ in W5500 !!!"
#endif
#else
#error "Unknown _WIZCHIP_IO_MODE_ in W5500. !!!!"
#endif

    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
}

void     WIZCHIP_WRITE_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t len)
{
    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if( (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_))

#if  ( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_ )
    //TRIG_LOW(TRIG7);
    AddrSel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);
    WIZCHIP.IF.SPI._write_byte((AddrSel & 0x00FF0000) >> 16);
    WIZCHIP.IF.SPI._write_byte((AddrSel & 0x0000FF00) >> 8);
    WIZCHIP.IF.SPI._write_byte((AddrSel & 0x000000FF) >> 0);
    for (int i = 0; i < len; i++)
        WIZCHIP.IF.SPI._write_byte(pBuf[i]);
    //TRIG_HIGH(TRIG7);
#elif( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_ )
    uint16_t i = 0, j = 0;

    AddrSel |= (_W5500_SPI_WRITE_ | _W5500_SPI_FDM_OP_LEN4_);
    for (i = 0; i < len / 4; i++)
    {
        WIZCHIP.IF.SPI._write_byte((AddrSel & 0x00FF0000) >> 16);
        WIZCHIP.IF.SPI._write_byte((AddrSel & 0x0000FF00) >> 8);
        WIZCHIP.IF.SPI._write_byte((AddrSel & 0x000000FF) >> 0);
        WIZCHIP.IF.SPI._write_byte(pBuf[i * 4]);
        WIZCHIP.IF.SPI._write_byte(pBuf[i * 4 + 1]);
        WIZCHIP.IF.SPI._write_byte(pBuf[i * 4 + 2]);
        WIZCHIP.IF.SPI._write_byte(pBuf[i * 4 + 3]);
        //AddrSel += (4 << 8);    // offset address + 4
        AddrSel = WIZCHIP_OFFSET_INC(AddrSel, 4);
    }
    len %= 4;      // for the rest data
    if (len)
    {
        AddrSel -= 1;  // change _W5500_SPI_FDM_OP_LEN4_ to _W5500_SPI_FDM_OP_LEN2_
        i *= 4;
        for (uint16_t j = 0; j < len / 2; j++)
        {
            WIZCHIP.IF.SPI._write_byte((AddrSel & 0x00FF0000) >> 16);
            WIZCHIP.IF.SPI._write_byte((AddrSel & 0x0000FF00) >> 8);
            WIZCHIP.IF.SPI._write_byte((AddrSel & 0x000000FF) >> 0);
            WIZCHIP.IF.SPI._write_byte(pBuf[i]);
            WIZCHIP.IF.SPI._write_byte(pBuf[i + 1]);
            i += 2;
            //AddrSel += (2 << 8);    // offset address + 2
            AddrSel = WIZCHIP_OFFSET_INC(AddrSel, 2);
        }
        len %= 2;
        if (len)
        {
            AddrSel -= 1;  // change _W5500_SPI_FDM_OP_LEN2_ to _W5500_SPI_FDM_OP_LEN1_
            WIZCHIP.IF.SPI._write_byte((AddrSel & 0x00FF0000) >> 16);
            WIZCHIP.IF.SPI._write_byte((AddrSel & 0x0000FF00) >> 8);
            WIZCHIP.IF.SPI._write_byte((AddrSel & 0x000000FF) >> 0);
            WIZCHIP.IF.SPI._write_byte(pBuf[i]);
        }
    }
#elif ( _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_DMA_ )
    uint8_t frame[4];

    //TRIG_LOW(TRIG7); 
    /* VDM Frame MSB first : Address(16bits) + Control(8bits : BSB<<3 | R/W<<2 | OP<<0) + Data(8bits) */
    AddrSel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);
    frame[0] = (AddrSel >> 16) & 0xff;
    frame[1] = (AddrSel >> 8) & 0xff;
    frame[2] = (AddrSel >> 0) & 0xff;
    WIZCHIP.IF.SPI._tx_req(frame, 3);
    WIZCHIP.IF.SPI._tx_req(pBuf, len);

    //TRIG_HIGH(TRIG7); 

#else
#error "Unsupported _WIZCHIP_IO_SPI_ in W5500 !!!"
#endif

#elif ( (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_BUS_) )

#if  (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_DIR_)

#elif(_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_)

#else
#error "Unsupported _WIZCHIP_IO_MODE_BUS_ in W5500 !!!"
#endif
#else
#error "Unknown _WIZCHIP_IO_MODE_ in W5500. !!!!"
#endif

    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
}


uint16_t getSn_TX_FSR(uint8_t sn)
{
    uint16_t val = 0, val1 = 0;
    do
    {
        val1 = WIZCHIP_READ(Sn_TX_FSR(sn));
        val1 = (val1 << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_TX_FSR(sn), 1));
        if (val1 != 0)
        {
            val = WIZCHIP_READ(Sn_TX_FSR(sn));
            val = (val << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_TX_FSR(sn), 1));
        }
    } while (val != val1);
    return val;
}


uint16_t getSn_RX_RSR(uint8_t sn)
{
    uint16_t val = 0, val1 = 0;
    do
    {
        val1 = WIZCHIP_READ(Sn_RX_RSR(sn));
        val1 = (val1 << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_RSR(sn), 1));
        if (val1 != 0)
        {
            val = WIZCHIP_READ(Sn_RX_RSR(sn));
            val = (val << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_RSR(sn), 1));
        }
    } while (val != val1);
    return val;
}

void wiz_send_data(uint8_t sn, uint8_t* wizdata, uint16_t len)
{
    uint16_t ptr = 0;
    uint32_t addrsel = 0;
    if (len == 0)  return;
    ptr = getSn_TX_WR(sn);

    addrsel = (ptr << 8) + (WIZCHIP_TXBUF_BLOCK(sn) << 3);
    WIZCHIP_WRITE_BUF(addrsel, wizdata, len);

    ptr += len;
    setSn_TX_WR(sn, ptr);
}

void wiz_recv_data(uint8_t sn, uint8_t* wizdata, uint16_t len)
{
    uint16_t ptr = 0;
    uint32_t addrsel = 0;

    if (len == 0) return;
    ptr = getSn_RX_RD(sn);
    addrsel = (ptr << 8) + (WIZCHIP_RXBUF_BLOCK(sn) << 3);

    WIZCHIP_READ_BUF(addrsel, wizdata, len);
    ptr += len;

    setSn_RX_RD(sn, ptr);
}


void wiz_recv_ignore(uint8_t sn, uint16_t len)
{
    uint16_t ptr = 0;
    ptr = getSn_RX_RD(sn);
    ptr += len;
    setSn_RX_RD(sn, ptr);
}

