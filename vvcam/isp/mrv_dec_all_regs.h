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

/**
*-----------------------------------------------------------------------------
* $HeadURL$
* $Author$
* $Rev$
* $Date$
*-----------------------------------------------------------------------------
* @file mrv_dec_all_regs.h
*
* <pre>
*
* Description:
*   This header file exports the module register structure and masks.
*   it should not be included directly by your driver/application, it will be
*   exported by the <TOP>_regs_io.h header file.
*
* </pre>
*/
/*****************************************************************************/

#ifndef _MRV_DEC_ALL_REGS_H
#define _MRV_DEC_ALL_REGS_H

/*! Mrv Dec All Register layout */
typedef struct {
	uint32_t _notused_0[(0x24 - 0x00) / 4];
	uint32_t isp_dec_chipRev;	/*!<(r) */
	uint32_t isp_dec_chipDate;	/*!<(r) */
	uint32_t _notused_1[(0x98 - 0x2c) / 4];
	uint32_t isp_dec_hichipPatchRev;	/*!<(r) */
	uint32_t _notused_2[(0xA8 - 0x9c) / 4];
	uint32_t isp_dec_productId;	/*!<(r) */

	uint32_t _notused_3[(0x800 - 0xAc) / 4];
	uint32_t isp_dec_ctrl;	/*!<(rw), DEC_BASE + 0x00000000 */
	uint32_t isp_dec_ctrl_ex;	/*!<(rw), DEC_BASE + 0x00000004 */
	uint32_t isp_dec_ctrl_ex2;	/*!<(rw), DEC_BASE + 0x00000008 */
	uint32_t isp_dec_intr_enbl;	/*!<(rw), DEC_BASE + 0x0000000C */
	uint32_t isp_dec_intr_enbl_ex;	/*!<(rw), DEC_BASE + 0x00000010 */
	uint32_t isp_dec_intr_enbl_ex2;	/*!<(rw), DEC_BASE + 0x00000014 */
	uint32_t isp_dec_intr_acknowledge;	/*!<(ro), DEC_BASE + 0x00000018 */
	uint32_t isp_dec_intr_acknowledge_ex;	/*!<(ro), DEC_BASE + 0x0000001C */
	uint32_t isp_dec_intr_acknowledge_ex2;	/*!<(ro), DEC_BASE + 0x00000020 */
	uint32_t isp_dec_tile_status_debug;	/*!<(ro), DEC_BASE + 0x00000024 */
	uint32_t isp_dec_encoder_debug;	/*!<(ro), DEC_BASE + 0x00000028 */
	uint32_t isp_dec_decoder_debug;	/*!<(ro), DEC_BASE + 0x0000002C */
	uint32_t isp_dec_total_reads_in;	/*!<(ro), DEC_BASE + 0x00000030 */
	uint32_t isp_dec_total_writes_in;	/*!<(ro), DEC_BASE + 0x00000034 */
	uint32_t isp_dec_total_read_bursts_in;	/*!<(ro), DEC_BASE + 0x00000038 */
	uint32_t isp_dec_total_write_bursts_in;	/*!<(ro), DEC_BASE + 0x0000003C */
	uint32_t isp_dec_total_read_reqs_in;	/*!<(ro), DEC_BASE + 0x00000040 */
	uint32_t isp_dec_total_write_reqs_in;	/*!<(ro), DEC_BASE + 0x00000044 */
	uint32_t isp_dec_total_read_lasts_in;	/*!<(ro), DEC_BASE + 0x00000048 */
	uint32_t isp_dec_total_write_lasts_in;	/*!<(ro), DEC_BASE + 0x0000004C */
	uint32_t isp_dec_total_write_response_in;	/*!<(ro), DEC_BASE + 0x00000050 */
	uint32_t isp_dec_total_reads_out;	/*!<(ro), DEC_BASE + 0x00000054 */
	uint32_t isp_dec_total_writes_out;	/*!<(ro), DEC_BASE + 0x00000058 */
	uint32_t isp_dec_total_read_bursts_out;	/*!<(ro), DEC_BASE + 0x0000005C */
	uint32_t isp_dec_total_write_bursts_out;	/*!<(ro), DEC_BASE + 0x00000060 */
	uint32_t isp_dec_total_read_reqs_out;	/*!<(ro), DEC_BASE + 0x00000064 */
	uint32_t isp_dec_total_write_reqs_out;	/*!<(ro), DEC_BASE + 0x00000068 */
	uint32_t isp_dec_total_read_lasts_out;	/*!<(ro), DEC_BASE + 0x0000006C */
	uint32_t isp_dec_total_write_lasts_out;	/*!<(ro), DEC_BASE + 0x00000070 */
	uint32_t isp_dec_total_write_response_out;	/*!<(ro), DEC_BASE + 0x00000074 */
	uint32_t isp_dec_status;	/*!<(ro), DEC_BASE + 0x00000078 */
	uint32_t isp_dec_debug_info_select;	/*!<(rw), DEC_BASE + 0x0000007C */
	uint32_t isp_dec_read_config_0;	/*!<(rw), DEC_BASE + 0x00000080 */
	uint32_t isp_dec_read_config_1;	/*!<(rw), DEC_BASE + 0x00000084 */
	uint32_t isp_dec_read_config_2;	/*!<(rw), DEC_BASE + 0x00000088 */
	uint32_t isp_dec_read_config_3;	/*!<(rw), DEC_BASE + 0x0000008C */
	uint32_t isp_dec_read_config_4;	/*!<(rw), DEC_BASE + 0x00000090 */
	uint32_t isp_dec_read_config_5;	/*!<(rw), DEC_BASE + 0x00000094 */
	uint32_t isp_dec_read_config_6;	/*!<(rw), DEC_BASE + 0x00000098 */
	uint32_t isp_dec_read_config_7;	/*!<(rw), DEC_BASE + 0x0000009C */
	uint32_t isp_dec_read_config_8;	/*!<(rw), DEC_BASE + 0x000000A0 */
	uint32_t isp_dec_read_config_9;	/*!<(rw), DEC_BASE + 0x000000A4 */
	uint32_t isp_dec_read_config_10;	/*!<(rw), DEC_BASE + 0x000000A8 */
	uint32_t isp_dec_read_config_11;	/*!<(rw), DEC_BASE + 0x000000AC */
	uint32_t isp_dec_read_config_12;	/*!<(rw), DEC_BASE + 0x000000B0 */
	uint32_t isp_dec_read_config_13;	/*!<(rw), DEC_BASE + 0x000000B4 */
	uint32_t isp_dec_read_config_14;	/*!<(rw), DEC_BASE + 0x000000B8 */
	uint32_t isp_dec_read_config_15;	/*!<(rw), DEC_BASE + 0x000000BC */
	uint32_t isp_dec_read_config_16;	/*!<(rw), DEC_BASE + 0x000000C0 */
	uint32_t isp_dec_read_config_17;	/*!<(rw), DEC_BASE + 0x000000C4 */
	uint32_t isp_dec_read_config_18;	/*!<(rw), DEC_BASE + 0x000000C8 */
	uint32_t isp_dec_read_config_19;	/*!<(rw), DEC_BASE + 0x000000CC */
	uint32_t isp_dec_read_config_20;	/*!<(rw), DEC_BASE + 0x000000D0 */
	uint32_t isp_dec_read_config_21;	/*!<(rw), DEC_BASE + 0x000000D4 */
	uint32_t isp_dec_read_config_22;	/*!<(rw), DEC_BASE + 0x000000D8 */
	uint32_t isp_dec_read_config_23;	/*!<(rw), DEC_BASE + 0x000000DC */
	uint32_t isp_dec_read_config_24;	/*!<(rw), DEC_BASE + 0x000000E0 */
	uint32_t isp_dec_read_config_25;	/*!<(rw), DEC_BASE + 0x000000E4 */
	uint32_t isp_dec_read_config_26;	/*!<(rw), DEC_BASE + 0x000000E8 */
	uint32_t isp_dec_read_config_27;	/*!<(rw), DEC_BASE + 0x000000EC */
	uint32_t isp_dec_read_config_28;	/*!<(rw), DEC_BASE + 0x000000F0 */
	uint32_t isp_dec_read_config_29;	/*!<(rw), DEC_BASE + 0x000000F4 */
	uint32_t isp_dec_read_config_30;	/*!<(rw), DEC_BASE + 0x000000F8 */
	uint32_t isp_dec_read_config_31;	/*!<(rw), DEC_BASE + 0x000000FC */
	uint32_t isp_dec_read_ex_config_0;	/*!<(rw), DEC_BASE + 0x00000100 */
	uint32_t isp_dec_read_ex_config_1;	/*!<(rw), DEC_BASE + 0x00000104 */
	uint32_t isp_dec_read_ex_config_2;	/*!<(rw), DEC_BASE + 0x00000108 */
	uint32_t isp_dec_read_ex_config_3;	/*!<(rw), DEC_BASE + 0x0000010C */
	uint32_t isp_dec_read_ex_config_4;	/*!<(rw), DEC_BASE + 0x00000110 */
	uint32_t isp_dec_read_ex_config_5;	/*!<(rw), DEC_BASE + 0x00000114 */
	uint32_t isp_dec_read_ex_config_6;	/*!<(rw), DEC_BASE + 0x00000118 */
	uint32_t isp_dec_read_ex_config_7;	/*!<(rw), DEC_BASE + 0x0000011C */
	uint32_t isp_dec_read_ex_config_8;	/*!<(rw), DEC_BASE + 0x00000120 */
	uint32_t isp_dec_read_ex_config_9;	/*!<(rw), DEC_BASE + 0x00000124 */
	uint32_t isp_dec_read_ex_config_10;	/*!<(rw), DEC_BASE + 0x00000128 */
	uint32_t isp_dec_read_ex_config_11;	/*!<(rw), DEC_BASE + 0x0000012C */
	uint32_t isp_dec_read_ex_config_12;	/*!<(rw), DEC_BASE + 0x00000130 */
	uint32_t isp_dec_read_ex_config_13;	/*!<(rw), DEC_BASE + 0x00000134 */
	uint32_t isp_dec_read_ex_config_14;	/*!<(rw), DEC_BASE + 0x00000138 */
	uint32_t isp_dec_read_ex_config_15;	/*!<(rw), DEC_BASE + 0x0000013C */
	uint32_t isp_dec_read_ex_config_16;	/*!<(rw), DEC_BASE + 0x00000140 */
	uint32_t isp_dec_read_ex_config_17;	/*!<(rw), DEC_BASE + 0x00000144 */
	uint32_t isp_dec_read_ex_config_18;	/*!<(rw), DEC_BASE + 0x00000148 */
	uint32_t isp_dec_read_ex_config_19;	/*!<(rw), DEC_BASE + 0x0000014C */
	uint32_t isp_dec_read_ex_config_20;	/*!<(rw), DEC_BASE + 0x00000150 */
	uint32_t isp_dec_read_ex_config_21;	/*!<(rw), DEC_BASE + 0x00000154 */
	uint32_t isp_dec_read_ex_config_22;	/*!<(rw), DEC_BASE + 0x00000158 */
	uint32_t isp_dec_read_ex_config_23;	/*!<(rw), DEC_BASE + 0x0000015C */
	uint32_t isp_dec_read_ex_config_24;	/*!<(rw), DEC_BASE + 0x00000160 */
	uint32_t isp_dec_read_ex_config_25;	/*!<(rw), DEC_BASE + 0x00000164 */
	uint32_t isp_dec_read_ex_config_26;	/*!<(rw), DEC_BASE + 0x00000168 */
	uint32_t isp_dec_read_ex_config_27;	/*!<(rw), DEC_BASE + 0x0000016C */
	uint32_t isp_dec_read_ex_config_28;	/*!<(rw), DEC_BASE + 0x00000170 */
	uint32_t isp_dec_read_ex_config_29;	/*!<(rw), DEC_BASE + 0x00000174 */
	uint32_t isp_dec_read_ex_config_30;	/*!<(rw), DEC_BASE + 0x00000178 */
	uint32_t isp_dec_read_ex_config_31;	/*!<(rw), DEC_BASE + 0x0000017C */
	uint32_t isp_dec_write_config_0;	/*!<(rw), DEC_BASE + 0x00000180 */
	uint32_t isp_dec_write_config_1;	/*!<(rw), DEC_BASE + 0x00000184 */
	uint32_t isp_dec_write_config_2;	/*!<(rw), DEC_BASE + 0x00000188 */
	uint32_t isp_dec_write_config_3;	/*!<(rw), DEC_BASE + 0x0000018C */
	uint32_t isp_dec_write_config_4;	/*!<(rw), DEC_BASE + 0x00000190 */
	uint32_t isp_dec_write_config_5;	/*!<(rw), DEC_BASE + 0x00000194 */
	uint32_t isp_dec_write_config_6;	/*!<(rw), DEC_BASE + 0x00000198 */
	uint32_t isp_dec_write_config_7;	/*!<(rw), DEC_BASE + 0x0000019C */
	uint32_t isp_dec_write_config_8;	/*!<(rw), DEC_BASE + 0x000001A0 */
	uint32_t isp_dec_write_config_9;	/*!<(rw), DEC_BASE + 0x000001A4 */
	uint32_t isp_dec_write_config_10;	/*!<(rw), DEC_BASE + 0x000001A8 */
	uint32_t isp_dec_write_config_11;	/*!<(rw), DEC_BASE + 0x000001AC */
	uint32_t isp_dec_write_config_12;	/*!<(rw), DEC_BASE + 0x000001B0 */
	uint32_t isp_dec_write_config_13;	/*!<(rw), DEC_BASE + 0x000001B4 */
	uint32_t isp_dec_write_config_14;	/*!<(rw), DEC_BASE + 0x000001B8 */
	uint32_t isp_dec_write_config_15;	/*!<(rw), DEC_BASE + 0x000001BC */
	uint32_t isp_dec_write_config_16;	/*!<(rw), DEC_BASE + 0x000001C0 */
	uint32_t isp_dec_write_config_17;	/*!<(rw), DEC_BASE + 0x000001C4 */
	uint32_t isp_dec_write_config_18;	/*!<(rw), DEC_BASE + 0x000001C8 */
	uint32_t isp_dec_write_config_19;	/*!<(rw), DEC_BASE + 0x000001CC */
	uint32_t isp_dec_write_config_20;	/*!<(rw), DEC_BASE + 0x000001D0 */
	uint32_t isp_dec_write_config_21;	/*!<(rw), DEC_BASE + 0x000001D4 */
	uint32_t isp_dec_write_config_22;	/*!<(rw), DEC_BASE + 0x000001D8 */
	uint32_t isp_dec_write_config_23;	/*!<(rw), DEC_BASE + 0x000001DC */
	uint32_t isp_dec_write_config_24;	/*!<(rw), DEC_BASE + 0x000001E0 */
	uint32_t isp_dec_write_config_25;	/*!<(rw), DEC_BASE + 0x000001E4 */
	uint32_t isp_dec_write_config_26;	/*!<(rw), DEC_BASE + 0x000001E8 */
	uint32_t isp_dec_write_config_27;	/*!<(rw), DEC_BASE + 0x000001EC */
	uint32_t isp_dec_write_config_28;	/*!<(rw), DEC_BASE + 0x000001F0 */
	uint32_t isp_dec_write_config_29;	/*!<(rw), DEC_BASE + 0x000001F4 */
	uint32_t isp_dec_write_config_30;	/*!<(rw), DEC_BASE + 0x000001F8 */
	uint32_t isp_dec_write_config_31;	/*!<(rw), DEC_BASE + 0x000001FC */
	uint32_t isp_dec_write_ex_config_0;	/*!<(rw), DEC_BASE + 0x00000200 */
	uint32_t isp_dec_write_ex_config_1;	/*!<(rw), DEC_BASE + 0x00000204 */
	uint32_t isp_dec_write_ex_config_2;	/*!<(rw), DEC_BASE + 0x00000208 */
	uint32_t isp_dec_write_ex_config_3;	/*!<(rw), DEC_BASE + 0x0000020C */
	uint32_t isp_dec_write_ex_config_4;	/*!<(rw), DEC_BASE + 0x00000210 */
	uint32_t isp_dec_write_ex_config_5;	/*!<(rw), DEC_BASE + 0x00000214 */
	uint32_t isp_dec_write_ex_config_6;	/*!<(rw), DEC_BASE + 0x00000218 */
	uint32_t isp_dec_write_ex_config_7;	/*!<(rw), DEC_BASE + 0x0000021C */
	uint32_t isp_dec_write_ex_config_8;	/*!<(rw), DEC_BASE + 0x00000220 */
	uint32_t isp_dec_write_ex_config_9;	/*!<(rw), DEC_BASE + 0x00000224 */
	uint32_t isp_dec_write_ex_config_10;	/*!<(rw), DEC_BASE + 0x00000228 */
	uint32_t isp_dec_write_ex_config_11;	/*!<(rw), DEC_BASE + 0x0000022C */
	uint32_t isp_dec_write_ex_config_12;	/*!<(rw), DEC_BASE + 0x00000230 */
	uint32_t isp_dec_write_ex_config_13;	/*!<(rw), DEC_BASE + 0x00000234 */
	uint32_t isp_dec_write_ex_config_14;	/*!<(rw), DEC_BASE + 0x00000238 */
	uint32_t isp_dec_write_ex_config_15;	/*!<(rw), DEC_BASE + 0x0000023C */
	uint32_t isp_dec_write_ex_config_16;	/*!<(rw), DEC_BASE + 0x00000240 */
	uint32_t isp_dec_write_ex_config_17;	/*!<(rw), DEC_BASE + 0x00000244 */
	uint32_t isp_dec_write_ex_config_18;	/*!<(rw), DEC_BASE + 0x00000248 */
	uint32_t isp_dec_write_ex_config_19;	/*!<(rw), DEC_BASE + 0x0000024C */
	uint32_t isp_dec_write_ex_config_20;	/*!<(rw), DEC_BASE + 0x00000250 */
	uint32_t isp_dec_write_ex_config_21;	/*!<(rw), DEC_BASE + 0x00000254 */
	uint32_t isp_dec_write_ex_config_22;	/*!<(rw), DEC_BASE + 0x00000258 */
	uint32_t isp_dec_write_ex_config_23;	/*!<(rw), DEC_BASE + 0x0000025C */
	uint32_t isp_dec_write_ex_config_24;	/*!<(rw), DEC_BASE + 0x00000260 */
	uint32_t isp_dec_write_ex_config_25;	/*!<(rw), DEC_BASE + 0x00000264 */
	uint32_t isp_dec_write_ex_config_26;	/*!<(rw), DEC_BASE + 0x00000268 */
	uint32_t isp_dec_write_ex_config_27;	/*!<(rw), DEC_BASE + 0x0000026C */
	uint32_t isp_dec_write_ex_config_28;	/*!<(rw), DEC_BASE + 0x00000270 */
	uint32_t isp_dec_write_ex_config_29;	/*!<(rw), DEC_BASE + 0x00000274 */
	uint32_t isp_dec_write_ex_config_30;	/*!<(rw), DEC_BASE + 0x00000278 */
	uint32_t isp_dec_write_ex_config_31;	/*!<(rw), DEC_BASE + 0x0000027C */
	uint32_t isp_dec_read_buffer_base_0;	/*!<(rw), DEC_BASE + 0x00000280 */
	uint32_t isp_dec_read_buffer_base_1;	/*!<(rw), DEC_BASE + 0x00000284 */
	uint32_t isp_dec_read_buffer_base_2;	/*!<(rw), DEC_BASE + 0x00000288 */
	uint32_t isp_dec_read_buffer_base_3;	/*!<(rw), DEC_BASE + 0x0000028C */
	uint32_t isp_dec_read_buffer_base_4;	/*!<(rw), DEC_BASE + 0x00000290 */
	uint32_t isp_dec_read_buffer_base_5;	/*!<(rw), DEC_BASE + 0x00000294 */
	uint32_t isp_dec_read_buffer_base_6;	/*!<(rw), DEC_BASE + 0x00000298 */
	uint32_t isp_dec_read_buffer_base_7;	/*!<(rw), DEC_BASE + 0x0000029C */
	uint32_t isp_dec_read_buffer_base_8;	/*!<(rw), DEC_BASE + 0x000002A0 */
	uint32_t isp_dec_read_buffer_base_9;	/*!<(rw), DEC_BASE + 0x000002A4 */
	uint32_t isp_dec_read_buffer_base_10;	/*!<(rw), DEC_BASE + 0x000002A8 */
	uint32_t isp_dec_read_buffer_base_11;	/*!<(rw), DEC_BASE + 0x000002AC */
	uint32_t isp_dec_read_buffer_base_12;	/*!<(rw), DEC_BASE + 0x000002B0 */
	uint32_t isp_dec_read_buffer_base_13;	/*!<(rw), DEC_BASE + 0x000002B4 */
	uint32_t isp_dec_read_buffer_base_14;	/*!<(rw), DEC_BASE + 0x000002B8 */
	uint32_t isp_dec_read_buffer_base_15;	/*!<(rw), DEC_BASE + 0x000002BC */
	uint32_t isp_dec_read_buffer_base_16;	/*!<(rw), DEC_BASE + 0x000002C0 */
	uint32_t isp_dec_read_buffer_base_17;	/*!<(rw), DEC_BASE + 0x000002C4 */
	uint32_t isp_dec_read_buffer_base_18;	/*!<(rw), DEC_BASE + 0x000002C8 */
	uint32_t isp_dec_read_buffer_base_19;	/*!<(rw), DEC_BASE + 0x000002CC */
	uint32_t isp_dec_read_buffer_base_20;	/*!<(rw), DEC_BASE + 0x000002D0 */
	uint32_t isp_dec_read_buffer_base_21;	/*!<(rw), DEC_BASE + 0x000002D4 */
	uint32_t isp_dec_read_buffer_base_22;	/*!<(rw), DEC_BASE + 0x000002D8 */
	uint32_t isp_dec_read_buffer_base_23;	/*!<(rw), DEC_BASE + 0x000002DC */
	uint32_t isp_dec_read_buffer_base_24;	/*!<(rw), DEC_BASE + 0x000002E0 */
	uint32_t isp_dec_read_buffer_base_25;	/*!<(rw), DEC_BASE + 0x000002E4 */
	uint32_t isp_dec_read_buffer_base_26;	/*!<(rw), DEC_BASE + 0x000002E8 */
	uint32_t isp_dec_read_buffer_base_27;	/*!<(rw), DEC_BASE + 0x000002EC */
	uint32_t isp_dec_read_buffer_base_28;	/*!<(rw), DEC_BASE + 0x000002F0 */
	uint32_t isp_dec_read_buffer_base_29;	/*!<(rw), DEC_BASE + 0x000002F4 */
	uint32_t isp_dec_read_buffer_base_30;	/*!<(rw), DEC_BASE + 0x000002F8 */
	uint32_t isp_dec_read_buffer_base_31;	/*!<(rw), DEC_BASE + 0x000002FC */
	uint32_t isp_dec_read_buffer_base_ex_0;	/*!<(rw), DEC_BASE + 0x00000300 */
	uint32_t isp_dec_read_buffer_base_ex_1;	/*!<(rw), DEC_BASE + 0x00000304 */
	uint32_t isp_dec_read_buffer_base_ex_2;	/*!<(rw), DEC_BASE + 0x00000308 */
	uint32_t isp_dec_read_buffer_base_ex_3;	/*!<(rw), DEC_BASE + 0x0000030C */
	uint32_t isp_dec_read_buffer_base_ex_4;	/*!<(rw), DEC_BASE + 0x00000310 */
	uint32_t isp_dec_read_buffer_base_ex_5;	/*!<(rw), DEC_BASE + 0x00000314 */
	uint32_t isp_dec_read_buffer_base_ex_6;	/*!<(rw), DEC_BASE + 0x00000318 */
	uint32_t isp_dec_read_buffer_base_ex_7;	/*!<(rw), DEC_BASE + 0x0000031C */
	uint32_t isp_dec_read_buffer_base_ex_8;	/*!<(rw), DEC_BASE + 0x00000320 */
	uint32_t isp_dec_read_buffer_base_ex_9;	/*!<(rw), DEC_BASE + 0x00000324 */
	uint32_t isp_dec_read_buffer_base_ex_10;	/*!<(rw), DEC_BASE + 0x00000328 */
	uint32_t isp_dec_read_buffer_base_ex_11;	/*!<(rw), DEC_BASE + 0x0000032C */
	uint32_t isp_dec_read_buffer_base_ex_12;	/*!<(rw), DEC_BASE + 0x00000330 */
	uint32_t isp_dec_read_buffer_base_ex_13;	/*!<(rw), DEC_BASE + 0x00000334 */
	uint32_t isp_dec_read_buffer_base_ex_14;	/*!<(rw), DEC_BASE + 0x00000338 */
	uint32_t isp_dec_read_buffer_base_ex_15;	/*!<(rw), DEC_BASE + 0x0000033C */
	uint32_t isp_dec_read_buffer_base_ex_16;	/*!<(rw), DEC_BASE + 0x00000340 */
	uint32_t isp_dec_read_buffer_base_ex_17;	/*!<(rw), DEC_BASE + 0x00000344 */
	uint32_t isp_dec_read_buffer_base_ex_18;	/*!<(rw), DEC_BASE + 0x00000348 */
	uint32_t isp_dec_read_buffer_base_ex_19;	/*!<(rw), DEC_BASE + 0x0000034C */
	uint32_t isp_dec_read_buffer_base_ex_20;	/*!<(rw), DEC_BASE + 0x00000350 */
	uint32_t isp_dec_read_buffer_base_ex_21;	/*!<(rw), DEC_BASE + 0x00000354 */
	uint32_t isp_dec_read_buffer_base_ex_22;	/*!<(rw), DEC_BASE + 0x00000358 */
	uint32_t isp_dec_read_buffer_base_ex_23;	/*!<(rw), DEC_BASE + 0x0000035C */
	uint32_t isp_dec_read_buffer_base_ex_24;	/*!<(rw), DEC_BASE + 0x00000360 */
	uint32_t isp_dec_read_buffer_base_ex_25;	/*!<(rw), DEC_BASE + 0x00000364 */
	uint32_t isp_dec_read_buffer_base_ex_26;	/*!<(rw), DEC_BASE + 0x00000368 */
	uint32_t isp_dec_read_buffer_base_ex_27;	/*!<(rw), DEC_BASE + 0x0000036C */
	uint32_t isp_dec_read_buffer_base_ex_28;	/*!<(rw), DEC_BASE + 0x00000370 */
	uint32_t isp_dec_read_buffer_base_ex_29;	/*!<(rw), DEC_BASE + 0x00000374 */
	uint32_t isp_dec_read_buffer_base_ex_30;	/*!<(rw), DEC_BASE + 0x00000378 */
	uint32_t isp_dec_read_buffer_base_ex_31;	/*!<(rw), DEC_BASE + 0x0000037C */
	uint32_t isp_dec_read_buffer_end_0;	/*!<(rw), DEC_BASE + 0x00000380 */
	uint32_t isp_dec_read_buffer_end_1;	/*!<(rw), DEC_BASE + 0x00000384 */
	uint32_t isp_dec_read_buffer_end_2;	/*!<(rw), DEC_BASE + 0x00000388 */
	uint32_t isp_dec_read_buffer_end_3;	/*!<(rw), DEC_BASE + 0x0000038C */
	uint32_t isp_dec_read_buffer_end_4;	/*!<(rw), DEC_BASE + 0x00000390 */
	uint32_t isp_dec_read_buffer_end_5;	/*!<(rw), DEC_BASE + 0x00000394 */
	uint32_t isp_dec_read_buffer_end_6;	/*!<(rw), DEC_BASE + 0x00000398 */
	uint32_t isp_dec_read_buffer_end_7;	/*!<(rw), DEC_BASE + 0x0000039C */
	uint32_t isp_dec_read_buffer_end_8;	/*!<(rw), DEC_BASE + 0x000003A0 */
	uint32_t isp_dec_read_buffer_end_9;	/*!<(rw), DEC_BASE + 0x000003A4 */
	uint32_t isp_dec_read_buffer_end_10;	/*!<(rw), DEC_BASE + 0x000003A8 */
	uint32_t isp_dec_read_buffer_end_11;	/*!<(rw), DEC_BASE + 0x000003AC */
	uint32_t isp_dec_read_buffer_end_12;	/*!<(rw), DEC_BASE + 0x000003B0 */
	uint32_t isp_dec_read_buffer_end_13;	/*!<(rw), DEC_BASE + 0x000003B4 */
	uint32_t isp_dec_read_buffer_end_14;	/*!<(rw), DEC_BASE + 0x000003B8 */
	uint32_t isp_dec_read_buffer_end_15;	/*!<(rw), DEC_BASE + 0x000003BC */
	uint32_t isp_dec_read_buffer_end_16;	/*!<(rw), DEC_BASE + 0x000003C0 */
	uint32_t isp_dec_read_buffer_end_17;	/*!<(rw), DEC_BASE + 0x000003C4 */
	uint32_t isp_dec_read_buffer_end_18;	/*!<(rw), DEC_BASE + 0x000003C8 */
	uint32_t isp_dec_read_buffer_end_19;	/*!<(rw), DEC_BASE + 0x000003CC */
	uint32_t isp_dec_read_buffer_end_20;	/*!<(rw), DEC_BASE + 0x000003D0 */
	uint32_t isp_dec_read_buffer_end_21;	/*!<(rw), DEC_BASE + 0x000003D4 */
	uint32_t isp_dec_read_buffer_end_22;	/*!<(rw), DEC_BASE + 0x000003D8 */
	uint32_t isp_dec_read_buffer_end_23;	/*!<(rw), DEC_BASE + 0x000003DC */
	uint32_t isp_dec_read_buffer_end_24;	/*!<(rw), DEC_BASE + 0x000003E0 */
	uint32_t isp_dec_read_buffer_end_25;	/*!<(rw), DEC_BASE + 0x000003E4 */
	uint32_t isp_dec_read_buffer_end_26;	/*!<(rw), DEC_BASE + 0x000003E8 */
	uint32_t isp_dec_read_buffer_end_27;	/*!<(rw), DEC_BASE + 0x000003EC */
	uint32_t isp_dec_read_buffer_end_28;	/*!<(rw), DEC_BASE + 0x000003F0 */
	uint32_t isp_dec_read_buffer_end_29;	/*!<(rw), DEC_BASE + 0x000003F4 */
	uint32_t isp_dec_read_buffer_end_30;	/*!<(rw), DEC_BASE + 0x000003F8 */
	uint32_t isp_dec_read_buffer_end_31;	/*!<(rw), DEC_BASE + 0x000003FC */
	uint32_t isp_dec_read_buffer_end_ex_0;	/*!<(rw), DEC_BASE + 0x00000400 */
	uint32_t isp_dec_read_buffer_end_ex_1;	/*!<(rw), DEC_BASE + 0x00000404 */
	uint32_t isp_dec_read_buffer_end_ex_2;	/*!<(rw), DEC_BASE + 0x00000408 */
	uint32_t isp_dec_read_buffer_end_ex_3;	/*!<(rw), DEC_BASE + 0x0000040C */
	uint32_t isp_dec_read_buffer_end_ex_4;	/*!<(rw), DEC_BASE + 0x00000410 */
	uint32_t isp_dec_read_buffer_end_ex_5;	/*!<(rw), DEC_BASE + 0x00000414 */
	uint32_t isp_dec_read_buffer_end_ex_6;	/*!<(rw), DEC_BASE + 0x00000418 */
	uint32_t isp_dec_read_buffer_end_ex_7;	/*!<(rw), DEC_BASE + 0x0000041C */
	uint32_t isp_dec_read_buffer_end_ex_8;	/*!<(rw), DEC_BASE + 0x00000420 */
	uint32_t isp_dec_read_buffer_end_ex_9;	/*!<(rw), DEC_BASE + 0x00000424 */
	uint32_t isp_dec_read_buffer_end_ex_10;	/*!<(rw), DEC_BASE + 0x00000428 */
	uint32_t isp_dec_read_buffer_end_ex_11;	/*!<(rw), DEC_BASE + 0x0000042C */
	uint32_t isp_dec_read_buffer_end_ex_12;	/*!<(rw), DEC_BASE + 0x00000430 */
	uint32_t isp_dec_read_buffer_end_ex_13;	/*!<(rw), DEC_BASE + 0x00000434 */
	uint32_t isp_dec_read_buffer_end_ex_14;	/*!<(rw), DEC_BASE + 0x00000438 */
	uint32_t isp_dec_read_buffer_end_ex_15;	/*!<(rw), DEC_BASE + 0x0000043C */
	uint32_t isp_dec_read_buffer_end_ex_16;	/*!<(rw), DEC_BASE + 0x00000440 */
	uint32_t isp_dec_read_buffer_end_ex_17;	/*!<(rw), DEC_BASE + 0x00000444 */
	uint32_t isp_dec_read_buffer_end_ex_18;	/*!<(rw), DEC_BASE + 0x00000448 */
	uint32_t isp_dec_read_buffer_end_ex_19;	/*!<(rw), DEC_BASE + 0x0000044C */
	uint32_t isp_dec_read_buffer_end_ex_20;	/*!<(rw), DEC_BASE + 0x00000450 */
	uint32_t isp_dec_read_buffer_end_ex_21;	/*!<(rw), DEC_BASE + 0x00000454 */
	uint32_t isp_dec_read_buffer_end_ex_22;	/*!<(rw), DEC_BASE + 0x00000458 */
	uint32_t isp_dec_read_buffer_end_ex_23;	/*!<(rw), DEC_BASE + 0x0000045C */
	uint32_t isp_dec_read_buffer_end_ex_24;	/*!<(rw), DEC_BASE + 0x00000460 */
	uint32_t isp_dec_read_buffer_end_ex_25;	/*!<(rw), DEC_BASE + 0x00000464 */
	uint32_t isp_dec_read_buffer_end_ex_26;	/*!<(rw), DEC_BASE + 0x00000468 */
	uint32_t isp_dec_read_buffer_end_ex_27;	/*!<(rw), DEC_BASE + 0x0000046C */
	uint32_t isp_dec_read_buffer_end_ex_28;	/*!<(rw), DEC_BASE + 0x00000470 */
	uint32_t isp_dec_read_buffer_end_ex_29;	/*!<(rw), DEC_BASE + 0x00000474 */
	uint32_t isp_dec_read_buffer_end_ex_30;	/*!<(rw), DEC_BASE + 0x00000478 */
	uint32_t isp_dec_read_buffer_end_ex_31;	/*!<(rw), DEC_BASE + 0x0000047C */
	uint32_t isp_dec_read_flush_cache_0;	/*!<(rw), DEC_BASE + 0x00000480 */
	uint32_t isp_dec_read_flush_cache_1;	/*!<(rw), DEC_BASE + 0x00000484 */
	uint32_t isp_dec_read_flush_cache_2;	/*!<(rw), DEC_BASE + 0x00000488 */
	uint32_t isp_dec_read_flush_cache_3;	/*!<(rw), DEC_BASE + 0x0000048C */
	uint32_t isp_dec_read_flush_cache_4;	/*!<(rw), DEC_BASE + 0x00000490 */
	uint32_t isp_dec_read_flush_cache_5;	/*!<(rw), DEC_BASE + 0x00000494 */
	uint32_t isp_dec_read_flush_cache_6;	/*!<(rw), DEC_BASE + 0x00000498 */
	uint32_t isp_dec_read_flush_cache_7;	/*!<(rw), DEC_BASE + 0x0000049C */
	uint32_t isp_dec_read_flush_cache_8;	/*!<(rw), DEC_BASE + 0x000004A0 */
	uint32_t isp_dec_read_flush_cache_9;	/*!<(rw), DEC_BASE + 0x000004A4 */
	uint32_t isp_dec_read_flush_cache_10;	/*!<(rw), DEC_BASE + 0x000004A8 */
	uint32_t isp_dec_read_flush_cache_11;	/*!<(rw), DEC_BASE + 0x000004AC */
	uint32_t isp_dec_read_flush_cache_12;	/*!<(rw), DEC_BASE + 0x000004B0 */
	uint32_t isp_dec_read_flush_cache_13;	/*!<(rw), DEC_BASE + 0x000004B4 */
	uint32_t isp_dec_read_flush_cache_14;	/*!<(rw), DEC_BASE + 0x000004B8 */
	uint32_t isp_dec_read_flush_cache_15;	/*!<(rw), DEC_BASE + 0x000004BC */
	uint32_t isp_dec_read_flush_cache_16;	/*!<(rw), DEC_BASE + 0x000004C0 */
	uint32_t isp_dec_read_flush_cache_17;	/*!<(rw), DEC_BASE + 0x000004C4 */
	uint32_t isp_dec_read_flush_cache_18;	/*!<(rw), DEC_BASE + 0x000004C8 */
	uint32_t isp_dec_read_flush_cache_19;	/*!<(rw), DEC_BASE + 0x000004CC */
	uint32_t isp_dec_read_flush_cache_20;	/*!<(rw), DEC_BASE + 0x000004D0 */
	uint32_t isp_dec_read_flush_cache_21;	/*!<(rw), DEC_BASE + 0x000004D4 */
	uint32_t isp_dec_read_flush_cache_22;	/*!<(rw), DEC_BASE + 0x000004D8 */
	uint32_t isp_dec_read_flush_cache_23;	/*!<(rw), DEC_BASE + 0x000004DC */
	uint32_t isp_dec_read_flush_cache_24;	/*!<(rw), DEC_BASE + 0x000004E0 */
	uint32_t isp_dec_read_flush_cache_25;	/*!<(rw), DEC_BASE + 0x000004E4 */
	uint32_t isp_dec_read_flush_cache_26;	/*!<(rw), DEC_BASE + 0x000004E8 */
	uint32_t isp_dec_read_flush_cache_27;	/*!<(rw), DEC_BASE + 0x000004EC */
	uint32_t isp_dec_read_flush_cache_28;	/*!<(rw), DEC_BASE + 0x000004F0 */
	uint32_t isp_dec_read_flush_cache_29;	/*!<(rw), DEC_BASE + 0x000004F4 */
	uint32_t isp_dec_read_flush_cache_30;	/*!<(rw), DEC_BASE + 0x000004F8 */
	uint32_t isp_dec_read_flush_cache_31;	/*!<(rw), DEC_BASE + 0x000004FC */
	uint32_t isp_dec_read_flush_cache_ex_0;	/*!<(rw), DEC_BASE + 0x00000500 */
	uint32_t isp_dec_read_flush_cache_ex_1;	/*!<(rw), DEC_BASE + 0x00000504 */
	uint32_t isp_dec_read_flush_cache_ex_2;	/*!<(rw), DEC_BASE + 0x00000508 */
	uint32_t isp_dec_read_flush_cache_ex_3;	/*!<(rw), DEC_BASE + 0x0000050C */
	uint32_t isp_dec_read_flush_cache_ex_4;	/*!<(rw), DEC_BASE + 0x00000510 */
	uint32_t isp_dec_read_flush_cache_ex_5;	/*!<(rw), DEC_BASE + 0x00000514 */
	uint32_t isp_dec_read_flush_cache_ex_6;	/*!<(rw), DEC_BASE + 0x00000518 */
	uint32_t isp_dec_read_flush_cache_ex_7;	/*!<(rw), DEC_BASE + 0x0000051C */
	uint32_t isp_dec_read_flush_cache_ex_8;	/*!<(rw), DEC_BASE + 0x00000520 */
	uint32_t isp_dec_read_flush_cache_ex_9;	/*!<(rw), DEC_BASE + 0x00000524 */
	uint32_t isp_dec_read_flush_cache_ex_10;	/*!<(rw), DEC_BASE + 0x00000528 */
	uint32_t isp_dec_read_flush_cache_ex_11;	/*!<(rw), DEC_BASE + 0x0000052C */
	uint32_t isp_dec_read_flush_cache_ex_12;	/*!<(rw), DEC_BASE + 0x00000530 */
	uint32_t isp_dec_read_flush_cache_ex_13;	/*!<(rw), DEC_BASE + 0x00000534 */
	uint32_t isp_dec_read_flush_cache_ex_14;	/*!<(rw), DEC_BASE + 0x00000538 */
	uint32_t isp_dec_read_flush_cache_ex_15;	/*!<(rw), DEC_BASE + 0x0000053C */
	uint32_t isp_dec_read_flush_cache_ex_16;	/*!<(rw), DEC_BASE + 0x00000540 */
	uint32_t isp_dec_read_flush_cache_ex_17;	/*!<(rw), DEC_BASE + 0x00000544 */
	uint32_t isp_dec_read_flush_cache_ex_18;	/*!<(rw), DEC_BASE + 0x00000548 */
	uint32_t isp_dec_read_flush_cache_ex_19;	/*!<(rw), DEC_BASE + 0x0000054C */
	uint32_t isp_dec_read_flush_cache_ex_20;	/*!<(rw), DEC_BASE + 0x00000550 */
	uint32_t isp_dec_read_flush_cache_ex_21;	/*!<(rw), DEC_BASE + 0x00000554 */
	uint32_t isp_dec_read_flush_cache_ex_22;	/*!<(rw), DEC_BASE + 0x00000558 */
	uint32_t isp_dec_read_flush_cache_ex_23;	/*!<(rw), DEC_BASE + 0x0000055C */
	uint32_t isp_dec_read_flush_cache_ex_24;	/*!<(rw), DEC_BASE + 0x00000560 */
	uint32_t isp_dec_read_flush_cache_ex_25;	/*!<(rw), DEC_BASE + 0x00000564 */
	uint32_t isp_dec_read_flush_cache_ex_26;	/*!<(rw), DEC_BASE + 0x00000568 */
	uint32_t isp_dec_read_flush_cache_ex_27;	/*!<(rw), DEC_BASE + 0x0000056C */
	uint32_t isp_dec_read_flush_cache_ex_28;	/*!<(rw), DEC_BASE + 0x00000570 */
	uint32_t isp_dec_read_flush_cache_ex_29;	/*!<(rw), DEC_BASE + 0x00000574 */
	uint32_t isp_dec_read_flush_cache_ex_30;	/*!<(rw), DEC_BASE + 0x00000578 */
	uint32_t isp_dec_read_flush_cache_ex_31;	/*!<(rw), DEC_BASE + 0x0000057C */
	uint32_t isp_dec_write_buffer_base_0;	/*!<(rw), DEC_BASE + 0x00000580 */
	uint32_t isp_dec_write_buffer_base_1;	/*!<(rw), DEC_BASE + 0x00000584 */
	uint32_t isp_dec_write_buffer_base_2;	/*!<(rw), DEC_BASE + 0x00000588 */
	uint32_t isp_dec_write_buffer_base_3;	/*!<(rw), DEC_BASE + 0x0000058C */
	uint32_t isp_dec_write_buffer_base_4;	/*!<(rw), DEC_BASE + 0x00000590 */
	uint32_t isp_dec_write_buffer_base_5;	/*!<(rw), DEC_BASE + 0x00000594 */
	uint32_t isp_dec_write_buffer_base_6;	/*!<(rw), DEC_BASE + 0x00000598 */
	uint32_t isp_dec_write_buffer_base_7;	/*!<(rw), DEC_BASE + 0x0000059C */
	uint32_t isp_dec_write_buffer_base_8;	/*!<(rw), DEC_BASE + 0x000005A0 */
	uint32_t isp_dec_write_buffer_base_9;	/*!<(rw), DEC_BASE + 0x000005A4 */
	uint32_t isp_dec_write_buffer_base_10;	/*!<(rw), DEC_BASE + 0x000005A8 */
	uint32_t isp_dec_write_buffer_base_11;	/*!<(rw), DEC_BASE + 0x000005AC */
	uint32_t isp_dec_write_buffer_base_12;	/*!<(rw), DEC_BASE + 0x000005B0 */
	uint32_t isp_dec_write_buffer_base_13;	/*!<(rw), DEC_BASE + 0x000005B4 */
	uint32_t isp_dec_write_buffer_base_14;	/*!<(rw), DEC_BASE + 0x000005B8 */
	uint32_t isp_dec_write_buffer_base_15;	/*!<(rw), DEC_BASE + 0x000005BC */
	uint32_t isp_dec_write_buffer_base_16;	/*!<(rw), DEC_BASE + 0x000005C0 */
	uint32_t isp_dec_write_buffer_base_17;	/*!<(rw), DEC_BASE + 0x000005C4 */
	uint32_t isp_dec_write_buffer_base_18;	/*!<(rw), DEC_BASE + 0x000005C8 */
	uint32_t isp_dec_write_buffer_base_19;	/*!<(rw), DEC_BASE + 0x000005CC */
	uint32_t isp_dec_write_buffer_base_20;	/*!<(rw), DEC_BASE + 0x000005D0 */
	uint32_t isp_dec_write_buffer_base_21;	/*!<(rw), DEC_BASE + 0x000005D4 */
	uint32_t isp_dec_write_buffer_base_22;	/*!<(rw), DEC_BASE + 0x000005D8 */
	uint32_t isp_dec_write_buffer_base_23;	/*!<(rw), DEC_BASE + 0x000005DC */
	uint32_t isp_dec_write_buffer_base_24;	/*!<(rw), DEC_BASE + 0x000005E0 */
	uint32_t isp_dec_write_buffer_base_25;	/*!<(rw), DEC_BASE + 0x000005E4 */
	uint32_t isp_dec_write_buffer_base_26;	/*!<(rw), DEC_BASE + 0x000005E8 */
	uint32_t isp_dec_write_buffer_base_27;	/*!<(rw), DEC_BASE + 0x000005EC */
	uint32_t isp_dec_write_buffer_base_28;	/*!<(rw), DEC_BASE + 0x000005F0 */
	uint32_t isp_dec_write_buffer_base_29;	/*!<(rw), DEC_BASE + 0x000005F4 */
	uint32_t isp_dec_write_buffer_base_30;	/*!<(rw), DEC_BASE + 0x000005F8 */
	uint32_t isp_dec_write_buffer_base_31;	/*!<(rw), DEC_BASE + 0x000005FC */
	uint32_t isp_dec_write_buffer_base_ex_0;	/*!<(rw), DEC_BASE + 0x00000600 */
	uint32_t isp_dec_write_buffer_base_ex_1;	/*!<(rw), DEC_BASE + 0x00000604 */
	uint32_t isp_dec_write_buffer_base_ex_2;	/*!<(rw), DEC_BASE + 0x00000608 */
	uint32_t isp_dec_write_buffer_base_ex_3;	/*!<(rw), DEC_BASE + 0x0000060C */
	uint32_t isp_dec_write_buffer_base_ex_4;	/*!<(rw), DEC_BASE + 0x00000610 */
	uint32_t isp_dec_write_buffer_base_ex_5;	/*!<(rw), DEC_BASE + 0x00000614 */
	uint32_t isp_dec_write_buffer_base_ex_6;	/*!<(rw), DEC_BASE + 0x00000618 */
	uint32_t isp_dec_write_buffer_base_ex_7;	/*!<(rw), DEC_BASE + 0x0000061C */
	uint32_t isp_dec_write_buffer_base_ex_8;	/*!<(rw), DEC_BASE + 0x00000620 */
	uint32_t isp_dec_write_buffer_base_ex_9;	/*!<(rw), DEC_BASE + 0x00000624 */
	uint32_t isp_dec_write_buffer_base_ex_10;	/*!<(rw), DEC_BASE + 0x00000628 */
	uint32_t isp_dec_write_buffer_base_ex_11;	/*!<(rw), DEC_BASE + 0x0000062C */
	uint32_t isp_dec_write_buffer_base_ex_12;	/*!<(rw), DEC_BASE + 0x00000630 */
	uint32_t isp_dec_write_buffer_base_ex_13;	/*!<(rw), DEC_BASE + 0x00000634 */
	uint32_t isp_dec_write_buffer_base_ex_14;	/*!<(rw), DEC_BASE + 0x00000638 */
	uint32_t isp_dec_write_buffer_base_ex_15;	/*!<(rw), DEC_BASE + 0x0000063C */
	uint32_t isp_dec_write_buffer_base_ex_16;	/*!<(rw), DEC_BASE + 0x00000640 */
	uint32_t isp_dec_write_buffer_base_ex_17;	/*!<(rw), DEC_BASE + 0x00000644 */
	uint32_t isp_dec_write_buffer_base_ex_18;	/*!<(rw), DEC_BASE + 0x00000648 */
	uint32_t isp_dec_write_buffer_base_ex_19;	/*!<(rw), DEC_BASE + 0x0000064C */
	uint32_t isp_dec_write_buffer_base_ex_20;	/*!<(rw), DEC_BASE + 0x00000650 */
	uint32_t isp_dec_write_buffer_base_ex_21;	/*!<(rw), DEC_BASE + 0x00000654 */
	uint32_t isp_dec_write_buffer_base_ex_22;	/*!<(rw), DEC_BASE + 0x00000658 */
	uint32_t isp_dec_write_buffer_base_ex_23;	/*!<(rw), DEC_BASE + 0x0000065C */
	uint32_t isp_dec_write_buffer_base_ex_24;	/*!<(rw), DEC_BASE + 0x00000660 */
	uint32_t isp_dec_write_buffer_base_ex_25;	/*!<(rw), DEC_BASE + 0x00000664 */
	uint32_t isp_dec_write_buffer_base_ex_26;	/*!<(rw), DEC_BASE + 0x00000668 */
	uint32_t isp_dec_write_buffer_base_ex_27;	/*!<(rw), DEC_BASE + 0x0000066C */
	uint32_t isp_dec_write_buffer_base_ex_28;	/*!<(rw), DEC_BASE + 0x00000670 */
	uint32_t isp_dec_write_buffer_base_ex_29;	/*!<(rw), DEC_BASE + 0x00000674 */
	uint32_t isp_dec_write_buffer_base_ex_30;	/*!<(rw), DEC_BASE + 0x00000678 */
	uint32_t isp_dec_write_buffer_base_ex_31;	/*!<(rw), DEC_BASE + 0x0000067C */
	uint32_t isp_dec_write_buffer_end_0;	/*!<(rw), DEC_BASE + 0x00000680 */
	uint32_t isp_dec_write_buffer_end_1;	/*!<(rw), DEC_BASE + 0x00000684 */
	uint32_t isp_dec_write_buffer_end_2;	/*!<(rw), DEC_BASE + 0x00000688 */
	uint32_t isp_dec_write_buffer_end_3;	/*!<(rw), DEC_BASE + 0x0000068C */
	uint32_t isp_dec_write_buffer_end_4;	/*!<(rw), DEC_BASE + 0x00000690 */
	uint32_t isp_dec_write_buffer_end_5;	/*!<(rw), DEC_BASE + 0x00000694 */
	uint32_t isp_dec_write_buffer_end_6;	/*!<(rw), DEC_BASE + 0x00000698 */
	uint32_t isp_dec_write_buffer_end_7;	/*!<(rw), DEC_BASE + 0x0000069C */
	uint32_t isp_dec_write_buffer_end_8;	/*!<(rw), DEC_BASE + 0x000006A0 */
	uint32_t isp_dec_write_buffer_end_9;	/*!<(rw), DEC_BASE + 0x000006A4 */
	uint32_t isp_dec_write_buffer_end_10;	/*!<(rw), DEC_BASE + 0x000006A8 */
	uint32_t isp_dec_write_buffer_end_11;	/*!<(rw), DEC_BASE + 0x000006AC */
	uint32_t isp_dec_write_buffer_end_12;	/*!<(rw), DEC_BASE + 0x000006B0 */
	uint32_t isp_dec_write_buffer_end_13;	/*!<(rw), DEC_BASE + 0x000006B4 */
	uint32_t isp_dec_write_buffer_end_14;	/*!<(rw), DEC_BASE + 0x000006B8 */
	uint32_t isp_dec_write_buffer_end_15;	/*!<(rw), DEC_BASE + 0x000006BC */
	uint32_t isp_dec_write_buffer_end_16;	/*!<(rw), DEC_BASE + 0x000006C0 */
	uint32_t isp_dec_write_buffer_end_17;	/*!<(rw), DEC_BASE + 0x000006C4 */
	uint32_t isp_dec_write_buffer_end_18;	/*!<(rw), DEC_BASE + 0x000006C8 */
	uint32_t isp_dec_write_buffer_end_19;	/*!<(rw), DEC_BASE + 0x000006CC */
	uint32_t isp_dec_write_buffer_end_20;	/*!<(rw), DEC_BASE + 0x000006D0 */
	uint32_t isp_dec_write_buffer_end_21;	/*!<(rw), DEC_BASE + 0x000006D4 */
	uint32_t isp_dec_write_buffer_end_22;	/*!<(rw), DEC_BASE + 0x000006D8 */
	uint32_t isp_dec_write_buffer_end_23;	/*!<(rw), DEC_BASE + 0x000006DC */
	uint32_t isp_dec_write_buffer_end_24;	/*!<(rw), DEC_BASE + 0x000006E0 */
	uint32_t isp_dec_write_buffer_end_25;	/*!<(rw), DEC_BASE + 0x000006E4 */
	uint32_t isp_dec_write_buffer_end_26;	/*!<(rw), DEC_BASE + 0x000006E8 */
	uint32_t isp_dec_write_buffer_end_27;	/*!<(rw), DEC_BASE + 0x000006EC */
	uint32_t isp_dec_write_buffer_end_28;	/*!<(rw), DEC_BASE + 0x000006F0 */
	uint32_t isp_dec_write_buffer_end_29;	/*!<(rw), DEC_BASE + 0x000006F4 */
	uint32_t isp_dec_write_buffer_end_30;	/*!<(rw), DEC_BASE + 0x000006F8 */
	uint32_t isp_dec_write_buffer_end_31;	/*!<(rw), DEC_BASE + 0x000006FC */
	uint32_t isp_dec_write_buffer_end_ex_0;	/*!<(rw), DEC_BASE + 0x00000700 */
	uint32_t isp_dec_write_buffer_end_ex_1;	/*!<(rw), DEC_BASE + 0x00000704 */
	uint32_t isp_dec_write_buffer_end_ex_2;	/*!<(rw), DEC_BASE + 0x00000708 */
	uint32_t isp_dec_write_buffer_end_ex_3;	/*!<(rw), DEC_BASE + 0x0000070C */
	uint32_t isp_dec_write_buffer_end_ex_4;	/*!<(rw), DEC_BASE + 0x00000710 */
	uint32_t isp_dec_write_buffer_end_ex_5;	/*!<(rw), DEC_BASE + 0x00000714 */
	uint32_t isp_dec_write_buffer_end_ex_6;	/*!<(rw), DEC_BASE + 0x00000718 */
	uint32_t isp_dec_write_buffer_end_ex_7;	/*!<(rw), DEC_BASE + 0x0000071C */
	uint32_t isp_dec_write_buffer_end_ex_8;	/*!<(rw), DEC_BASE + 0x00000720 */
	uint32_t isp_dec_write_buffer_end_ex_9;	/*!<(rw), DEC_BASE + 0x00000724 */
	uint32_t isp_dec_write_buffer_end_ex_10;	/*!<(rw), DEC_BASE + 0x00000728 */
	uint32_t isp_dec_write_buffer_end_ex_11;	/*!<(rw), DEC_BASE + 0x0000072C */
	uint32_t isp_dec_write_buffer_end_ex_12;	/*!<(rw), DEC_BASE + 0x00000730 */
	uint32_t isp_dec_write_buffer_end_ex_13;	/*!<(rw), DEC_BASE + 0x00000734 */
	uint32_t isp_dec_write_buffer_end_ex_14;	/*!<(rw), DEC_BASE + 0x00000738 */
	uint32_t isp_dec_write_buffer_end_ex_15;	/*!<(rw), DEC_BASE + 0x0000073C */
	uint32_t isp_dec_write_buffer_end_ex_16;	/*!<(rw), DEC_BASE + 0x00000740 */
	uint32_t isp_dec_write_buffer_end_ex_17;	/*!<(rw), DEC_BASE + 0x00000744 */
	uint32_t isp_dec_write_buffer_end_ex_18;	/*!<(rw), DEC_BASE + 0x00000748 */
	uint32_t isp_dec_write_buffer_end_ex_19;	/*!<(rw), DEC_BASE + 0x0000074C */
	uint32_t isp_dec_write_buffer_end_ex_20;	/*!<(rw), DEC_BASE + 0x00000750 */
	uint32_t isp_dec_write_buffer_end_ex_21;	/*!<(rw), DEC_BASE + 0x00000754 */
	uint32_t isp_dec_write_buffer_end_ex_22;	/*!<(rw), DEC_BASE + 0x00000758 */
	uint32_t isp_dec_write_buffer_end_ex_23;	/*!<(rw), DEC_BASE + 0x0000075C */
	uint32_t isp_dec_write_buffer_end_ex_24;	/*!<(rw), DEC_BASE + 0x00000760 */
	uint32_t isp_dec_write_buffer_end_ex_25;	/*!<(rw), DEC_BASE + 0x00000764 */
	uint32_t isp_dec_write_buffer_end_ex_26;	/*!<(rw), DEC_BASE + 0x00000768 */
	uint32_t isp_dec_write_buffer_end_ex_27;	/*!<(rw), DEC_BASE + 0x0000076C */
	uint32_t isp_dec_write_buffer_end_ex_28;	/*!<(rw), DEC_BASE + 0x00000770 */
	uint32_t isp_dec_write_buffer_end_ex_29;	/*!<(rw), DEC_BASE + 0x00000774 */
	uint32_t isp_dec_write_buffer_end_ex_30;	/*!<(rw), DEC_BASE + 0x00000778 */
	uint32_t isp_dec_write_buffer_end_ex_31;	/*!<(rw), DEC_BASE + 0x0000077C */
	uint32_t isp_dec_write_flush_cache_0;	/*!<(rw), DEC_BASE + 0x00000780 */
	uint32_t isp_dec_write_flush_cache_1;	/*!<(rw), DEC_BASE + 0x00000784 */
	uint32_t isp_dec_write_flush_cache_2;	/*!<(rw), DEC_BASE + 0x00000788 */
	uint32_t isp_dec_write_flush_cache_3;	/*!<(rw), DEC_BASE + 0x0000078C */
	uint32_t isp_dec_write_flush_cache_4;	/*!<(rw), DEC_BASE + 0x00000790 */
	uint32_t isp_dec_write_flush_cache_5;	/*!<(rw), DEC_BASE + 0x00000794 */
	uint32_t isp_dec_write_flush_cache_6;	/*!<(rw), DEC_BASE + 0x00000798 */
	uint32_t isp_dec_write_flush_cache_7;	/*!<(rw), DEC_BASE + 0x0000079C */
	uint32_t isp_dec_write_flush_cache_8;	/*!<(rw), DEC_BASE + 0x000007A0 */
	uint32_t isp_dec_write_flush_cache_9;	/*!<(rw), DEC_BASE + 0x000007A4 */
	uint32_t isp_dec_write_flush_cache_10;	/*!<(rw), DEC_BASE + 0x000007A8 */
	uint32_t isp_dec_write_flush_cache_11;	/*!<(rw), DEC_BASE + 0x000007AC */
	uint32_t isp_dec_write_flush_cache_12;	/*!<(rw), DEC_BASE + 0x000007B0 */
	uint32_t isp_dec_write_flush_cache_13;	/*!<(rw), DEC_BASE + 0x000007B4 */
	uint32_t isp_dec_write_flush_cache_14;	/*!<(rw), DEC_BASE + 0x000007B8 */
	uint32_t isp_dec_write_flush_cache_15;	/*!<(rw), DEC_BASE + 0x000007BC */
	uint32_t isp_dec_write_flush_cache_16;	/*!<(rw), DEC_BASE + 0x000007C0 */
	uint32_t isp_dec_write_flush_cache_17;	/*!<(rw), DEC_BASE + 0x000007C4 */
	uint32_t isp_dec_write_flush_cache_18;	/*!<(rw), DEC_BASE + 0x000007C8 */
	uint32_t isp_dec_write_flush_cache_19;	/*!<(rw), DEC_BASE + 0x000007CC */
	uint32_t isp_dec_write_flush_cache_20;	/*!<(rw), DEC_BASE + 0x000007D0 */
	uint32_t isp_dec_write_flush_cache_21;	/*!<(rw), DEC_BASE + 0x000007D4 */
	uint32_t isp_dec_write_flush_cache_22;	/*!<(rw), DEC_BASE + 0x000007D8 */
	uint32_t isp_dec_write_flush_cache_23;	/*!<(rw), DEC_BASE + 0x000007DC */
	uint32_t isp_dec_write_flush_cache_24;	/*!<(rw), DEC_BASE + 0x000007E0 */
	uint32_t isp_dec_write_flush_cache_25;	/*!<(rw), DEC_BASE + 0x000007E4 */
	uint32_t isp_dec_write_flush_cache_26;	/*!<(rw), DEC_BASE + 0x000007E8 */
	uint32_t isp_dec_write_flush_cache_27;	/*!<(rw), DEC_BASE + 0x000007EC */
	uint32_t isp_dec_write_flush_cache_28;	/*!<(rw), DEC_BASE + 0x000007F0 */
	uint32_t isp_dec_write_flush_cache_29;	/*!<(rw), DEC_BASE + 0x000007F4 */
	uint32_t isp_dec_write_flush_cache_30;	/*!<(rw), DEC_BASE + 0x000007F8 */
	uint32_t isp_dec_write_flush_cache_31;	/*!<(rw), DEC_BASE + 0x000007FC */
	uint32_t isp_dec_write_flush_cache_ex_0;	/*!<(rw), DEC_BASE + 0x00000800 */
	uint32_t isp_dec_write_flush_cache_ex_1;	/*!<(rw), DEC_BASE + 0x00000804 */
	uint32_t isp_dec_write_flush_cache_ex_2;	/*!<(rw), DEC_BASE + 0x00000808 */
	uint32_t isp_dec_write_flush_cache_ex_3;	/*!<(rw), DEC_BASE + 0x0000080C */
	uint32_t isp_dec_write_flush_cache_ex_4;	/*!<(rw), DEC_BASE + 0x00000810 */
	uint32_t isp_dec_write_flush_cache_ex_5;	/*!<(rw), DEC_BASE + 0x00000814 */
	uint32_t isp_dec_write_flush_cache_ex_6;	/*!<(rw), DEC_BASE + 0x00000818 */
	uint32_t isp_dec_write_flush_cache_ex_7;	/*!<(rw), DEC_BASE + 0x0000081C */
	uint32_t isp_dec_write_flush_cache_ex_8;	/*!<(rw), DEC_BASE + 0x00000820 */
	uint32_t isp_dec_write_flush_cache_ex_9;	/*!<(rw), DEC_BASE + 0x00000824 */
	uint32_t isp_dec_write_flush_cache_ex_10;	/*!<(rw), DEC_BASE + 0x00000828 */
	uint32_t isp_dec_write_flush_cache_ex_11;	/*!<(rw), DEC_BASE + 0x0000082C */
	uint32_t isp_dec_write_flush_cache_ex_12;	/*!<(rw), DEC_BASE + 0x00000830 */
	uint32_t isp_dec_write_flush_cache_ex_13;	/*!<(rw), DEC_BASE + 0x00000834 */
	uint32_t isp_dec_write_flush_cache_ex_14;	/*!<(rw), DEC_BASE + 0x00000838 */
	uint32_t isp_dec_write_flush_cache_ex_15;	/*!<(rw), DEC_BASE + 0x0000083C */
	uint32_t isp_dec_write_flush_cache_ex_16;	/*!<(rw), DEC_BASE + 0x00000840 */
	uint32_t isp_dec_write_flush_cache_ex_17;	/*!<(rw), DEC_BASE + 0x00000844 */
	uint32_t isp_dec_write_flush_cache_ex_18;	/*!<(rw), DEC_BASE + 0x00000848 */
	uint32_t isp_dec_write_flush_cache_ex_19;	/*!<(rw), DEC_BASE + 0x0000084C */
	uint32_t isp_dec_write_flush_cache_ex_20;	/*!<(rw), DEC_BASE + 0x00000850 */
	uint32_t isp_dec_write_flush_cache_ex_21;	/*!<(rw), DEC_BASE + 0x00000854 */
	uint32_t isp_dec_write_flush_cache_ex_22;	/*!<(rw), DEC_BASE + 0x00000858 */
	uint32_t isp_dec_write_flush_cache_ex_23;	/*!<(rw), DEC_BASE + 0x0000085C */
	uint32_t isp_dec_write_flush_cache_ex_24;	/*!<(rw), DEC_BASE + 0x00000860 */
	uint32_t isp_dec_write_flush_cache_ex_25;	/*!<(rw), DEC_BASE + 0x00000864 */
	uint32_t isp_dec_write_flush_cache_ex_26;	/*!<(rw), DEC_BASE + 0x00000868 */
	uint32_t isp_dec_write_flush_cache_ex_27;	/*!<(rw), DEC_BASE + 0x0000086C */
	uint32_t isp_dec_write_flush_cache_ex_28;	/*!<(rw), DEC_BASE + 0x00000870 */
	uint32_t isp_dec_write_flush_cache_ex_29;	/*!<(rw), DEC_BASE + 0x00000874 */
	uint32_t isp_dec_write_flush_cache_ex_30;	/*!<(rw), DEC_BASE + 0x00000878 */
	uint32_t isp_dec_write_flush_cache_ex_31;	/*!<(rw), DEC_BASE + 0x0000087C */
	uint32_t isp_dec_read_cache_base_0;	/*!<(rw), DEC_BASE + 0x00000880 */
	uint32_t isp_dec_read_cache_base_1;	/*!<(rw), DEC_BASE + 0x00000884 */
	uint32_t isp_dec_read_cache_base_2;	/*!<(rw), DEC_BASE + 0x00000888 */
	uint32_t isp_dec_read_cache_base_3;	/*!<(rw), DEC_BASE + 0x0000088C */
	uint32_t isp_dec_read_cache_base_4;	/*!<(rw), DEC_BASE + 0x00000890 */
	uint32_t isp_dec_read_cache_base_5;	/*!<(rw), DEC_BASE + 0x00000894 */
	uint32_t isp_dec_read_cache_base_6;	/*!<(rw), DEC_BASE + 0x00000898 */
	uint32_t isp_dec_read_cache_base_7;	/*!<(rw), DEC_BASE + 0x0000089C */
	uint32_t isp_dec_read_cache_base_8;	/*!<(rw), DEC_BASE + 0x000008A0 */
	uint32_t isp_dec_read_cache_base_9;	/*!<(rw), DEC_BASE + 0x000008A4 */
	uint32_t isp_dec_read_cache_base_10;	/*!<(rw), DEC_BASE + 0x000008A8 */
	uint32_t isp_dec_read_cache_base_11;	/*!<(rw), DEC_BASE + 0x000008AC */
	uint32_t isp_dec_read_cache_base_12;	/*!<(rw), DEC_BASE + 0x000008B0 */
	uint32_t isp_dec_read_cache_base_13;	/*!<(rw), DEC_BASE + 0x000008B4 */
	uint32_t isp_dec_read_cache_base_14;	/*!<(rw), DEC_BASE + 0x000008B8 */
	uint32_t isp_dec_read_cache_base_15;	/*!<(rw), DEC_BASE + 0x000008BC */
	uint32_t isp_dec_read_cache_base_16;	/*!<(rw), DEC_BASE + 0x000008C0 */
	uint32_t isp_dec_read_cache_base_17;	/*!<(rw), DEC_BASE + 0x000008C4 */
	uint32_t isp_dec_read_cache_base_18;	/*!<(rw), DEC_BASE + 0x000008C8 */
	uint32_t isp_dec_read_cache_base_19;	/*!<(rw), DEC_BASE + 0x000008CC */
	uint32_t isp_dec_read_cache_base_20;	/*!<(rw), DEC_BASE + 0x000008D0 */
	uint32_t isp_dec_read_cache_base_21;	/*!<(rw), DEC_BASE + 0x000008D4 */
	uint32_t isp_dec_read_cache_base_22;	/*!<(rw), DEC_BASE + 0x000008D8 */
	uint32_t isp_dec_read_cache_base_23;	/*!<(rw), DEC_BASE + 0x000008DC */
	uint32_t isp_dec_read_cache_base_24;	/*!<(rw), DEC_BASE + 0x000008E0 */
	uint32_t isp_dec_read_cache_base_25;	/*!<(rw), DEC_BASE + 0x000008E4 */
	uint32_t isp_dec_read_cache_base_26;	/*!<(rw), DEC_BASE + 0x000008E8 */
	uint32_t isp_dec_read_cache_base_27;	/*!<(rw), DEC_BASE + 0x000008EC */
	uint32_t isp_dec_read_cache_base_28;	/*!<(rw), DEC_BASE + 0x000008F0 */
	uint32_t isp_dec_read_cache_base_29;	/*!<(rw), DEC_BASE + 0x000008F4 */
	uint32_t isp_dec_read_cache_base_30;	/*!<(rw), DEC_BASE + 0x000008F8 */
	uint32_t isp_dec_read_cache_base_31;	/*!<(rw), DEC_BASE + 0x000008FC */
	uint32_t isp_dec_read_cache_base_ex_0;	/*!<(rw), DEC_BASE + 0x00000900 */
	uint32_t isp_dec_read_cache_base_ex_1;	/*!<(rw), DEC_BASE + 0x00000904 */
	uint32_t isp_dec_read_cache_base_ex_2;	/*!<(rw), DEC_BASE + 0x00000908 */
	uint32_t isp_dec_read_cache_base_ex_3;	/*!<(rw), DEC_BASE + 0x0000090C */
	uint32_t isp_dec_read_cache_base_ex_4;	/*!<(rw), DEC_BASE + 0x00000910 */
	uint32_t isp_dec_read_cache_base_ex_5;	/*!<(rw), DEC_BASE + 0x00000914 */
	uint32_t isp_dec_read_cache_base_ex_6;	/*!<(rw), DEC_BASE + 0x00000918 */
	uint32_t isp_dec_read_cache_base_ex_7;	/*!<(rw), DEC_BASE + 0x0000091C */
	uint32_t isp_dec_read_cache_base_ex_8;	/*!<(rw), DEC_BASE + 0x00000920 */
	uint32_t isp_dec_read_cache_base_ex_9;	/*!<(rw), DEC_BASE + 0x00000924 */
	uint32_t isp_dec_read_cache_base_ex_10;	/*!<(rw), DEC_BASE + 0x00000928 */
	uint32_t isp_dec_read_cache_base_ex_11;	/*!<(rw), DEC_BASE + 0x0000092C */
	uint32_t isp_dec_read_cache_base_ex_12;	/*!<(rw), DEC_BASE + 0x00000930 */
	uint32_t isp_dec_read_cache_base_ex_13;	/*!<(rw), DEC_BASE + 0x00000934 */
	uint32_t isp_dec_read_cache_base_ex_14;	/*!<(rw), DEC_BASE + 0x00000938 */
	uint32_t isp_dec_read_cache_base_ex_15;	/*!<(rw), DEC_BASE + 0x0000093C */
	uint32_t isp_dec_read_cache_base_ex_16;	/*!<(rw), DEC_BASE + 0x00000940 */
	uint32_t isp_dec_read_cache_base_ex_17;	/*!<(rw), DEC_BASE + 0x00000944 */
	uint32_t isp_dec_read_cache_base_ex_18;	/*!<(rw), DEC_BASE + 0x00000948 */
	uint32_t isp_dec_read_cache_base_ex_19;	/*!<(rw), DEC_BASE + 0x0000094C */
	uint32_t isp_dec_read_cache_base_ex_20;	/*!<(rw), DEC_BASE + 0x00000950 */
	uint32_t isp_dec_read_cache_base_ex_21;	/*!<(rw), DEC_BASE + 0x00000954 */
	uint32_t isp_dec_read_cache_base_ex_22;	/*!<(rw), DEC_BASE + 0x00000958 */
	uint32_t isp_dec_read_cache_base_ex_23;	/*!<(rw), DEC_BASE + 0x0000095C */
	uint32_t isp_dec_read_cache_base_ex_24;	/*!<(rw), DEC_BASE + 0x00000960 */
	uint32_t isp_dec_read_cache_base_ex_25;	/*!<(rw), DEC_BASE + 0x00000964 */
	uint32_t isp_dec_read_cache_base_ex_26;	/*!<(rw), DEC_BASE + 0x00000968 */
	uint32_t isp_dec_read_cache_base_ex_27;	/*!<(rw), DEC_BASE + 0x0000096C */
	uint32_t isp_dec_read_cache_base_ex_28;	/*!<(rw), DEC_BASE + 0x00000970 */
	uint32_t isp_dec_read_cache_base_ex_29;	/*!<(rw), DEC_BASE + 0x00000974 */
	uint32_t isp_dec_read_cache_base_ex_30;	/*!<(rw), DEC_BASE + 0x00000978 */
	uint32_t isp_dec_read_cache_base_ex_31;	/*!<(rw), DEC_BASE + 0x0000097C */
	uint32_t isp_dec_write_cache_base_0;	/*!<(rw), DEC_BASE + 0x00000980 */
	uint32_t isp_dec_write_cache_base_1;	/*!<(rw), DEC_BASE + 0x00000984 */
	uint32_t isp_dec_write_cache_base_2;	/*!<(rw), DEC_BASE + 0x00000988 */
	uint32_t isp_dec_write_cache_base_3;	/*!<(rw), DEC_BASE + 0x0000098C */
	uint32_t isp_dec_write_cache_base_4;	/*!<(rw), DEC_BASE + 0x00000990 */
	uint32_t isp_dec_write_cache_base_5;	/*!<(rw), DEC_BASE + 0x00000994 */
	uint32_t isp_dec_write_cache_base_6;	/*!<(rw), DEC_BASE + 0x00000998 */
	uint32_t isp_dec_write_cache_base_7;	/*!<(rw), DEC_BASE + 0x0000099C */
	uint32_t isp_dec_write_cache_base_8;	/*!<(rw), DEC_BASE + 0x000009A0 */
	uint32_t isp_dec_write_cache_base_9;	/*!<(rw), DEC_BASE + 0x000009A4 */
	uint32_t isp_dec_write_cache_base_10;	/*!<(rw), DEC_BASE + 0x000009A8 */
	uint32_t isp_dec_write_cache_base_11;	/*!<(rw), DEC_BASE + 0x000009AC */
	uint32_t isp_dec_write_cache_base_12;	/*!<(rw), DEC_BASE + 0x000009B0 */
	uint32_t isp_dec_write_cache_base_13;	/*!<(rw), DEC_BASE + 0x000009B4 */
	uint32_t isp_dec_write_cache_base_14;	/*!<(rw), DEC_BASE + 0x000009B8 */
	uint32_t isp_dec_write_cache_base_15;	/*!<(rw), DEC_BASE + 0x000009BC */
	uint32_t isp_dec_write_cache_base_16;	/*!<(rw), DEC_BASE + 0x000009C0 */
	uint32_t isp_dec_write_cache_base_17;	/*!<(rw), DEC_BASE + 0x000009C4 */
	uint32_t isp_dec_write_cache_base_18;	/*!<(rw), DEC_BASE + 0x000009C8 */
	uint32_t isp_dec_write_cache_base_19;	/*!<(rw), DEC_BASE + 0x000009CC */
	uint32_t isp_dec_write_cache_base_20;	/*!<(rw), DEC_BASE + 0x000009D0 */
	uint32_t isp_dec_write_cache_base_21;	/*!<(rw), DEC_BASE + 0x000009D4 */
	uint32_t isp_dec_write_cache_base_22;	/*!<(rw), DEC_BASE + 0x000009D8 */
	uint32_t isp_dec_write_cache_base_23;	/*!<(rw), DEC_BASE + 0x000009DC */
	uint32_t isp_dec_write_cache_base_24;	/*!<(rw), DEC_BASE + 0x000009E0 */
	uint32_t isp_dec_write_cache_base_25;	/*!<(rw), DEC_BASE + 0x000009E4 */
	uint32_t isp_dec_write_cache_base_26;	/*!<(rw), DEC_BASE + 0x000009E8 */
	uint32_t isp_dec_write_cache_base_27;	/*!<(rw), DEC_BASE + 0x000009EC */
	uint32_t isp_dec_write_cache_base_28;	/*!<(rw), DEC_BASE + 0x000009F0 */
	uint32_t isp_dec_write_cache_base_29;	/*!<(rw), DEC_BASE + 0x000009F4 */
	uint32_t isp_dec_write_cache_base_30;	/*!<(rw), DEC_BASE + 0x000009F8 */
	uint32_t isp_dec_write_cache_base_31;	/*!<(rw), DEC_BASE + 0x000009FC */
	uint32_t isp_dec_write_cache_base_ex_0;	/*!<(rw), DEC_BASE + 0x00000A00 */
	uint32_t isp_dec_write_cache_base_ex_1;	/*!<(rw), DEC_BASE + 0x00000A04 */
	uint32_t isp_dec_write_cache_base_ex_2;	/*!<(rw), DEC_BASE + 0x00000A08 */
	uint32_t isp_dec_write_cache_base_ex_3;	/*!<(rw), DEC_BASE + 0x00000A0C */
	uint32_t isp_dec_write_cache_base_ex_4;	/*!<(rw), DEC_BASE + 0x00000A10 */
	uint32_t isp_dec_write_cache_base_ex_5;	/*!<(rw), DEC_BASE + 0x00000A14 */
	uint32_t isp_dec_write_cache_base_ex_6;	/*!<(rw), DEC_BASE + 0x00000A18 */
	uint32_t isp_dec_write_cache_base_ex_7;	/*!<(rw), DEC_BASE + 0x00000A1C */
	uint32_t isp_dec_write_cache_base_ex_8;	/*!<(rw), DEC_BASE + 0x00000A20 */
	uint32_t isp_dec_write_cache_base_ex_9;	/*!<(rw), DEC_BASE + 0x00000A24 */
	uint32_t isp_dec_write_cache_base_ex_10;	/*!<(rw), DEC_BASE + 0x00000A28 */
	uint32_t isp_dec_write_cache_base_ex_11;	/*!<(rw), DEC_BASE + 0x00000A2C */
	uint32_t isp_dec_write_cache_base_ex_12;	/*!<(rw), DEC_BASE + 0x00000A30 */
	uint32_t isp_dec_write_cache_base_ex_13;	/*!<(rw), DEC_BASE + 0x00000A34 */
	uint32_t isp_dec_write_cache_base_ex_14;	/*!<(rw), DEC_BASE + 0x00000A38 */
	uint32_t isp_dec_write_cache_base_ex_15;	/*!<(rw), DEC_BASE + 0x00000A3C */
	uint32_t isp_dec_write_cache_base_ex_16;	/*!<(rw), DEC_BASE + 0x00000A40 */
	uint32_t isp_dec_write_cache_base_ex_17;	/*!<(rw), DEC_BASE + 0x00000A44 */
	uint32_t isp_dec_write_cache_base_ex_18;	/*!<(rw), DEC_BASE + 0x00000A48 */
	uint32_t isp_dec_write_cache_base_ex_19;	/*!<(rw), DEC_BASE + 0x00000A4C */
	uint32_t isp_dec_write_cache_base_ex_20;	/*!<(rw), DEC_BASE + 0x00000A50 */
	uint32_t isp_dec_write_cache_base_ex_21;	/*!<(rw), DEC_BASE + 0x00000A54 */
	uint32_t isp_dec_write_cache_base_ex_22;	/*!<(rw), DEC_BASE + 0x00000A58 */
	uint32_t isp_dec_write_cache_base_ex_23;	/*!<(rw), DEC_BASE + 0x00000A5C */
	uint32_t isp_dec_write_cache_base_ex_24;	/*!<(rw), DEC_BASE + 0x00000A60 */
	uint32_t isp_dec_write_cache_base_ex_25;	/*!<(rw), DEC_BASE + 0x00000A64 */
	uint32_t isp_dec_write_cache_base_ex_26;	/*!<(rw), DEC_BASE + 0x00000A68 */
	uint32_t isp_dec_write_cache_base_ex_27;	/*!<(rw), DEC_BASE + 0x00000A6C */
	uint32_t isp_dec_write_cache_base_ex_28;	/*!<(rw), DEC_BASE + 0x00000A70 */
	uint32_t isp_dec_write_cache_base_ex_29;	/*!<(rw), DEC_BASE + 0x00000A74 */
	uint32_t isp_dec_write_cache_base_ex_30;	/*!<(rw), DEC_BASE + 0x00000A78 */
	uint32_t isp_dec_write_cache_base_ex_31;	/*!<(rw), DEC_BASE + 0x00000A7C */
	uint32_t isp_dec_debug_info_out;	/*!<(ro), DEC_BASE + 0x00000A80 */
	uint32_t isp_dec_debug_0;	/*!<(ro), DEC_BASE + 0x00000A84 */
	uint32_t isp_dec_debug_1;	/*!<(ro), DEC_BASE + 0x00000A88 */
	uint32_t isp_dec_debug_2;	/*!<(ro), DEC_BASE + 0x00000A8C */
	uint32_t isp_dec_debug_3;	/*!<(ro), DEC_BASE + 0x00000A90 */
	uint32_t isp_dec_debug_4;	/*!<(ro), DEC_BASE + 0x00000A94 */
	uint32_t isp_dec_debug_5;	/*!<(ro), DEC_BASE + 0x00000A98 */
	uint32_t isp_dec_state_commit;	/*!<(ro), DEC_BASE + 0x00000A9C */
	uint32_t isp_dec_debug_6;	/*!<(ro), DEC_BASE + 0x00000AA0 */
	uint32_t isp_dec_debug_7;	/*!<(ro), DEC_BASE + 0x00000AA4 */
	uint32_t isp_dec_tile_128_type_0;	/*!<(ro), DEC_BASE + 0x00000AA8 */
	uint32_t isp_dec_tile_128_type_1;	/*!<(ro), DEC_BASE + 0x00000AAC */
	uint32_t isp_dec_tile_128_type_2;	/*!<(ro), DEC_BASE + 0x00000AB0 */
	uint32_t isp_dec_tile_128_type_3;	/*!<(ro), DEC_BASE + 0x00000AB4 */
	uint32_t isp_dec_tile_256_type_0;	/*!<(ro), DEC_BASE + 0x00000AB8 */
	uint32_t isp_dec_tile_256_type_1;	/*!<(ro), DEC_BASE + 0x00000ABC */
	uint32_t isp_dec_tile_256_type_2;	/*!<(ro), DEC_BASE + 0x00000AC0 */
	uint32_t isp_dec_tile_256_type_3;	/*!<(ro), DEC_BASE + 0x00000AC4 */
	uint32_t isp_dec_tile_256_type_4;	/*!<(ro), DEC_BASE + 0x00000AC8 */
	uint32_t isp_dec_tile_256_type_5;	/*!<(ro), DEC_BASE + 0x00000ACC */
	uint32_t isp_dec_tile_256_type_6;	/*!<(ro), DEC_BASE + 0x00000AD0 */
	uint32_t isp_dec_tile_256_type_7;	/*!<(ro), DEC_BASE + 0x00000AD4 */
	uint32_t isp_dec_debug_read_gate_domain_clk_counter;	/*!<(ro), DEC_BASE + 0x00000AD8 */
	uint32_t isp_dec_debug_write_gate_domain_clk_counter;	/*!<(ro), DEC_BASE + 0x00000ADC */
	uint32_t isp_dec_debug_other_gate_domain_clk_counter;	/*!<(ro), DEC_BASE + 0x00000AE0 */
	uint32_t isp_dec_fast_clear_value_0;	/*!<(rw), DEC_BASE + 0x00000B00 */
	uint32_t isp_dec_fast_clear_value_1;	/*!<(rw), DEC_BASE + 0x00000B04 */
	uint32_t isp_dec_fast_clear_value_2;	/*!<(rw), DEC_BASE + 0x00000B08 */
	uint32_t isp_dec_fast_clear_value_3;	/*!<(rw), DEC_BASE + 0x00000B0C */
	uint32_t isp_dec_fast_clear_value_4;	/*!<(rw), DEC_BASE + 0x00000B10 */
	uint32_t isp_dec_fast_clear_value_5;	/*!<(rw), DEC_BASE + 0x00000B14 */
	uint32_t isp_dec_fast_clear_value_6;	/*!<(rw), DEC_BASE + 0x00000B18 */
	uint32_t isp_dec_fast_clear_value_7;	/*!<(rw), DEC_BASE + 0x00000B1C */
	uint32_t isp_dec_fast_clear_value_8;	/*!<(rw), DEC_BASE + 0x00000B20 */
	uint32_t isp_dec_fast_clear_value_9;	/*!<(rw), DEC_BASE + 0x00000B24 */
	uint32_t isp_dec_fast_clear_value_10;	/*!<(rw), DEC_BASE + 0x00000B28 */
	uint32_t isp_dec_fast_clear_value_11;	/*!<(rw), DEC_BASE + 0x00000B2C */
	uint32_t isp_dec_fast_clear_value_12;	/*!<(rw), DEC_BASE + 0x00000B30 */
	uint32_t isp_dec_fast_clear_value_13;	/*!<(rw), DEC_BASE + 0x00000B34 */
	uint32_t isp_dec_fast_clear_value_14;	/*!<(rw), DEC_BASE + 0x00000B38 */
	uint32_t isp_dec_fast_clear_value_15;	/*!<(rw), DEC_BASE + 0x00000B3C */
	uint32_t isp_dec_fast_clear_value_16;	/*!<(rw), DEC_BASE + 0x00000B40 */
	uint32_t isp_dec_fast_clear_value_17;	/*!<(rw), DEC_BASE + 0x00000B44 */
	uint32_t isp_dec_fast_clear_value_18;	/*!<(rw), DEC_BASE + 0x00000B48 */
	uint32_t isp_dec_fast_clear_value_19;	/*!<(rw), DEC_BASE + 0x00000B4C */
	uint32_t isp_dec_fast_clear_value_20;	/*!<(rw), DEC_BASE + 0x00000B50 */
	uint32_t isp_dec_fast_clear_value_21;	/*!<(rw), DEC_BASE + 0x00000B54 */
	uint32_t isp_dec_fast_clear_value_22;	/*!<(rw), DEC_BASE + 0x00000B58 */
	uint32_t isp_dec_fast_clear_value_23;	/*!<(rw), DEC_BASE + 0x00000B5C */
	uint32_t isp_dec_fast_clear_value_24;	/*!<(rw), DEC_BASE + 0x00000B60 */
	uint32_t isp_dec_fast_clear_value_25;	/*!<(rw), DEC_BASE + 0x00000B64 */
	uint32_t isp_dec_fast_clear_value_26;	/*!<(rw), DEC_BASE + 0x00000B68 */
	uint32_t isp_dec_fast_clear_value_27;	/*!<(rw), DEC_BASE + 0x00000B6C */
	uint32_t isp_dec_fast_clear_value_28;	/*!<(rw), DEC_BASE + 0x00000B70 */
	uint32_t isp_dec_fast_clear_value_29;	/*!<(rw), DEC_BASE + 0x00000B74 */
	uint32_t isp_dec_fast_clear_value_30;	/*!<(rw), DEC_BASE + 0x00000B78 */
	uint32_t isp_dec_fast_clear_value_31;	/*!<(rw), DEC_BASE + 0x00000B7C */
	uint32_t isp_dec_fast_clear_value_ex_0;	/*!<(rw), DEC_BASE + 0x00000B80 */
	uint32_t isp_dec_fast_clear_value_ex_1;	/*!<(rw), DEC_BASE + 0x00000B84 */
	uint32_t isp_dec_fast_clear_value_ex_2;	/*!<(rw), DEC_BASE + 0x00000B88 */
	uint32_t isp_dec_fast_clear_value_ex_3;	/*!<(rw), DEC_BASE + 0x00000B8C */
	uint32_t isp_dec_fast_clear_value_ex_4;	/*!<(rw), DEC_BASE + 0x00000B90 */
	uint32_t isp_dec_fast_clear_value_ex_5;	/*!<(rw), DEC_BASE + 0x00000B94 */
	uint32_t isp_dec_fast_clear_value_ex_6;	/*!<(rw), DEC_BASE + 0x00000B98 */
	uint32_t isp_dec_fast_clear_value_ex_7;	/*!<(rw), DEC_BASE + 0x00000B9C */
	uint32_t isp_dec_fast_clear_value_ex_8;	/*!<(rw), DEC_BASE + 0x00000BA0 */
	uint32_t isp_dec_fast_clear_value_ex_9;	/*!<(rw), DEC_BASE + 0x00000BA4 */
	uint32_t isp_dec_fast_clear_value_ex_10;	/*!<(rw), DEC_BASE + 0x00000BA8 */
	uint32_t isp_dec_fast_clear_value_ex_11;	/*!<(rw), DEC_BASE + 0x00000BAC */
	uint32_t isp_dec_fast_clear_value_ex_12;	/*!<(rw), DEC_BASE + 0x00000BB0 */
	uint32_t isp_dec_fast_clear_value_ex_13;	/*!<(rw), DEC_BASE + 0x00000BB4 */
	uint32_t isp_dec_fast_clear_value_ex_14;	/*!<(rw), DEC_BASE + 0x00000BB8 */
	uint32_t isp_dec_fast_clear_value_ex_15;	/*!<(rw), DEC_BASE + 0x00000BBC */
	uint32_t isp_dec_fast_clear_value_ex_16;	/*!<(rw), DEC_BASE + 0x00000BC0 */
	uint32_t isp_dec_fast_clear_value_ex_17;	/*!<(rw), DEC_BASE + 0x00000BC4 */
	uint32_t isp_dec_fast_clear_value_ex_18;	/*!<(rw), DEC_BASE + 0x00000BC8 */
	uint32_t isp_dec_fast_clear_value_ex_19;	/*!<(rw), DEC_BASE + 0x00000BCC */
	uint32_t isp_dec_fast_clear_value_ex_20;	/*!<(rw), DEC_BASE + 0x00000BD0 */
	uint32_t isp_dec_fast_clear_value_ex_21;	/*!<(rw), DEC_BASE + 0x00000BD4 */
	uint32_t isp_dec_fast_clear_value_ex_22;	/*!<(rw), DEC_BASE + 0x00000BD8 */
	uint32_t isp_dec_fast_clear_value_ex_23;	/*!<(rw), DEC_BASE + 0x00000BDC */
	uint32_t isp_dec_fast_clear_value_ex_24;	/*!<(rw), DEC_BASE + 0x00000BE0 */
	uint32_t isp_dec_fast_clear_value_ex_25;	/*!<(rw), DEC_BASE + 0x00000BE4 */
	uint32_t isp_dec_fast_clear_value_ex_26;	/*!<(rw), DEC_BASE + 0x00000BE8 */
	uint32_t isp_dec_fast_clear_value_ex_27;	/*!<(rw), DEC_BASE + 0x00000BEC */
	uint32_t isp_dec_fast_clear_value_ex_28;	/*!<(rw), DEC_BASE + 0x00000BF0 */
	uint32_t isp_dec_fast_clear_value_ex_29;	/*!<(rw), DEC_BASE + 0x00000BF4 */
	uint32_t isp_dec_fast_clear_value_ex_30;	/*!<(rw), DEC_BASE + 0x00000BF8 */
	uint32_t isp_dec_fast_clear_value_ex_31;	/*!<(rw), DEC_BASE + 0x00000BFC */

} MrvDecAllRegister_t;

/*! Register: isp_dec_ctrl  (DEC_BASE + 0x00000000)*/
/*! Slice: isp_dec_ctrl_flush:*/
#define  DEC_CTRL_FLUSH
#define  DEC_CTRL_FLUSH_MASK 0x00000001U
#define  DEC_CTRL_FLUSH_SHIFT 0U

/*! Slice: isp_dec_ctrl_disable_compression:*/
#define  DEC_CTRL_DISABLE_COMPRESSION
#define  DEC_CTRL_DISABLE_COMPRESSION_MASK 0x00000002U
#define  DEC_CTRL_DISABLE_COMPRESSION_SHIFT 1U

/*! Slice: isp_dec_ctrl_disable_ram_clock_gating:*/
#define  DEC_CTRL_DISABLE_RAM_CLOCK_GATING
#define  DEC_CTRL_DISABLE_RAM_CLOCK_GATING_MASK 0x00000004U
#define  DEC_CTRL_DISABLE_RAM_CLOCK_GATING_SHIFT 2U

/*! Slice: isp_dec_ctrl_disable_debug_registers:*/
#define  DEC_CTRL_DISABLE_DEBUG_REGISTERS
#define  DEC_CTRL_DISABLE_DEBUG_REGISTERS_MASK 0x00000008U
#define  DEC_CTRL_DISABLE_DEBUG_REGISTERS_SHIFT 3U

/*! Slice: isp_dec_ctrl_soft_reset:*/
#define  DEC_CTRL_SOFT_RESET
#define  DEC_CTRL_SOFT_RESET_MASK 0x00000010U
#define  DEC_CTRL_SOFT_RESET_SHIFT 4U

/*! Slice: isp_dec_ctrl_flush_dcache:*/
#define  DEC_CTRL_FLUSH_DCACHE
#define  DEC_CTRL_FLUSH_DCACHE_MASK 0x00000040U
#define  DEC_CTRL_FLUSH_DCACHE_SHIFT 6U

/*! Slice: isp_dec_ctrl_disable_dcache:*/
#define  DEC_CTRL_DISABLE_DCACHE
#define  DEC_CTRL_DISABLE_DCACHE_MASK 0x00000080U
#define  DEC_CTRL_DISABLE_DCACHE_SHIFT 7U

/*! Slice: isp_dec_ctrl_disable_hw_flush:*/
#define  DEC_CTRL_DISABLE_HW_FLUSH
#define  DEC_CTRL_DISABLE_HW_FLUSH_MASK 0x00010000U
#define  DEC_CTRL_DISABLE_HW_FLUSH_SHIFT 16U

/*! Slice: isp_dec_ctrl_clk_dis:*/
#define  DEC_CTRL_CLK_DIS
#define  DEC_CTRL_CLK_DIS_MASK 0x00FC0000U
#define  DEC_CTRL_CLK_DIS_SHIFT 17U

/*! Slice: isp_dec_ctrl_sw_flush_id:*/
#define  DEC_CTRL_SW_FLUSH_ID
#define  DEC_CTRL_SW_FLUSH_ID_MASK 0x00020000U
#define  DEC_CTRL_SW_FLUSH_ID_SHIFT 18U

/*! Slice: isp_dec_ctrl_disable_cache_prefetch:*/
#define  DEC_CTRL_DISABLE_CACHE_PREFETCH
#define  DEC_CTRL_DISABLE_CACHE_PREFETCH_MASK 0x02000000U
#define  DEC_CTRL_DISABLE_CACHE_PREFETCH_SHIFT 25U

/*! Slice: isp_dec_ctrl_hw_update_shadow_reg_mode:*/
#define  DEC_CTRL_HW_UPDATE_SHADOW_REG_MODE
#define  DEC_CTRL_HW_UPDATE_SHADOW_REG_MODE_MASK 0x10000000U
#define  DEC_CTRL_HW_UPDATE_SHADOW_REG_MODE_SHIFT 28U

/*! Slice: isp_dec_ctrl_soft_update_shadow_reg:*/
#define  DEC_CTRL_SOFT_UPDATE_SHADOW_REG
#define  DEC_CTRL_SOFT_UPDATE_SHADOW_REG_MASK 0x20000000U
#define  DEC_CTRL_SOFT_UPDATE_SHADOW_REG_SHIFT 29U

/*! Slice: isp_dec_ctrl_disable_module_clock_gating:*/
#define  DEC_CTRL_DISABLE_MODULE_CLOCK_GATING
#define  DEC_CTRL_DISABLE_MODULE_CLOCK_GATING_MASK 0x40000000U
#define  DEC_CTRL_DISABLE_MODULE_CLOCK_GATING_SHIFT 30U

/*! Slice: isp_dec_ctrl_disable_global_clock_gating:*/
#define  DEC_CTRL_DISABLE_GLOBAL_CLOCK_GATING
#define  DEC_CTRL_DISABLE_GLOBAL_CLOCK_GATING_MASK 0x80000000U
#define  DEC_CTRL_DISABLE_GLOBAL_CLOCK_GATING_SHIFT 31U

/*! Register: isp_dec_ctrl_ex  (DEC_BASE + 0x00000004)*/
/*! Slice: isp_dec_ctrl_ex_enable_burst_split:*/
#define  DEC_CTRL_EX_ENABLE_BURST_SPLIT
#define  DEC_CTRL_EX_ENABLE_BURST_SPLIT_MASK 0x00010000U
#define  DEC_CTRL_EX_ENABLE_BURST_SPLIT_SHIFT 16U

/*! Slice: isp_dec_ctrl_ex_enable_end_address_check:*/
#define  DEC_CTRL_EX_ENABLE_END_ADDRESS_RANGE_CHECK
#define  DEC_CTRL_EX_ENABLE_END_ADDRESS_RANGE_CHECK_MASK 0x00020000U
#define  DEC_CTRL_EX_ENABLE_END_ADDRESS_RANGE_CHECK_SHIFT 17U

/*! Slice: isp_dec_ctrl_ex_write_miss_policy:*/
#define  DEC_CTRL_EX_WRITE_MISS_POLICY
#define  DEC_CTRL_EX_WRITE_MISS_POLICY_MASK 0x00080000U
#define  DEC_CTRL_EX_WRITE_MISS_POLICY_SHIFT 19U

/*! Slice: isp_dec_ctrl_ex_read_miss_policy:*/
#define  DEC_CTRL_EX_READ_MISS_POLICY
#define  DEC_CTRL_EX_READ_MISS_POLICY_MASK 0x20000000U
#define  DEC_CTRL_EX_READ_MISS_POLICY_SHIFT 29U

/*! Register: isp_dec_ctrl_ex2  (DEC_BASE + 0x00000008)*/
/*! Slice: isp_dec_ctrl_ex2_tile_status_read_id:*/
#define  DEC_CTRL_EX2_TILE_STATUS_READ_ID
#define  DEC_CTRL_EX2_TILE_STATUS_READ_ID_MASK 0x0000007FU
#define  DEC_CTRL_EX2_TILE_STATUS_READ_ID_SHIFT 0U

/*! Slice: isp_dec_ctrl_ex2_tile_status_write_id:*/
#define  DEC_CTRL_EX2_TILE_STATUS_WRITE_ID
#define  DEC_CTRL_EX2_TILE_STATUS_WRITE_ID_MASK 0x0003F80U
#define  DEC_CTRL_EX2_TILE_STATUS_WRITE_ID_SHIFT 7U

/*! Register: isp_dec_intr_enbl  (DEC_BASE + 0x0000000C)*/
/*! Slice: isp_dec_intr_enbl_vec:*/
#define  DEC_INTR_ENBL_VEC
#define  DEC_INTR_ENBL_VEC_MASK 0xFFFFFFFFU
#define  DEC_INTR_ENBL_VEC_SHIFT 0U

/*! Register: isp_dec_intr_enbl_ex  (DEC_BASE + 0x00000010)*/
/*! Slice: isp_dec_intr_enbl_ex_vec:*/
#define  DEC_INTR_ENBL_EX_VEC
#define  DEC_INTR_ENBL_EX_VEC_MASK 0xFFFFFFFFU
#define  DEC_INTR_ENBL_EX_VEC_SHIFT 0U

/*! Register: isp_dec_intr_enbl_ex2  (DEC_BASE + 0x00000014)*/
/*! Slice: isp_dec_intr_enbl_ex2_vec:*/
#define  DEC_INTR_ENBL_EX2_VEC
#define  DEC_INTR_ENBL_EX2_VEC_MASK 0xFFFFFFFFU
#define  DEC_INTR_ENBL_EX2_VEC_SHIFT 0U

/*! Register: isp_dec_read_config  (DEC_BASE + 0x00000080)*/
/*! Slice: isp_dec_read_config_compression_enable:*/
#define  DEC_READ_CONFIG_COMPRESSION_ENABLE
#define  DEC_READ_CONFIG_COMPRESSION_ENABLE_MASK 0x00000001U
#define  DEC_READ_CONFIG_COMPRESSION_ENABLE_SHIFT 0U

/*! Slice: isp_dec_read_config_compression_format:*/
#define  DEC_READ_CONFIG_COMPRESSION_FORMAT
#define  DEC_READ_CONFIG_COMPRESSION_FORMAT_MASK 0x000000F8U
#define  DEC_READ_CONFIG_COMPRESSION_FORMAT_SHIFT 3U

/*! Slice: isp_dec_read_config_compression_align_mode:*/
#define  DEC_READ_CONFIG_COMPRESSION_ALIGN_MODE
#define  DEC_READ_CONFIG_COMPRESSION_ALIGN_MODE_MASK 0x00030000U
#define  DEC_READ_CONFIG_COMPRESSION_ALIGN_MODE_SHIFT 16U

/*! Slice: isp_dec_read_config_compression_align_mode1:*/
#define  DEC_READ_CONFIG_COMPRESSION_ALIGN_MODE1
#define  DEC_READ_CONFIG_COMPRESSION_ALIGN_MODE1_MASK 0x001C0000U
#define  DEC_READ_CONFIG_COMPRESSION_ALIGN_MODE1_SHIFT 18U

/*! Slice: isp_dec_read_config_tile_mode:*/
#define  DEC_READ_CONFIG_TILE_MODE
#define  DEC_READ_CONFIG_TILE_MODE_MASK 0x7E000000U
#define  DEC_READ_CONFIG_TILE_MODE_SHIFT 25U

/*! Register: isp_dec_read_ex_config  (DEC_BASE + 0x00000100)*/
/*! Slice: isp_dec_read_ex_config_bit_depth:*/
#define  DEC_READ_EX_CONFIG_BIT_DEPTH
#define  DEC_READ_EX_CONFIG_BIT_DEPTH_MASK 0x00070000U
#define  DEC_READ_EX_CONFIG_BIT_DEPTH_SHIFT 16U

/*! Slice: isp_dec_read_ex_config_tile_y:*/
#define  DEC_READ_EX_CONFIG_TILE_Y
#define  DEC_READ_EX_CONFIG_TILE_Y_MASK 0x00080000U
#define  DEC_READ_EX_CONFIG_TILE_Y_SHIFT 19U

/*! Register: isp_dec_write_config  (DEC_BASE + 0x00000180)*/
/*! Slice: isp_dec_write_config_compression_enable:*/
#define  DEC_WRITE_CONFIG_COMPRESSION_ENABLE
#define  DEC_WRITE_CONFIG_COMPRESSION_ENABLE_MASK 0x00000001U
#define  DEC_WRITE_CONFIG_COMPRESSION_ENABLE_SHIFT 0U

/*! Slice: isp_dec_write_config_compression_format:*/
#define  DEC_WRITE_CONFIG_COMPRESSION_FORMAT
#define  DEC_WRITE_CONFIG_COMPRESSION_FORMAT_MASK 0x000000F8U
#define  DEC_WRITE_CONFIG_COMPRESSION_FORMAT_SHIFT 3U

/*! Slice: isp_dec_write_config_compression_align_mode:*/
#define  DEC_WRITE_CONFIG_COMPRESSION_ALIGN_MODE
#define  DEC_WRITE_CONFIG_COMPRESSION_ALIGN_MODE_MASK 0x00030000U
#define  DEC_WRITE_CONFIG_COMPRESSION_ALIGN_MODE_SHIFT 16U

/*! Slice: isp_dec_write_config_compression_align_mode1:*/
#define  DEC_WRITE_CONFIG_COMPRESSION_ALIGN_MODE1
#define  DEC_WRITE_CONFIG_COMPRESSION_ALIGN_MODE1_MASK 0x001C0000U
#define  DEC_WRITE_CONFIG_COMPRESSION_ALIGN_MODE1_SHIFT 18U

/*! Slice: isp_dec_write_config_tile_mode:*/
#define  DEC_WRITE_CONFIG_TILE_MODE
#define  DEC_WRITE_CONFIG_TILE_MODE_MASK 0x7E000000U
#define  DEC_WRITE_CONFIG_TILE_MODE_SHIFT 25U

/*! Register: isp_dec_write_ex_config  (DEC_BASE + 0x00000200)*/
/*! Slice: isp_dec_write_ex_config_bit_depth:*/
#define  DEC_WRITE_EX_CONFIG_BIT_DEPTH
#define  DEC_WRITE_EX_CONFIG_BIT_DEPTH_MASK 0x00070000U
#define  DEC_WRITE_EX_CONFIG_BIT_DEPTH_SHIFT 16U

/*! Slice: isp_dec_write_ex_config_tile_y:*/
#define  DEC_WRITE_EX_CONFIG_TILE_Y
#define  DEC_WRITE_EX_CONFIG_TILE_Y_MASK 0x00080000U
#define  DEC_WRITE_EX_CONFIG_TILE_Y_SHIFT 19U

/*! Register: isp_dec_read_buffer_base  (DEC_BASE + 0x00000280)*/
/*! Slice: isp_dec_read_buffer_base:*/
#define  DEC_READ_BUFFER_BASE
#define  DEC_READ_BUFFER_BASE_MASK 0xFFFFFFFFU
#define  DEC_READ_BUFFER_BASE_SHIFT 0U

/*! Register: isp_dec_read_buffer_base_ex  (DEC_BASE + 0x00000300)*/
/*! Slice: isp_dec_read_buffer_base_ex:*/
#define  DEC_READ_BUFFER_BASE_EX
#define  DEC_READ_BUFFER_BASE_EX_MASK 0x000000FFU
#define  DEC_READ_BUFFER_BASE_EX_SHIFT 0U

/*! Register: isp_dec_read_buffer_end  (DEC_BASE + 0x00000380)*/
/*! Slice: isp_dec_read_buffer_end:*/
#define  DEC_READ_BUFFER_END
#define  DEC_READ_BUFFER_END_MASK 0xFFFFFFFFU
#define  DEC_READ_BUFFER_END_SHIFT 0U

/*! Register: isp_dec_read_buffer_end_ex  (DEC_BASE + 0x00000400)*/
/*! Slice: isp_dec_read_buffer_end_ex:*/
#define  DEC_READ_BUFFER_END_EX
#define  DEC_READ_BUFFER_END_EX_MASK 0x000000FFU
#define  DEC_READ_BUFFER_END_EX_SHIFT 0U

/*! Register: isp_dec_read_flush_cache  (DEC_BASE + 0x00000480)*/
/*! Slice: isp_dec_read_flush_cache:*/
#define  DEC_READ_FLUSH_CACHE
#define  DEC_READ_FLUSH_CACHE_MASK 0xFFFFFFFFU
#define  DEC_READ_FLUSH_CACHE_SHIFT 0U

/*! Register: isp_dec_read_flush_cache_ex  (DEC_BASE + 0x00000500)*/
/*! Slice: isp_dec_read_flush_cache_ex:*/
#define  DEC_READ_FLUSH_CACHE_EX
#define  DEC_READ_FLUSH_CACHE_EX_MASK 0xFFFFFFFFU
#define  DEC_READ_FLUSH_CACHE_EX_SHIFT 0U

/*! Register: isp_dec_write_buffer_base  (DEC_BASE + 0x00000580)*/
/*! Slice: isp_dec_write_buffer_base:*/
#define  DEC_WRITE_BUFFER_BASE
#define  DEC_WRITE_BUFFER_BASE_MASK 0xFFFFFFFFU
#define  DEC_WRITE_BUFFER_BASE_SHIFT 0U

/*! Register: isp_dec_write_buffer_base_ex  (DEC_BASE + 0x00000600)*/
/*! Slice: isp_dec_write_buffer_base_ex:*/
#define  DEC_WRITE_BUFFER_BASE_EX
#define  DEC_WRITE_BUFFER_BASE_EX_MASK 0x000000FFU
#define  DEC_WRITE_BUFFER_BASE_EX_SHIFT 0U

/*! Register: isp_dec_write_buffer_end  (DEC_BASE + 0x00000680)*/
/*! Slice: isp_dec_write_buffer_end:*/
#define  DEC_WRITE_BUFFER_END
#define  DEC_WRITE_BUFFER_END_MASK 0xFFFFFFFFU
#define  DEC_WRITE_BUFFER_END_SHIFT 0U

/*! Register: isp_dec_write_buffer_end_ex  (DEC_BASE + 0x00000700)*/
/*! Slice: isp_dec_write_buffer_end_ex:*/
#define  DEC_WRITE_BUFFER_END_EX
#define  DEC_WRITE_BUFFER_END_EX_MASK 0x000000FFU
#define  DEC_WRITE_BUFFER_END_EX_SHIFT 0U

/*! Register: isp_dec_write_flush_cache  (DEC_BASE + 0x00000780)*/
/*! Slice: isp_dec_write_flush_cache:*/
#define  DEC_WRITE_FLUSH_CACHE
#define  DEC_WRITE_FLUSH_CACHE_MASK 0xFFFFFFFFU
#define  DEC_WRITE_FLUSH_CACHE_SHIFT 0U

/*! Register: isp_dec_write_flush_cache_ex  (DEC_BASE + 0x00000800)*/
/*! Slice: isp_dec_write_flush_cache_ex:*/
#define  DEC_WRITE_FLUSH_CACHE_EX
#define  DEC_WRITE_FLUSH_CACHE_EX_MASK 0xFFFFFFFFU
#define  DEC_WRITE_FLUSH_CACHE_EX_SHIFT 0U

/*! Register: isp_dec_read_cache_base  (DEC_BASE + 0x00000880)*/
/*! Slice: isp_dec_read_cache_base:*/
#define  DEC_READ_CACHE_BASE
#define  DEC_READ_CACHE_BASE_MASK 0xFFFFFFFFU
#define  DEC_READ_CACHE_BASE_SHIFT 0U

/*! Register: isp_dec_read_cache_base_ex  (DEC_BASE + 0x00000900)*/
/*! Slice: isp_dec_read_cache_base_ex:*/
#define  DEC_READ_CACHE_BASE_EX
#define  DEC_READ_CACHE_BASE_EX_MASK 0xFFFFFFFFU
#define  DEC_READ_CACHE_BASE_EX_SHIFT 0U

/*! Register: isp_dec_write_cache_base  (DEC_BASE + 0x00000980)*/
/*! Slice: isp_dec_write_cache_base:*/
#define  DEC_WRITE_CACHE_BASE
#define  DEC_WRITE_CACHE_BASE_MASK 0xFFFFFFFFU
#define  DEC_WRITE_CACHE_BASE_SHIFT 0U

/*! Register: isp_dec_write_cache_base_ex  (DEC_BASE + 0x00000A00)*/
/*! Slice: isp_dec_write_cache_base_ex:*/
#define  DEC_WRITE_CACHE_BASE_EX
#define  DEC_WRITE_CACHE_BASE_EX_MASK 0xFFFFFFFFU
#define  DEC_WRITE_CACHE_BASE_EX_SHIFT 0U

#endif /* _MRV_DEC_DEC_ALL_REGS_H */
