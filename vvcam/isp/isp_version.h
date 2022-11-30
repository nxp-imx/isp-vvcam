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
#ifndef _ISP_VERSION_H_
#define _ISP_VERSION_H_

#ifdef ISP8000L_V1902
#ifndef ISP_EE
#define ISP_EE
#endif
#ifndef ISP_2DNR
#define ISP_2DNR
#endif
#ifndef ISP_3DNR
#define ISP_3DNR
#endif
#ifndef ISP_WDR_V3
#define ISP_WDR_V3
#endif
#ifndef ISP_WDR_V3_20BIT
#define ISP_WDR_V3_20BIT
#endif
#ifndef ISP_MIV2
#define ISP_MIV2
#endif
#ifndef ISP_AEV2
#define ISP_AEV2
#endif
#ifndef ISP_COMPAND
#define ISP_COMPAND
#endif
#ifndef ISPVI_EXPAND_CHAN
#define ISPVI_EXPAND_CHAN
#endif
#ifndef ISP_HDR_STITCH
#define ISP_HDR_STITCH
#endif
#endif /* ISP8000L_V1902 */

#ifdef ISP8000NANO_V1802
#ifndef ISP_WDR_V3
#define ISP_WDR_V3
#endif
#ifndef ISP_MIV1
#define ISP_MIV1
#endif
#ifndef ISP_COMPAND
#define ISP_COMPAND
#endif
#ifndef ISP_FILTER
#define ISP_FILTER
#endif
#ifndef ISP_CNR
#define ISP_CNR
#endif
#endif /* ISP8000NANO_V1802 */

#ifdef ISP8000NANO_V1801
#ifndef ISP_BLS
#define ISP_BLS
#endif
#ifndef ISP_WDR_V3
#define ISP_WDR_V3
#endif
#ifndef ISP_MIV1
#define ISP_MIV1
#endif
#ifndef ISP_GCMONO
#define ISP_GCMONO
#endif
#ifndef ISP_MI_BP
#define ISP_MI_BP
#endif
#ifndef ISP_RAWIS
#define ISP_RAWIS
#endif
#ifndef ISP_RGBGC
#define ISP_RGBGC
#endif
#ifndef ISP_HIST256
#define ISP_HIST256
#endif
#ifndef ISP_GAMMA_IN
#define ISP_GAMMA_IN
#endif
#endif /* ISP8000NANO_V1801 */

#ifdef ISP8000L_V1901
#ifndef ISP_BLS
#define ISP_BLS
#endif
#ifndef ISP_WDR_V2
#define ISP_WDR_V2
#endif
#ifndef ISP_MIV1
#define ISP_MIV1
#endif
#ifndef ISP_MI_BP
#define ISP_MI_BP
#endif
#ifndef ISP_GAMMA_IN
#define ISP_GAMMA_IN
#endif
#endif /* ISP8000L_V1901 */

#ifdef ISP8000L_V1903
#ifndef ISP_CNR
#define ISP_CNR
#endif
#ifndef ISP_EE
#define ISP_EE
#endif
#ifndef ISP_2DNR
#define ISP_2DNR
#endif
#ifndef ISP_WDR_V3
#define ISP_WDR_V3
#endif
#ifndef ISP_WDR_V3_20BIT
#define ISP_WDR_V3_20BIT
#endif
#ifndef ISP_MIV2
#define ISP_MIV2
#endif
#ifndef ISP_AEV2
#define ISP_AEV2
#endif
#ifndef ISP_AE_SHADOW
#define ISP_AE_SHADOW
#endif
#ifndef ISP_COMPAND
#define ISP_COMPAND
#endif
#ifndef ISPVI_EXPAND_CHAN
#define ISPVI_EXPAND_CHAN
#endif
#ifndef ISP_FILTER
#define ISP_FILTER
#endif
#endif /* ISP8000L_V1903 */

#ifdef ISP8000L_V1905
#ifndef ISP_EE
#define ISP_EE
#endif
#ifndef ISP_2DNR
#define ISP_2DNR
#endif
#ifndef ISP_3DNR
#define ISP_3DNR
#endif
#ifndef ISP_WDR_V3
#define ISP_WDR_V3
#endif
#ifndef ISP_WDR_V3_20BIT
#define ISP_WDR_V3_20BIT
#endif
#ifndef ISP_MIV2
#define ISP_MIV2
#endif
#ifndef ISP_AEV2
#define ISP_AEV2
#endif
#ifndef ISP_AE_SHADOW
#define ISP_AE_SHADOW
#endif
#ifndef ISP_COMPAND
#define ISP_COMPAND
#endif
#ifndef ISPVI_EXPAND_CHAN
#define ISPVI_EXPAND_CHAN
#endif
#endif /* ISP8000L_V1905 */

#ifdef ISP8000L_V2001
#ifndef ISP_EE
#define ISP_EE
#endif
#ifndef ISP_2DNR
#define ISP_2DNR
#endif
#ifndef ISP_3DNR
#define ISP_3DNR
#endif
#ifndef ISP_WDR_V3
#define ISP_WDR_V3
#endif
#ifndef ISP_WDR_V3_20BIT
#define ISP_WDR_V3_20BIT
#endif
#ifndef ISP_MIV2
#define ISP_MIV2
#endif
#ifndef ISP_AEV2
#define ISP_AEV2
#endif
#ifndef ISP_COMPAND
#define ISP_COMPAND
#endif
#ifndef ISPVI_EXPAND_CHAN
#define ISPVI_EXPAND_CHAN
#endif
#ifndef ISP_HDR_STITCH
#define ISP_HDR_STITCH
#endif
#ifndef ISP_FILTER
#define ISP_FILTER
#endif
#endif /* ISP8000L_V2001 */

#ifdef ISP8000_V1901
#ifndef ISP_3DNR
#define ISP_3DNR
#endif
#ifndef ISP_WDR_V3
#define ISP_WDR_V3
#endif
#ifndef ISP_WDR_V3_20BIT
#define ISP_WDR_V3_20BIT
#endif
#ifndef ISP_MIV2
#define ISP_MIV2
#endif
#ifndef ISP_AEV2
#define ISP_AEV2
#endif
#ifndef ISP_COMPAND
#define ISP_COMPAND
#endif
#ifndef ISPVI_EXPAND_CHAN
#define ISPVI_EXPAND_CHAN
#endif
#ifndef ISP_HDR_STITCH
#define ISP_HDR_STITCH
#endif
#ifndef ISP_FILTER
#define ISP_FILTER
#endif
#endif /* ISP8000_V1901 */

#ifdef ISP8000L_V2002
#ifndef ISP_EE
#define ISP_EE
#endif
#ifndef ISP_2DNR
#define ISP_2DNR
#endif
#ifndef ISP_3DNR
#define ISP_3DNR
#endif
#ifndef ISP_WDR_V3
#define ISP_WDR_V3
#endif
#ifndef ISP_WDR_V3_20BIT
#define ISP_WDR_V3_20BIT
#endif
#ifndef ISP_MIV2
#define ISP_MIV2
#endif
#ifndef ISP_AEV2
#define ISP_AEV2
#endif
#ifndef ISP_COMPAND
#define ISP_COMPAND
#endif
#ifndef ISPVI_EXPAND_CHAN
#define ISPVI_EXPAND_CHAN
#endif
#ifndef ISP_HDR_STITCH
#define ISP_HDR_STITCH
#endif
#ifndef ISP_FILTER
#define ISP_FILTER
#endif
#endif /* ISP8000L_V2002 */

#ifdef ISP8000_V2003
#ifndef ISP_EE
#define ISP_EE
#endif
#ifndef ISP_2DNR
#define ISP_2DNR
#endif
#ifndef ISP_3DNR
#define ISP_3DNR
#endif
#ifndef ISP_WDR_V3
#define ISP_WDR_V3
#endif
#ifndef ISP_WDR_V3_20BIT
#define ISP_WDR_V3_20BIT
#endif
#ifndef ISP_MIV2
#define ISP_MIV2
#endif
#ifndef ISP_AEV2
#define ISP_AEV2
#endif
#ifndef ISP_AE_SHADOW
#define ISP_AE_SHADOW
#endif
#ifndef ISP_COMPAND
#define ISP_COMPAND
#endif
#ifndef ISPVI_EXPAND_CHAN
#define ISPVI_EXPAND_CHAN
#endif
#ifndef ISP_3DNR_DDR_LESS
#define ISP_3DNR_DDR_LESS
#endif
#ifndef ISP_FILTER
#define ISP_FILTER
#endif
#ifndef ISP_DEMOSAIC2
#define ISP_DEMOSAIC2
#endif
#ifndef ISP_HDR_STITCH
#define ISP_HDR_STITCH
#endif
#endif /* ISP8000_V2003 */

#define ISP_EE_SUPPORT 1
#define ISP_2DNR_SUPPORT (1 << 1)
#define ISP_3DNR_SUPPORT (1 << 2)
#define ISP_WDR3_SUPPORT (1 << 3)
#define ISP_MIV2_SUPPORT (1 << 4)
#define ISP_AEV2_SUPPORT (1 << 5)
#define ISP_COMPAND_SUPPORT (1 << 6)
#define ISP_HDR_STITCH_SUPPORT (1 << 7)


/*****************************************************************************/
/**
 * @brief   HDR module version type
 */
/*****************************************************************************/
#define VSI_ISP_HDR_NOTSUPPORT  0   /*!< Not support HDR */
#define VSI_ISP_HDR_V10         1   /*!< Version V10 */
#define VSI_ISP_HDR_V11         2   /*!< Version V11 */
#define VSI_ISP_HDR_V12         3   /*!< Version V12 */
#define VSI_ISP_HDR_V13         4   /*!< Version V13 */


/***************************************
    add Version defines
***************************************/
#ifdef ISP8000NANO_V1801
#ifdef ISP_EE
#define MRV_EE_VERSION                  1
#endif
#ifdef ISP_GCMONO
#define MRV_GCMONO_VERSION              1
#endif
#define MRV_AUTO_EXPOSURE_VERSION       1
#define MRV_BLACK_LEVEL_VERSION         1
#define MRV_OUTPUT_FORMATTER_VERSION    1
#define MRV_GAMMA_OUT_VERSION           1
#define MRV_FLASH_LIGHT_VERSION         1
#define MRV_SHUTTER_VERSION             1
#define MRV_MAINPATH_SCALER_VERSION     1
#define MRV_SELFPATH_SCALER_VERSION     1
#define MRV_MI_VERSION                  1
#define MRV_JPE_VERSION                 1
#define MRV_SMIA_VERSION                1
#define MRV_MIPI_VERSION                1
#define MRV_AUTOFOCUS_VERSION           1
#define MRV_LSC_VERSION                 1
#define MRV_IS_VERSION                  1
#define MRV_HISTOGRAM_VERSION           1
#define MRV_FILTER_VERSION              1
#define MRV_CAC_VERSION                 1
#define MRV_DPF_VERSION                 1
#define MRV_DPCC_VERSION                1
#define MRV_WDR_VERSION                 1
#define MRV_CSM_VERSION                 1
#define MRV_AWB_VERSION                 1
#define MRV_ELAWB_VERSION               1
#define MRV_GAMMA_IN_VERSION            1
#define MRV_SHUTTER_CTRL_VERSION        1
#define MRV_CT_VERSION                  1
#define MRV_COLOR_PROCESSING_VERSION    1
#define MRV_CNR_VERSION                 1
#define MRV_TPG_VERSION                 1

#elif defined ISP8000NANO_V1802
#define MRV_SUPER_IMPOSE_VERSION        1
#define MRV_CMPD_VERSION                1
#ifdef ISP_HDR_STITCH
#undef MRV_STITCHING_VERSION
#define MRV_STITCHING_VERSION           1    /* 1--V11, 2--V12 */
#endif
#define MRV_WDR3_VERSION                1
#ifdef ISP_GCMONO
#define MRV_GCMONO_VERSION              1
#endif
#define MRV_AUTO_EXPOSURE_VERSION       1
#define MRV_BLACK_LEVEL_VERSION         1
#define MRV_OUTPUT_FORMATTER_VERSION    1
#define MRV_GAMMA_OUT_VERSION           1
#define MRV_FLASH_LIGHT_VERSION         1
#define MRV_SHUTTER_VERSION             1
#define MRV_MAINPATH_SCALER_VERSION     1
#define MRV_SELFPATH_SCALER_VERSION     1
#define MRV_MI_VERSION                  1
#define MRV_JPE_VERSION                 1
#define MRV_SMIA_VERSION                1
#define MRV_MIPI_VERSION                1
#define MRV_AUTOFOCUS_VERSION           1
#define MRV_LSC_VERSION                 1
#define MRV_IS_VERSION                  1
#define MRV_HISTOGRAM_VERSION           1
#define MRV_FILTER_VERSION              1
#define MRV_CAC_VERSION                 1
#define MRV_DPF_VERSION                 1
#define MRV_DPCC_VERSION                1
#define MRV_WDR_VERSION                 1
#define MRV_CSM_VERSION                 1
#define MRV_AWB_VERSION                 1
#define MRV_ELAWB_VERSION               1
#define MRV_GAMMA_IN_VERSION            1
#define MRV_SHUTTER_CTRL_VERSION        1
#define MRV_CT_VERSION                  1
#define MRV_COLOR_PROCESSING_VERSION    1
#define MRV_CNR_VERSION                 1
#define MRV_TPG_VERSION                 1

#elif defined ISP8000_V1901
#define MRV_IMAGE_EFFECTS_VERSION       1
#ifdef ISP_3DNR
#define MRV_3DNR_VERSION                1
#endif
#define MRV_SUPER_IMPOSE_VERSION        1
#define MRV_CMPD_VERSION                1
#ifdef ISP_HDR_STITCH
#undef MRV_STITCHING_VERSION
#define MRV_STITCHING_VERSION           1    /* 1--V11, 2--V12  */
#endif
#define MRV_AEV2_VERSION                1
#define MRV_WDR3_VERSION                1
#ifdef ISP_GCMONO
#define MRV_GCMONO_VERSION              1
#endif
#define MRV_AUTO_EXPOSURE_VERSION       1
#define MRV_BLACK_LEVEL_VERSION         1
#define MRV_OUTPUT_FORMATTER_VERSION    1
#define MRV_GAMMA_OUT_VERSION           1
#define MRV_FLASH_LIGHT_VERSION         1
#define MRV_SHUTTER_VERSION             1
#define MRV_MAINPATH_SCALER_VERSION     1
#define MRV_SELFPATH_SCALER_VERSION     1
#define MRV_MI_VERSION                  1
#define MRV_JPE_VERSION                 1
#define MRV_SMIA_VERSION                1
#define MRV_MIPI_VERSION                1
#define MRV_AUTOFOCUS_VERSION           1
#define MRV_LSC_VERSION                 1
#define MRV_IS_VERSION                  1
#define MRV_HISTOGRAM_VERSION           1
#define MRV_FILTER_VERSION              1
#define MRV_CAC_VERSION                 1
#define MRV_DPF_VERSION                 1
#define MRV_DPCC_VERSION                1
#define MRV_WDR_VERSION                 1
#define MRV_CSM_VERSION                 1
#define MRV_AWB_VERSION                 1
#define MRV_ELAWB_VERSION               1
#define MRV_GAMMA_IN_VERSION            1
#define MRV_SHUTTER_CTRL_VERSION        1
#define MRV_CT_VERSION                  1
#define MRV_COLOR_PROCESSING_VERSION    1
#define MRV_CNR_VERSION                 1
#define MRV_TPG_VERSION                 1

#elif defined ISP8000L_V1901
#define MRV_IMAGE_EFFECTS_VERSION       1
#ifdef ISP_3DNR
#define MRV_3DNR_VERSION                1
#endif
#define MRV_SUPER_IMPOSE_VERSION        1
#define MRV_CMPD_VERSION                1
#ifdef ISP_HDR_STITCH
#undef MRV_STITCHING_VERSION
#define MRV_STITCHING_VERSION           1    /* 1--V11, 2--V12 */
#endif
#define MRV_SP2_VERSION                 1
#define MRV_AEV2_VERSION                1
#define MRV_WDR3_VERSION                1
#ifdef ISP_GCMONO
#define MRV_GCMONO_VERSION              1
#endif
#define MRV_AUTO_EXPOSURE_VERSION       1
#define MRV_BLACK_LEVEL_VERSION         1
#define MRV_OUTPUT_FORMATTER_VERSION    1
#define MRV_GAMMA_OUT_VERSION           1
#define MRV_FLASH_LIGHT_VERSION         1
#define MRV_SHUTTER_VERSION             1
#define MRV_MAINPATH_SCALER_VERSION     1
#define MRV_SELFPATH_SCALER_VERSION     1
#define MRV_MI_VERSION                  1
#define MRV_JPE_VERSION                 1
#define MRV_SMIA_VERSION                1
#define MRV_MIPI_VERSION                1
#define MRV_AUTOFOCUS_VERSION           1
#define MRV_LSC_VERSION                 1
#define MRV_IS_VERSION                  1
#define MRV_HISTOGRAM_VERSION           1
#define MRV_FILTER_VERSION              1
#define MRV_CAC_VERSION                 1
#define MRV_DPF_VERSION                 1
#define MRV_DPCC_VERSION                1
#define MRV_WDR_VERSION                 1
#define MRV_CSM_VERSION                 1
#define MRV_AWB_VERSION                 1
#define MRV_ELAWB_VERSION               1
#define MRV_GAMMA_IN_VERSION            1
#define MRV_SHUTTER_CTRL_VERSION        1
#define MRV_CT_VERSION                  1
#define MRV_COLOR_PROCESSING_VERSION    1
#define MRV_CNR_VERSION                 1
#define MRV_TPG_VERSION                 1

#elif defined ISP8000L_V1902
#define MRV_IMAGE_EFFECTS_VERSION       1
#ifdef ISP_3DNR
#define MRV_3DNR_VERSION                1
#endif
#ifdef ISP_VSM
#define MRV_VSM_VERSION                 1
#endif
#ifdef ISP_EE
#define MRV_EE_VERSION                  1
#endif
#ifdef ISP_2DNR
#define MRV_2DNR_VERSION                1
#endif
#define MRV_SUPER_IMPOSE_VERSION        1
#define MRV_CMPD_VERSION                1
#ifdef ISP_HDR_STITCH
#undef MRV_STITCHING_VERSION
#define MRV_STITCHING_VERSION           1    /* 1--V11, 2--V12 */
#endif
#define MRV_SP2_VERSION                 1
#define MRV_AEV2_VERSION                1
#define MRV_WDR3_VERSION                1
#ifdef ISP_GCMONO
#define MRV_GCMONO_VERSION              1
#endif
#define MRV_AUTO_EXPOSURE_VERSION       1
#define MRV_BLACK_LEVEL_VERSION         1
#define MRV_OUTPUT_FORMATTER_VERSION    1
#define MRV_GAMMA_OUT_VERSION           1
#define MRV_FLASH_LIGHT_VERSION         1
#define MRV_SHUTTER_VERSION             1
#define MRV_MAINPATH_SCALER_VERSION     1
#define MRV_SELFPATH_SCALER_VERSION     1
#define MRV_MI_VERSION                  1
#define MRV_JPE_VERSION                 1
#define MRV_SMIA_VERSION                1
#define MRV_MIPI_VERSION                1
#define MRV_AUTOFOCUS_VERSION           1
#define MRV_LSC_VERSION                 1
#define MRV_IS_VERSION                  1
#define MRV_HISTOGRAM_VERSION           1
#define MRV_FILTER_VERSION              1
#define MRV_CAC_VERSION                 1
#define MRV_DPF_VERSION                 1
#define MRV_DPCC_VERSION                1
#define MRV_WDR_VERSION                 1
#define MRV_CSM_VERSION                 1
#define MRV_AWB_VERSION                 1
#define MRV_ELAWB_VERSION               1
#define MRV_GAMMA_IN_VERSION            1
#define MRV_SHUTTER_CTRL_VERSION        1
#define MRV_CT_VERSION                  1
#define MRV_COLOR_PROCESSING_VERSION    1
#define MRV_CNR_VERSION                 1
#define MRV_TPG_VERSION                 1

#elif defined ISP8000L_V1903
#ifdef ISP_VSM
#define MRV_VSM_VERSION                 1
#endif
#ifdef ISP_EE
#define MRV_EE_VERSION                  1
#endif
#ifdef ISP_2DNR
#define MRV_2DNR_VERSION                1
#endif
#define MRV_SUPER_IMPOSE_VERSION        1
#define MRV_CMPD_VERSION                1
#ifdef ISP_HDR_STITCH
#undef MRV_STITCHING_VERSION
#define MRV_STITCHING_VERSION           2    /* 1--V11, 2--V12 */
#endif
#define MRV_SP2_VERSION                 1
#define MRV_AEV2_VERSION                1
#define MRV_WDR3_VERSION                1
#ifdef ISP_GCMONO
#define MRV_GCMONO_VERSION              1
#endif
#define MRV_AUTO_EXPOSURE_VERSION       1
#define MRV_BLACK_LEVEL_VERSION         1
#define MRV_OUTPUT_FORMATTER_VERSION    1
#define MRV_GAMMA_OUT_VERSION           1
#define MRV_FLASH_LIGHT_VERSION         1
#define MRV_SHUTTER_VERSION             1
#define MRV_MAINPATH_SCALER_VERSION     1
#define MRV_SELFPATH_SCALER_VERSION     1
#define MRV_MI_VERSION                  1
#define MRV_JPE_VERSION                 1
#define MRV_SMIA_VERSION                1
#define MRV_MIPI_VERSION                1
#define MRV_AUTOFOCUS_VERSION           1
#define MRV_LSC_VERSION                 1
#define MRV_IS_VERSION                  1
#define MRV_HISTOGRAM_VERSION           1
#define MRV_FILTER_VERSION              1
#define MRV_CAC_VERSION                 1
#define MRV_DPF_VERSION                 1
#define MRV_DPCC_VERSION                1
#define MRV_WDR_VERSION                 1
#define MRV_CSM_VERSION                 1
#define MRV_AWB_VERSION                 1
#define MRV_ELAWB_VERSION               1
#define MRV_GAMMA_IN_VERSION            1
#define MRV_SHUTTER_CTRL_VERSION        1
#define MRV_CT_VERSION                  1
#define MRV_COLOR_PROCESSING_VERSION    1
#define MRV_CNR_VERSION                 1
#define MRV_TPG_VERSION                 1

#elif defined ISP8000L_V1905
#ifdef ISP_3DNR
#define MRV_3DNR_VERSION                1
#endif
#ifdef ISP_VSM
#define MRV_VSM_VERSION                 1
#endif
#ifdef ISP_EE
#define MRV_EE_VERSION                  1
#endif
#ifdef ISP_2DNR
#define MRV_2DNR_VERSION                1
#endif
#define MRV_SUPER_IMPOSE_VERSION        1
#define MRV_CMPD_VERSION                1
#ifdef ISP_HDR_STITCH
#undef MRV_STITCHING_VERSION
#define MRV_STITCHING_VERSION           2    /* 1--V11, 2--V12 */
#endif
#define MRV_SP2_VERSION                 1
#define MRV_AEV2_VERSION                1
#ifdef ISP_GCMONO
#define MRV_GCMONO_VERSION              1
#endif
#define MRV_AUTO_EXPOSURE_VERSION       1
#define MRV_BLACK_LEVEL_VERSION         1
#define MRV_OUTPUT_FORMATTER_VERSION    1
#define MRV_GAMMA_OUT_VERSION           1
#define MRV_FLASH_LIGHT_VERSION         1
#define MRV_SHUTTER_VERSION             1
#define MRV_MAINPATH_SCALER_VERSION     1
#define MRV_SELFPATH_SCALER_VERSION     1
#define MRV_MI_VERSION                  1
#define MRV_JPE_VERSION                 1
#define MRV_SMIA_VERSION                1
#define MRV_MIPI_VERSION                1
#define MRV_AUTOFOCUS_VERSION           1
#define MRV_LSC_VERSION                 1
#define MRV_IS_VERSION                  1
#define MRV_HISTOGRAM_VERSION           1
#define MRV_FILTER_VERSION              1
#define MRV_CAC_VERSION                 1
#define MRV_DPF_VERSION                 1
#define MRV_DPCC_VERSION                1
#define MRV_WDR_VERSION                 1
#define MRV_CSM_VERSION                 1
#define MRV_AWB_VERSION                 1
#define MRV_ELAWB_VERSION               1
#define MRV_GAMMA_IN_VERSION            1
#define MRV_SHUTTER_CTRL_VERSION        1
#define MRV_CT_VERSION                  1
#define MRV_COLOR_PROCESSING_VERSION    1
#define MRV_CNR_VERSION                 1
#define MRV_TPG_VERSION                 1

#elif defined ISP8000L_V2001
#define MRV_IMAGE_EFFECTS_VERSION       1
#ifdef ISP_3DNR
#define MRV_3DNR_VERSION                1
#endif
#ifdef ISP_VSM
#define MRV_VSM_VERSION                 1
#endif
#ifdef ISP_EE
#define MRV_EE_VERSION                  1
#endif
#ifdef ISP_2DNR
#define MRV_2DNR_VERSION                1
#endif
#define MRV_SUPER_IMPOSE_VERSION        1
#define MRV_CMPD_VERSION                1
#ifdef ISP_HDR_STITCH
#undef MRV_STITCHING_VERSION
#define MRV_STITCHING_VERSION           2    /* 1--V11, 2--V12 */
#endif
#define MRV_SP2_VERSION                 1
#define MRV_AEV2_VERSION                1
#define MRV_WDR3_VERSION                1
#ifdef ISP_GCMONO
#define MRV_GCMONO_VERSION              1
#endif
#define MRV_AUTO_EXPOSURE_VERSION       1
#define MRV_BLACK_LEVEL_VERSION         1
#define MRV_OUTPUT_FORMATTER_VERSION    1
#define MRV_GAMMA_OUT_VERSION           1
#define MRV_FLASH_LIGHT_VERSION         1
#define MRV_SHUTTER_VERSION             1
#define MRV_MAINPATH_SCALER_VERSION     1
#define MRV_SELFPATH_SCALER_VERSION     1
#define MRV_MI_VERSION                  1
#define MRV_JPE_VERSION                 1
#define MRV_SMIA_VERSION                1
#define MRV_MIPI_VERSION                1
#define MRV_AUTOFOCUS_VERSION           1
#define MRV_LSC_VERSION                 1
#define MRV_IS_VERSION                  1
#define MRV_HISTOGRAM_VERSION           1
#define MRV_FILTER_VERSION              1
#define MRV_CAC_VERSION                 1
#define MRV_DPF_VERSION                 1
#define MRV_DPCC_VERSION                1
#define MRV_WDR_VERSION                 1
#define MRV_CSM_VERSION                 1
#define MRV_AWB_VERSION                 1
#define MRV_ELAWB_VERSION               1
#define MRV_GAMMA_IN_VERSION            1
#define MRV_SHUTTER_CTRL_VERSION        1
#define MRV_CT_VERSION                  1
#define MRV_COLOR_PROCESSING_VERSION    1
#define MRV_CNR_VERSION                 1
#define MRV_TPG_VERSION                 1

#elif defined(ISP8000L_V2002)
#define MRV_IMAGE_EFFECTS_VERSION       1
#ifdef ISP_3DNR
#define MRV_3DNR_VERSION                1
#endif
#ifdef ISP_VSM
#define MRV_VSM_VERSION                 1
#endif
#ifdef ISP_EE
#define MRV_EE_VERSION                  1
#endif
#ifdef ISP_2DNR
#define MRV_2DNR_VERSION                1
#endif
#define MRV_SUPER_IMPOSE_VERSION        1
#define MRV_CMPD_VERSION                1
#ifdef ISP_HDR_STITCH
#undef MRV_STITCHING_VERSION
#define MRV_STITCHING_VERSION           2    /* 1--V11, 2--V12 */
#endif
#define MRV_SP2_VERSION                 1
#define MRV_AEV2_VERSION                1
#define MRV_WDR3_VERSION                1
#ifdef ISP_GCMONO
#define MRV_GCMONO_VERSION              1
#endif
#define MRV_AUTO_EXPOSURE_VERSION       1
#define MRV_BLACK_LEVEL_VERSION         1
#define MRV_OUTPUT_FORMATTER_VERSION    1
#define MRV_GAMMA_OUT_VERSION           1
#define MRV_FLASH_LIGHT_VERSION         1
#define MRV_SHUTTER_VERSION             1
#define MRV_MAINPATH_SCALER_VERSION     1
#define MRV_SELFPATH_SCALER_VERSION     1
#define MRV_MI_VERSION                  1
#define MRV_JPE_VERSION                 1
#define MRV_SMIA_VERSION                1
#define MRV_MIPI_VERSION                1
#define MRV_AUTOFOCUS_VERSION           1
#define MRV_LSC_VERSION                 1
#define MRV_IS_VERSION                  1
#define MRV_HISTOGRAM_VERSION           1
#define MRV_FILTER_VERSION              1
#define MRV_CAC_VERSION                 1
#define MRV_DPF_VERSION                 1
#define MRV_DPCC_VERSION                1
#define MRV_WDR_VERSION                 1
#define MRV_CSM_VERSION                 1
#define MRV_AWB_VERSION                 1
#define MRV_ELAWB_VERSION               1
#define MRV_GAMMA_IN_VERSION            1
#define MRV_SHUTTER_CTRL_VERSION        1
#define MRV_CT_VERSION                  1
#define MRV_COLOR_PROCESSING_VERSION    1
#define MRV_CNR_VERSION                 1
#define MRV_TPG_VERSION                 1

#elif defined ISP8000_V2003
#define MRV_IMAGE_EFFECTS_VERSION       1
#ifdef ISP_3DNR
#define MRV_3DNR_VERSION                1
#endif
#ifdef ISP_VSM
#define MRV_VSM_VERSION                 1
#endif
#ifdef ISP_EE
#define MRV_EE_VERSION                  1
#endif
#ifdef ISP_2DNR
#define MRV_2DNR_VERSION                1
#endif
#ifdef ISP_HDR_STITCH
#undef MRV_STITCHING_VERSION
#define MRV_STITCHING_VERSION           1    /* 1--V11, 2--V12 */
#endif

#define MRV_WDR3_VERSION                1
#define MRV_CMPD_VERSION                1
#define MRV_AUTO_EXPOSURE_VERSION       1
#define MRV_OUTPUT_FORMATTER_VERSION    1
#define MRV_GAMMA_OUT_VERSION           1
#define MRV_FLASH_LIGHT_VERSION         1
#define MRV_SHUTTER_VERSION             1
#define MRV_MAINPATH_SCALER_VERSION     1
#define MRV_MI_VERSION                  1
#define MRV_JPE_VERSION                 1
#define MRV_SMIA_VERSION                1
#define MRV_MIPI_VERSION                1
#define MRV_AUTOFOCUS_VERSION           1
#define MRV_LSC_VERSION                 1
#define MRV_IS_VERSION                  1
#define MRV_HISTOGRAM_VERSION           1
#ifdef ISP_FILTER
#define MRV_FILTER_VERSION              1
#endif
#define MRV_CAC_VERSION                 1
#define MRV_DPCC_VERSION                1
#define MRV_WDR_VERSION                 1
#define MRV_CSM_VERSION                 1
#define MRV_AWB_VERSION                 1
#define MRV_ELAWB_VERSION               1
#define MRV_SHUTTER_CTRL_VERSION        1
#define MRV_CT_VERSION                  1
#define MRV_COLOR_PROCESSING_VERSION    1
#define MRV_CNR_VERSION                 1
#define MRV_TPG_VERSION                 1

#elif defined(ISP8000L_V2006)
#ifdef ISP_HDR_STITCH
#undef MRV_STITCHING_VERSION
#define MRV_STITCHING_VERSION           1    /* 1--V11, 2--V12 */
#endif

#endif

#endif /* _ISP_VERSION_H_ */
