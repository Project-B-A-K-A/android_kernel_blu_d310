/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/****************************************
History:
	

1. lidong.zhou@ragentek.com    2013.4.7
   Description: add mipi  djn otm8018b


---------------------------------------------------------------------------------
VENDOR	|   Driver IC   |    LCD_ID	|  Module N/O	        |   Range	|
---------------------------------------------------------------------------------
DJN	|   OTM8108B	|	0V	|		 	|   0.0-0.1	|	
---------------------------------------------------------------------------------
TRULY   |   OTM8108B	|	1.5V	|		        |   1.4-1,6	|
---------------------------------------------------------------------------------
        |               |		|		        |		|
---------------------------------------------------------------------------------
        |   		|		|   			|		|
---------------------------------------------------------------------------------
****************************************/

#ifdef BUILD_LK
#else
#include <linux/string.h>
#if defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#endif
#endif
#include "lcm_drv.h"
#include <cust_adc.h>    	// zhoulidong  add for lcm detect
// zhoulidong  add for lcm detect ,read adc voltage
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)


#define MIN_VOLTAGE (0)     // zhoulidong  add for djn lcm detect
#define MAX_VOLTAGE (100)     // zhoulidong  add for djn lcm detect
#define COMPARE_BY_ADC   1

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER

#define LCM_ID_OTM8018B	0x8009

#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define   TRUE     1
#endif
 
#ifndef FALSE
    #define   FALSE    0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

 struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[128];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/

	//Enable EXTC
	{0xFF,3,{0x80,0x09,0x01}}, 
	//Enable Orise mode                                                                                                                                                                                                                                                                                                     
	{0x00,1,{0x80}},                                                                                                                                                                                                                                                                                                                   
	{0xFF,2,{0x80,0x09}},
	//SPI CMD2 READ												                                                                                                                                                                                                      
	{0x00,1,{0x03}}, 										                                                                                                                                                                                                                                      
	{0xFF,1,{0x01}}, 
	//											                                                                                                                                                                                                                      
	{0x00,1,{0xB4}}, 										                                                                                                                                                                                                                                      
	{0xC0,1,{0x10}}, 											                                                                                                                                                                                                                              
	{0x00,1,{0x82}}, 										                                                                                                                                                                                                                                      
	{0xC5,1,{0xA3}}, 

	{0x00,1,{0x90}}, 										                                                                                                                                                                                                                                      
	{0xC5,1,{0x96}}, 
	//VGL,VGH										                                                                                                                                                                                                              
	{0x00,1,{0x91}}, 										                                                                                                                                                                                                                                      
	{0xC5,1,{0x76}}, 
	//GVDD,NGVDD										                                                                                                                                                                                                                              
	{0x00,1,{0x00}}, 										                                                                                                                                                                                                                                      
	{0xD8,2,{0x83,0x85}}, 
	//VCOM												                                                                                                                                                                                                              
	{0x00,1,{0x00}}, 										                                                                                                                                                                                                                                      
	{0xD9,1,{0x60}}, 
	//GAMMA SETTING
	{0x00,1,{0x00}}, 							                                                                                                                                                                                                                                                              
	{0xE1,16,{0x02,0x04,0x0a,0x0e,0x07,0x1d,0x0f,0x0f,0x00,0x04,0x02,0x06,0x0e,0x23,0x20,0x17}}, 												                                                                                                                                      
	{0x00,1,{0x00}}, 							                                                                                                                                                                                                                                                              
	{0xE2,16,{0x02,0x04,0x0a,0x0e,0x07,0x1d,0x0f,0x0f,0x00,0x04,0x02,0x06,0x0e,0x23,0x20,0x17}},                                                                                                                                                                                                                                        
	//FRAME RATE												                                                                                                                                                                                                                      
	{0x00,1,{0x81}}, 										                                                                                                                                                                                                                                      
	{0xC1,7,{0x66,0x0E,0x08,0x04,0x00,0x02,0x30}}, 
	//Display On Pump setting													                                                                                                                                                                                                              
	{0x00,1,{0xA6}}, 										                                                                                                                                                                                                                                      
	{0xC1,1,{0x01}}, 													                                                                                                                                                                                                              
	{0x00,1,{0xC0}}, 										                                                                                                                                                                                                                                      
	{0xC5,1,{0x00}}, 								                                                                                                                                                                                                                                                      
	{0x00,1,{0x8B}}, 										                                                                                                                                                                                                                                      
	{0xB0,1,{0x40}}, 	
	//VRGH Disable												                                                                                                                                                                                                              
	{0x00,1,{0xB2}}, 										                                                                                                                                                                                                                                      
	{0xF5,4,{0x15,0x00,0x15,0x00}},  
	//VRGH minimum                                                                                                                                                                                                                                                                                                              
	{0x00,1,{0x93}},                                                                                                                                                                                                                                                                                                                   
	{0xC5,1,{0x03}},                                                                                                                                                                                                                                                                                                                   
	{0x00,1,{0x81}}, 										                                                                                                                                                                                                                                      
	{0xC4,1,{0x83}},                                                                                                                                                                                                                                                                                                                   
	{0x00,1,{0x92}}, 										                                                                                                                                                                                                                                      
	{0xC5,1,{0x01}},                                                                                                                                                                                                                                                                                                                   
	{0x00,1,{0xB1}}, 										                                                                                                                                                                                                                                      
	{0xC5,1,{0xA9}}, 													                                                                                                                                                                                                              
	{0x00,1,{0x92}}, 										                                                                                                                                                                                                                                      
	{0xB3,1,{0x45}},   
	{0x00,1,{0x90}}, 										                                                                                                                                                                                                                                      
	{0xB3,1,{0x02}}, 
	//C08x   
	{0x00,1,{0x80}}, 										                                                                                                                                                                                                                                      
	{0xC0,5,{0x00,0x58,0x00,0x14,0x16}},  
	//C09X
	{0x00,1,{0x90}}, 										                                                                                                                                                                                                                                      
	{0xC0,6,{0x00,0x56,0x00,0x00,0x00,0x03}}, 
	{0x00,1,{0xA6}}, 										                                                                                                                                                                                                                                      
	{0xC1,3,{0x00,0x00,0x00}}, 
	//CE8X <TCON_GOA_WAVE>
	{0x00,1,{0x80}}, 										                                                                                                                                                                                                                                      
	{0xCE,12,{0x87,0x03,0x00,0x85,0x03,0x00,0x86,0x03,0x00,0x84,0x03,0x00}}, 
	//CEAx
	{0x00,1,{0xA0}}, 										                                                                                                                                                                                                                                      
	{0xCE,14,{0x38,0x03,0x03,0x58,0x00,0x00,0x00,0x38,0x02,0x03,0x59,0x00,0x00,0x00}}, 
	//CEBx
	{0x00,1,{0xB0}}, 										                                                                                                                                                                                                                                      
	{0xCE,14,{0x38,0x01,0x03,0x5A,0x00,0x00,0x00,0x38,0x00,0x03,0x5B,0x00,0x00,0x00}}, 
	//CECx
	{0x00,1,{0xC0}}, 										                                                                                                                                                                                                                                      
	{0xCE,14,{0x30,0x00,0x03,0x5C,0x00,0x00,0x00,0x30,0x01,0x03,0x5D,0x00,0x00,0x00}}, 
	//CEDx
	{0x00,1,{0xD0}}, 										                                                                                                                                                                                                                                      
	{0xCE,14,{0x30,0x02,0x03,0x5E,0x00,0x00,0x00,0x30,0x03,0x03,0x5F,0x00,0x00,0x00}}, 
	//CFCx 
	{0x00,1,{0xC7}},                                                                                                                                                                                                                                                                                                                   
	{0xCF,1,{0x00}},  
	{0x00,1,{0xC9}},                                                                                                                                                                                                                                                                                                                   
	{0xCF,1,{0x00}},  
	//CBCx
	{0x00,1,{0xC4}}, 										                                                                                                                                                                                                                                      
	{0xCB,6,{0x04,0x04,0x04,0x04,0x04,0x04}}, 
	//CBDX 
	{0x00,1,{0xD9}}, 										                                                                                                                                                                                                                                      
	{0xCB,6,{0x04,0x04,0x04,0x04,0x04,0x04}}, 
	//CC8x 
	{0x00,1,{0x84}}, 										                                                                                                                                                                                                                                      
	{0xCC,7,{0x0C,0x0A,0x10,0x0E,0x03,0x04,0x0B}}, 
	//CCAx 
	{0x00,1,{0xA0}}, 										                                                                                                                                                                                                                                      
	{0xCC,5,{0x09,0x0F,0x0D,0x01,0x02}}, 
	//CCBx 
	{0x00,1,{0xB4}}, 										                                                                                                                                                                                                                                      
	{0xCC,6,{0x0D,0x0F,0x09,0x0B,0x02,0x01}},
	//CCCx
	{0x00,1,{0xCE}},
	{0xCC,1,{0x0E}},  
	//CCDx
	{0x00,1,{0xD0}}, 										                                                                                                                                                                                                                                      
	{0xCC,5,{0x10,0x0A,0x0C,0x04,0x03}},
	//24bit mode
	//{0x00,0x00}},
	{0x3A,1,{0x77}},  
	{0x11,	1,	{0x00}},
	{REGFLAG_DELAY, 150, {}},///120
	//{REGFLAG_DELAY, 100, {}},
	{0x29,	1,	{0x00}},
	
	{REGFLAG_DELAY, 20, {}},

	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

/*

static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
*/
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 200, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	//{0x28, 1, {0x00}},
	//{REGFLAG_DELAY, 50, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 150, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=128;

		// Video mode setting		
		params->dsi.intermediat_buffer_num =0;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 4;
		params->dsi.vertical_backporch					= 8;
		params->dsi.vertical_frontporch					= 8;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 6;
		params->dsi.horizontal_backporch				= 37;
		params->dsi.horizontal_frontporch				= 37;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		params->dsi.pll_div1=0;//29;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
		params->dsi.fbk_div=10; 		
		//reefjiang 20130423 begin
		
		/* ESD or noise interference recovery For video mode LCM only. */ // Send TE packet to LCM in a period of n frames and check the response. 
		//params->dsi.lcm_int_te_monitor = FALSE; 
		//params->dsi.lcm_int_te_period = 1; // Unit : frames 
 
		// Need longer FP for more opportunity to do int. TE monitor applicably. 
		//if(params->dsi.lcm_int_te_monitor) 
		//	params->dsi.vertical_frontporch *= 2; 
 
		// Monitor external TE (or named VSYNC) from LCM once per 2 sec. (LCM VSYNC must be wired to baseband TE pin.) 
		//params->dsi.lcm_ext_te_monitor = FALSE; 
		// Non-continuous clock 
		//params->dsi.noncont_clock = TRUE; 
		//params->dsi.noncont_clock_period = 2; // Unit : frames
       //reefjiang 20130423 end
}


static void lcm_init(void)
{
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(50);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{

	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

	SET_RESET_PIN(1);
	MDELAY(1);
	//SET_RESET_PIN(0);
	//MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(60);
//	
}


static void lcm_resume(void)
{
	//lcm_init();
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

/*
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(data_array, 7, 0);

}


static void lcm_setbacklight(unsigned int level)
{
	unsigned int default_level = 145;
	unsigned int mapped_level = 0;

	//for LGE backlight IC mapping table
	if(level > 255) 
			level = 255;

	if(level >0) 
			mapped_level = default_level+(level)*(255-default_level)/(255);
	else
			mapped_level=0;

	// Refresh value of backlight level.
	lcm_backlight_level_setting[0].para_list[0] = mapped_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}

*/


static unsigned int lcm_compare_id(void)
{
#if  COMPARE_BY_ADC
	int data[4] = {0,0,0,0};
	int res = 0;
	int rawdata = 0;
	int lcm_vol = 0;

	res = IMM_GetOneChannelValue(AUXADC_LCM_VOLTAGE_CHANNEL, data, &rawdata);
	if(res < 0) {
#ifdef BUILD_UBOOT
		printf("[adc_uboot]: get data error\n");
#endif
		return 0;
	}

	lcm_vol = data[0] * 1000 + data[1] * 10;

#ifdef BUILD_UBOOT
	printf("[adc_uboot]: lcm_vol= %d\n",lcm_vol);
#endif

	if(lcm_vol >= MIN_VOLTAGE && lcm_vol <= MAX_VOLTAGE) {
		return 1;
	}

	return 0;

#else
	int array[4];
	char buffer[5];
	char id_high=0;
	char id_low=0;
	int id=0;

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(200);

	array[0] = 0x00053700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xa1, buffer, 5);

	id_high = buffer[2];
	id_low = buffer[3];
	id = (id_high<<8) | id_low;

	#if defined(BUILD_UBOOT)
		//printf("OTM8018B uboot %s \n", __func__);
		//printf("%s id = 0x%08x \n", __func__, id);
	#else
		//printk("OTM8018B kernel %s \n", __func__);
		//printk("%s id = 0x%08x \n", __func__, id);
	#endif

	return (LCM_ID_OTM8018B == id)?1:0;

#endif
}


LCM_DRIVER otm8018b_dsi_vdo_djn_lcm_drv = 
{
    .name			= "otm8018b_dsi_vdo_djn",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,	
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif
};
