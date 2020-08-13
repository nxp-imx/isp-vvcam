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
#ifndef _ISP_TYPES_H_
#define _ISP_TYPES_H_

enum {
	IC_MI_PATH_INVALID = -1,		   /**< lower border (only for an internal evaluation) */
	IC_MI_PATH_MAIN = 0,			   /**< main path index */
	IC_MI_PATH_SELF = 1,			   /**< self path index */
	IC_MI_PATH_SELF2 = 2,				   /**< self path index */
	IC_MI_PATH_MAX						 /**< upper border (only for an internal evaluation) */
};

enum {
	IC_MI_DATAMODE_INVALID = 0,		/**< lower border (only for an internal evaluation) */
	IC_MI_DATAMODE_DISABLED = 1,		/**< disables the path */
	IC_MI_DATAMODE_JPEG = 2,			/**< data output format is JPEG (only valid for mainpath @ref CamerIcMiPath_e) */
	IC_MI_DATAMODE_YUV444 = 3,		/**< data output format is YUV444 */
	IC_MI_DATAMODE_YUV422 = 4,		/**< data output format is YUV422 */
	IC_MI_DATAMODE_YUV420 = 5,		/**< data output format is YUV420 */
	IC_MI_DATAMODE_YUV400 = 6,		/**< data output format is YUV400 */
	IC_MI_DATAMODE_RGB888 = 7,		/**< data output format is RGB888 (only valid for selpath @ref CamerIcMiPath_e) */
	IC_MI_DATAMODE_RGB666 = 8,		/**< data output format is RGB666 (only valid for selpath @ref CamerIcMiPath_e) */
	IC_MI_DATAMODE_RGB565 = 9,		/**< data output format is RGB565 (only valid for selpath @ref CamerIcMiPath_e) */
	IC_MI_DATAMODE_RAW8 = 10,			/**< data output format is RAW8 (only valid for mainpath @ref CamerIcMiPath_e) */
	IC_MI_DATAMODE_RAW12 = 11,		   /**< data output format is RAW12 (only valid for mainpath @ref CamerIcMiPath_e) */
	IC_MI_DATAMODE_DPCC = 12,			/**< path dumps out the current measured defect pixel table */
	IC_MI_DATAMODE_RAW10 = 13,		   /**< data output format is RAW10 (only valid for mainpath @ref CamerIcMiPath_e) */
	IC_MI_DATAMODE_RAW14 = 14,		   /**< data output format is RAW14 (only valid for mainpath @ref CamerIcMiPath_e) */
	IC_MI_DATAMODE_RAW16 = 15,		   /**< data output format is RAW16 (only valid for mainpath @ref CamerIcMiPath_e) */
	IC_MI_DATAMODE_MAX					 /**< upper border (only for an internal evaluation) */
};

enum {
	IC_MI_DATASTORAGE_INVALID = 0,		/**< lower border (only for an internal evaluation) */
	IC_MI_DATASTORAGE_PLANAR = 1,		/**< YUV values are packed together as: YYYY......, UVUVUVUV...... */
	IC_MI_DATASTORAGE_SEMIPLANAR = 2,	/**< YUV values are packed together as: YUV, YUV, YUV, ...... */
	IC_MI_DATASTORAGE_INTERLEAVED = 3,	/**< Y values for all pixels are put together, as well as U and V,
													 like: YYYYYY......, UUUUUUU......., VVVVVV...... */
	IC_MI_DATASTORAGE_MAX				  /**< upper border (only for an internal evaluation) */
};

enum {
	ISP_PICBUF_TYPE_INVALID = 0x00,
	ISP_PICBUF_TYPE_DATA = 0x08,	/* just some sequential data */
	ISP_PICBUF_TYPE_RAW8 = 0x10,
	ISP_PICBUF_TYPE_RAW16 = 0x11,	/* includes: 9..16bits, MSBit aligned, LSByte first! */
	ISP_PICBUF_TYPE_RAW10 = 0x12,	/* includes: 10bits, MSBit aligned, LSByte first! */
	ISP_PICBUF_TYPE_RAW12 = 0x13,
	ISP_PICBUF_TYPE_RAW14 = 0x14,
	ISP_PICBUF_TYPE_JPEG = 0x20,
	ISP_PICBUF_TYPE_YCbCr444 = 0x30,
	ISP_PICBUF_TYPE_YCbCr422 = 0x31,
	ISP_PICBUF_TYPE_YCbCr420 = 0x32,
	ISP_PICBUF_TYPE_YCbCr400 = 0x33,
	ISP_PICBUF_TYPE_YCbCr32 = 0x3f,
	ISP_PICBUF_TYPE_RGB888 = 0x40,
	ISP_PICBUF_TYPE_RGB666 = 0x41,	/* R, G & B are LSBit aligned! */
	ISP_PICBUF_TYPE_RGB565 = 0x42,	/* TODO: don't know the memory layout right now, investigate! */
	ISP_PICBUF_TYPE_RGB32 = 0x4f,
	_ISP_PICBUF_TYPE_DUMMY_
};

enum {
	ISP_PICBUF_LAYOUT_INVALID = 0,
	ISP_PICBUF_LAYOUT_COMBINED = 0x10,	/* ISP_PICBUF_TYPE_DATA:        Data: D0 D1 D2... */
	/* ISP_PICBUF_TYPE_RAW8:          Data: D0 D1 D2... */
	/* ISP_PICBUF_TYPE_RAW16/10:  Data: D0L D0H D1L D1H... */
	/* ISP_PICBUF_TYPE_JPEG:          Data: J0 J1 J2... */
	/* ISP_PICBUF_TYPE_YCbCr444:  Data: Y0 Cb0 Cr0 Y1 Cb1Cr1... */
	/* ISP_PICBUF_TYPE_YCbCr422:  Data: Y0 Cb0 Y1 Cr0 Y2 Cb1 Y3 Cr1... */
	/* ISP_PICBUF_TYPE_YCbCr32:   Data: Cr0 Cb0 Y0 A0 Cr1 Cb1 Y1 A1... */
	/* ISP_PICBUF_TYPE_RGB888:      Data: R0 G0 B0 R1 B2 G1... */
	/* ISP_PICBUF_TYPE_RGB666:      Data: {00,R0[5:0]} {00,G0[5:0]} {00,B0[5:0]} {00,R1[5:0]} {00,G2[5:0]} {00,B3[5:0]}... */
	/* ISP_PICBUF_TYPE_RGB565:      Data: {R0[4:0],G0[5:3]} {G0[2:0],B0[4:0]} {R1[4:0],G1[5:3]} {G1[2:0],B1[4:0]}... (is this correct?) */
	/* ISP_PICBUF_TYPE_RGB32:        Data: B0 G0 R0 A0 B1 G1 R1 A1... */
	ISP_PICBUF_LAYOUT_BAYER_RGRGGBGB = 0x11,	/* 1st line: RGRG... , 2nd line GBGB... , etc. */
	ISP_PICBUF_LAYOUT_BAYER_GRGRBGBG = 0x12,	/* 1st line: GRGR... , 2nd line BGBG... , etc. */
	ISP_PICBUF_LAYOUT_BAYER_GBGBRGRG = 0x13,	/* 1st line: GBGB... , 2nd line RGRG... , etc. */
	ISP_PICBUF_LAYOUT_BAYER_BGBGGRGR = 0x14,	/* 1st line: BGBG... , 2nd line GRGR... , etc. */
	ISP_PICBUF_LAYOUT_SEMIPLANAR = 0x20,	/* ISP_PICBUF_TYPE_YCbCr422:  Luma:  Y0 Y1 Y2 Y3... ; Chroma: Cb0 Cr0 Cb1 Cr1... */
	/* ISP_PICBUF_TYPE_YCbCr420:  Luma:  Y0 Y1 Y2 Y3... ; Chroma: Cb0 Cr0 Cb1 Cr1... */
	/* ISP_PICBUF_TYPE_YCbCr400:  Luma:  Y0 Y1 Y2 Y3... ; Chroma: not used */
	ISP_PICBUF_LAYOUT_PLANAR = 0x30,	/* ISP_PICBUF_TYPE_YCbCr444:  Y: Y0 Y1 Y2 Y3...;  Cb: Cb0 Cb1 Cb2 Cb3...; Cr: Cr0 Cr1 Cr2 Cr3... */
	/* ISP_PICBUF_TYPE_YCbCr422:  Y: Y0 Y1 Y2 Y3...;  Cb: Cb0 Cb1 Cb2 Cb3...; Cr: Cr0 Cr1 Cr2 Cr3... */
	/* ISP_PICBUF_TYPE_YCbCr420:  Y: Y0 Y1 Y2 Y3...;  Cb: Cb0 Cb1 Cb2 Cb3...; Cr: Cr0 Cr1 Cr2 Cr3... */
	/* ISP_PICBUF_TYPE_YCbCr400:  Y: Y0 Y1 Y2 Y3...;  Cb: not used;            Cr: not used... */
	/* ISP_PICBUF_TYPE_RGB888:      R: R0 R1 R2 R3...;  G:  G0 G1 G2 G3...;  B:  B0 B1 B2 B3... */
	/* ISP_PICBUF_TYPE_RGB666:      R: {00,R0[5:0]}...; G:  {00,G0[5:0]}...;        B:  {00,B0[5:0]}... */
	_ISP_PICBUF_LAYOUT_DUMMY_
};

enum {
	IC_DPF_RB_FILTERSIZE_INVALID = 0,	/**< lower border (only for an internal evaluation) */
	IC_DPF_RB_FILTERSIZE_9x9 = 1,		/**< red and blue filter kernel size 9x9 (means 5x5 active pixel) */
	IC_DPF_RB_FILTERSIZE_13x9 = 2,		/**< red and blue filter kernel size 13x9 (means 7x5 active pixel) */
	IC_DPF_RB_FILTERSIZE_MAX			   /**< upper border (only for an internal evaluation) */
};

enum {
	ISP_MI_DATA_ALIGN_MODE_INVALID = -1,
	ISP_MI_DATA_UNALIGN_MODE = 0,	/* pixel data not aligned. */
	ISP_MI_DATA_ALIGN_128BIT_MODE = 1,	/* pixel data  aligned with 128 bit. */
	ISP_MI_DATA_ALIGN_16BIT_MODE = 2,	/* pixel data  aligned with 16 bit. */
	ISP_MI_DATA_ALIGN_MODE_MAX
};

enum {
	IC_DPF_GAIN_USAGE_INVALID = 0,		  /**< lower border (only for an internal evaluation) */
	IC_DPF_GAIN_USAGE_DISABLED = 1,		 /**< don't use any gains in preprocessing stage */
	IC_DPF_GAIN_USAGE_NF_GAINS = 2,		 /**< use only the noise function gains  from registers DPF_NF_GAIN_R, ... */
	IC_DPF_GAIN_USAGE_LSC_GAINS = 3,	/**< use only the gains from LSC module */
	IC_DPF_GAIN_USAGE_NF_LSC_GAINS = 4,    /**< use the moise function gains and the gains from LSC module */
	IC_DPF_GAIN_USAGE_AWB_GAINS = 5,	/**< use only the gains from AWB module */
	IC_DPF_GAIN_USAGE_AWB_LSC_GAINS = 6,   /**< use the gains from AWB and LSC module */
	IC_DPF_GAIN_USAGE_MAX				  /**< upper border (only for an internal evaluation) */
};

enum {
	IC_NLL_SCALE_INVALID = 0,		/**< lower border (only for an internal evaluation) */
	IC_NLL_SCALE_LINEAR = 1,			/**< use a linear scaling */
	IC_NLL_SCALE_LOGARITHMIC = 2,		/**< use a logarithmic scaling */
	IC_NLL_SCALE_MAX					   /**< upper border (only for an internal evaluation) */
};

#endif /* _ISP_TYPES_H_ */
