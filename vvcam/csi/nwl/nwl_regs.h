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
#ifndef _NWL_REGS_H_
#define _NWL_REGS_H_

/*
 * MRV_MIPICSI1_NUM_LANES
 * Config num lanes register [3:0] rw
 * 0000b - controller off
 * 0001b - 1 Lane
 * 0010b - 2 Lanes
 * 0011b - 3 Lanes
 * 0100b - 4 Lanes
 */
#define MRV_MIPICSI_NUM_LANES 0x0

/*
 * MRV_MIPICSI1_LANES_CLK
 * Configure lanes clock [0]
 * 0b - disable
 * 1b - enable
 */
#define MRV_MIPICSI_LANES_CLK 0x4

/*
 * MRV_MIPICSI1_LANES_DATA
 * enable/disable lanes data [7:0]
 * setting bits to a '1' value enable data lane
 */
#define MRV_MIPICSI_LANES_DATA 0x8

/*
 * MRV_MIPICSI1_IGNORE_VC
 * enable/disable lanes clock [0]
 * setting bits to a '1' value enable data value
 */
#define MRV_MIPICSI_IGNORE_VC 0x80

/*
 * MRV_MIPICSI1_OUT_SHIFT
 * Configure csi_vid_out register
 */

#define MRV_MIPICSI0_CTRL 0x108240 //0x308240
#define MRV_MIPICSI1_CTRL 0x8244   //0x308244

#endif /* _NWL_REGS_H_ */
