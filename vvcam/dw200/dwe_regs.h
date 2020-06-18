/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/
#ifndef _DWE_REGS_H_
#define _DWE_REGS_H_

#define REGISTER_NUM 100

/* DeWarpId 0x0000
   NUM 23:4 r Product Number. 100 for this IP.
*/
#define DEWARP_REGISTER_BASE_ADDR 0x00000C00

/* DewarpCtl 0x0004
   ENABLE 0:0 rw Enable the IP.
   START 1:1 rw Pulse to start dewarp to correct one image in the source.
   SOFT_RESET 2:2 rw Soft reset, active high
   CFG_UPDATE 3:3 rw Shadow register trigger bit. Use to trigger update after a configuration change.
   INPUT_FORMAT 5:4 rw Input format. 0: YUV422 semi-planar; 1: YUV422 interleaved; 2: YVU420 semi-planar.
   OUTPUT_FORMAT 7:6 rw Output format. 0: YUV422 semi-planar; 1: YUV422 interleaved; 2: YVU420 semi-planar.
   SRC_AUTO_SHADOW 8:8 rw Specify buffer mode for input source buffer. 0: Single buffer 1: Automatically switch between 2 source buffers
   HW_HANDSHAKE_EN 9:9 rw Specify signal to start dewarp correction: 0: SW 1: HW
   DST_AUTO_SHADOW 10:10 rw Specify buffer mode for output destination buffer. 0: Single buffer 1: Automatically switch between 2 destination buffers
   MODE_SELECT 11:11 rw Black line mode when there is a split image: 0: No black line 1: Images are separated by a black line
   CFG_FIX_VALUE 12:12 rw Specify when registers update. 0: registers update when frame end is controlled by cfg_gen_update 1: register update automatically at every frame end
   CFG_GEN_UPDATE 13:13 rw Shadow register trigger bit. Use to trigger a flip for each frame end.
   REG_SEL 17:16 rw 0:prefetch image pixel traversal 1:prefetch image pixel calculation 2:prefetch automatically
   REG_NUM 24:18 rw The 16x16 block contain num for prefetching automatically, if less than this number traversal image pixel, else calculation image pixel
   OUTPUT SEL 26:25  0 vse,  1 dma, 2  vse+dma
*/
#define DEWARP_CTRL         0x00000004

/* SwapCtl 0x0058
   SRC_SWAP_Y 3:0 rw Source1 Y swap
   SRC_SWAP_C 7:4 rw Source1 UV swap
   DST_SWAP_Y 11:8 rw Source2 Y swap
   DST_SWAP_C 15:12 rw Source2 UV swap
   SRC_SWAP_Y_2 19:16 rw Destination1 Y swap
   SRC_SWAP_C_2 23:20 rw Destination1 UV swap
   DST_SWAP_Y_2 27:24 rw Destination2 Y swap
   DST_SWAP_C_2 31:28 rw Destination2 UV swap
*/
#define SWAP_CONTROL        0x00000058

/* VSplitLine 0x005C
   VERT_SPLIT_LINE1 12:0 rw Image vertical split line1.
   VERT_SPLIT_LINE2 28:16 rw Image vertical split line2.
*/
#define VERTICAL_SPLIT_LINE 0x0000005C

/* HSplitLine 0x0060
   HOR_SPLIT_LINE 12:0 rw Image horizontal split line1.
*/
#define HORIZON_SPLIT_LINE  0x00000060

/* ScaleFactor 0x0064
   SCALE_FACTOR 31:0 rw Scale factor  [32, 128]
*/
#define SCALE_FACTOR    0x00000064

/* ROIStart 0x0068
    ROI_STX 12:0 rw Region of Interest X origin.
    ROI_STY 28:16 rw Region of Interest Y origin.
*/
#define ROI_START       0x00000068

/* Boundry Pixel 0x006C
   BOUNDRY_PIXELV 7:0 rw Boundary pixel value for V plane.
   BOUNDRY_PIXELU 15:8 rw Boundary pixel value for U plane.
   BOUNDRY_PIXELY 23:16 rw Boundary pixel value for Y plane.
*/
#define BOUNDRY_PIXEL 0x0000006C

/* INTERRUPT STATUS  0x0070
   INT_FRAME_DONE 0:0 r Frame done. READ ONLY.
   INT_ERR_STATUS 7:1 r Error status. READ ONLY.
   INT_MSK_STATUS 15:8 rw Interrupt Mask for bits 7:1
   FRAME_BUSY 16 r Frame busy. READ ONLY.
   INT_CLR 31:24 w Clear Interrupt Status for bits 7:0. WRITE ONLY.
*/
#define INTERRUPT_STATUS    0x00000070

#define INT_FRAME_DONE      (1 << 0)

#define INT_ERR_STATUS_MASK  0x000000FE
#define INT_ERR_STATUS_SHIFT 1

#define INT_MSK_STATUS_MASK  0x0000FF00
#define INT_MSK_STATUS_SHIFT 8

#define INT_FRAME_BUSY       0x00010000
#define INT_CLR_MASK         0x00007F00
#define INT_RESET_MASK       0x01000000

/* BUSCTL 0x0074
   AXI_MASTER_RD_OUTSTANDING_NUMBER 7:0 rw
   AXI maximum outstanding reads. Add 2 to this value (default is d’14).
   AXI_MASTER_RD_ID 15:8 rw AXI master Read ID
   AXI_MASTER_WR_ID 23:16 rw AXI master Write ID
   AXI_MASTER_TIMEOUT_OVERRIDE_ENABLE 26:26 rw AXI master timeout override enable set by software 1: Timeout cycles set by register. 0: timeout cycles set by a default parameter
   AXI_MASTER_WR_ID 29:29 rw AXI master Write ID enable 1: Enable
   AXI_MASTER_RD_ID 30:30 rw AXI master Read ID enable 1: Enable
   AXI_MASTER_ENABLE 31:31 rw AXI master enable 1: Enable
*/
#define BUS_CTRL                  0x00000074
#define DEWRAP_BUS_CTRL_ENABLE_MASK  (1 << 31)

/* BUSCTL1 0x0078
   AXI_MASTER_WR_OUTSTANDING_NUMBER 7:0 rw AXI maximum outstanding writes. Add 2 to this value (default is d’14).
*/
#define BUS_CTRL1                 0x00000078

/* ButTimeoutCircle 0x007C
   AXI_MASTER_TIME_OUT_CYCLE 30:0 rw Axi master, number of cycles to timeout
*/
#define BUS_TIME_OUT_CYCLE        0x0000007C

/* MapLutAddr 0x0008
   LUT_BASE 29:0 rw LUT start address. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define MAP_LUT_ADDR              0x00000008

/* MapLutSize 0x000C
   LUT_HSIZE 10:0 rw LUT horizontal size in words
   LUT_VSIZE 26:16 rw LUT vertical size in rows/lines
*/
#define MAP_LUT_SIZE              0x0000000C

/* SrcImgYBase 0x0010
   SRC_Y_BASE 29:0 r Address of Y plane origin. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define SRC_IMG_Y_BASE            0x00000010

/* SrcImgUVBase 0x0014
   SRC_UV_BASE 29:0 r Address of UV plane origin. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define SRC_IMG_UV_BASE           0x00000014

/* SrcImgSize 0x0018
   SRC_HSIZE 12:0 rw Source1 image horizontal stride in pixels.
   SRC_VSIZE 27:16 rw Source1 image vertical resolution in rows/lines
*/
#define SRC_IMG_SIZE              0x00000018

/* SrcImgStride 0x001C
   SRC_STRIDE 15:0 rw Source1 image horizontal resolution in pixels
*/
#define SRC_IMG_STRIDE            0x0000001C

/* MapLutAddr2 0x0020
   LUT_BASE 29:0 rw LUT start address. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define MAP_LUT_ADDR2             0x00000020

/* MapLutSize2 0x0024
   LUT_HSIZE 10:0 rw LUT horizontal size in pixels.
   LUT_VSIZE 26:16 rw LUT vertical size in rows/lines
*/
#define MAP_LUT_SIZE2             0x00000024

/* SrcImgYBase2 0x0028
   SRC_Y_BASE 29:0 r Address of Y plane origin. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define SRC_IMG_Y_BASE2           0x00000028

/* SrcImgUVBase2 0x002C
   SRC_UV_BASE 29:0 r Address of UV plane origin. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define SRC_IMG_UV_BASE2          0x0000002C

/* SrcImgSize2 0x0030
   SRC_HSIZE 12:0 rw Source2 image horizontal stride in pixels.
   SRC_VSIZE 27:16 rw Source2 image vertical resolution in rows/lines
*/
#define SRC_IMG_SIZE2             0x00000030

/* SrcImgStride2 0x0034
   SRC_STRIDE 15:0 rw Source2 image horizontal resolution in pixels.
   SRC_VSIZE 27:16 rw Source2 image vertical resolution in rows/lines
*/
#define SRC_IMG_STRIDE2           0x00000034

/* DstImgYBase 0x0038
   DST_Y_BASE 29:0 r Address of Y plane origin. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define DST_IMG_Y_BASE            0x00000038

/* DstImgUVBase 0x003C
   DST_UV_BASE 29:0 r Address of UV plane origin. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define DST_IMG_UV_BASE           0x0000003C

/* DstImgSize  0x0040
   DST_HSIZE 12:0 rw Destination1 image horizontal stride in pixels.
   DST_VSIZE 27:16 rw Destination1 image vertical resolution in rows/lines
*/
#define DST_IMG_SIZE              0x00000040

/* DstImgStride 0x0044
   DST_STRIDE 15:0 rw Destination1 image horizontal resolution in pixels.
   DST_VSIZE 27:16 rw Destination1 image vertical resolution in rows/lines
*/
#define DST_IMG_STRIDE            0x00000044

/* DstImgYBase2 0x0048
   DST_Y_BASE 29:0 r Address of Y plane origin. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define DST_IMG_Y_BASE2           0x00000048

/* DstImgUVBase2 0x004C
   DST_UV_BASE 29:0 r Address of UV plane origin. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define DST_IMG_UV_BASE2          0x0000004C

/* DstImgSize2   0x0050
   DST_HSIZE 12:0 rw Destination1 image horizontal stride in pixels.
   DST_VSIZE 27:16 rw Destination1 image vertical resolution in rows/lines
*/
#define DST_IMG_SIZE2             0x00000050

/* DstImgStride2 0x0054
   DST_STRIDE 15:0 rw Destination1 image horizontal resolution in pixels.
   DST_VSIZE 27:16 rw Destination1 image vertical resolution in rows/lines
*/
#define DST_IMG_STRIDE2           0x00000054

/* DstImgYSize1 0x0080
   DST_IMG_Y_SIZE1 29:0 rw Destination2 image Y ring buffer size. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define DST_IMG_Y_SIZE1           0x00000080

/* DstImgUVSize1 0x0084
   DST_IMG_UV_SIZE1 29:0 rw Destination2 image UV ring buffer size. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define DST_IMG_UV_SIZE1          0x00000084

/* DstImgYSize2   0x0088
   DST_IMG_Y_SIZE2 29:0 rw Destination2 image Y ring buffer size. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define DST_IMG_Y_SIZE2           0x00000088

/* DstImgUVSize2  0x008C
   DST_IMG_UV_SIZE2 29:0 rw Destination2 image UV ring buffer size. Upper 30 bits of 34 bit address; the lower 4 bits are zero.
*/
#define DST_IMG_UV_SIZE2          0x0000008C

#endif // _DWE_REGS_H_
