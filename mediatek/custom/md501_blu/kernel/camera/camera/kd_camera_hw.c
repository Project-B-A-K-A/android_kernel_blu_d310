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
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>
#include <linux/kernel.h>//for printk


#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_ERR_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    xlog_printk(ANDROID_LOG_ERROR, PFX , fmt, ##arg)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)         xlog_printk(ANDROID_LOG_ERROR, PFX , fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...) \
                do {    \
                   xlog_printk(ANDROID_LOG_INFO, PFX , fmt, ##arg); \
                } while(0)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#define PK_XLOG_INFO(fmt, args...)
#endif

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);

kal_uint16 __OV5647MIPI_read_cmos_sensor(kal_uint32 addr)
{
        kal_uint16 get_byte=0;
    char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
        iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,1,0x6c);
    return get_byte;
}

static int ov5647_version;
static int ov5647_true;


int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{
u32 pinSetIdx = 0;//default main sensor
int ret;

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4

#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3   
u32 pinSet[3][8] = {
                    //for main sensor
                    {GPIO_CAMERA_CMRST_PIN,
                        GPIO_CAMERA_CMRST_PIN_M_GPIO,   /* mode */
                        GPIO_OUT_ONE,                   /* ON state */
                        GPIO_OUT_ZERO,                  /* OFF state */
                     GPIO_CAMERA_CMPDN_PIN,
                        GPIO_CAMERA_CMPDN_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                    },
                    //for sub sensor
                    {GPIO_CAMERA_CMRST1_PIN,
                     GPIO_CAMERA_CMRST1_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                     GPIO_CAMERA_CMPDN1_PIN,
                        GPIO_CAMERA_CMPDN1_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                    },
                    //for main_2 sensor
                    {GPIO_CAMERA_2_CMRST_PIN,
                        GPIO_CAMERA_2_CMRST_PIN_M_GPIO,   /* mode */
                        GPIO_OUT_ONE,                   /* ON state */
                        GPIO_OUT_ZERO,                  /* OFF state */
                     GPIO_CAMERA_2_CMPDN_PIN,
                        GPIO_CAMERA_2_CMPDN_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                    }
                   };

	
	if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K4E1GA_MIPI_RAW,currSensorName)))
	{
		printk("XXXXXXXXXX S5K4E1 sensor PDN off is one\n");
		pinSet[0][IDX_PS_CMPDN + IDX_PS_ON] = GPIO_OUT_ZERO;
		pinSet[0][IDX_PS_CMPDN + IDX_PS_OFF] = GPIO_OUT_ONE;
	}


    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
        pinSetIdx = 0;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
    }
    else if (DUAL_CAMERA_MAIN_2_SENSOR == SensorIdx) {
       // pinSetIdx = 2;
    }

    //power ON
    if (On) {
        	PK_DBG(" kdCISModulePowerOn -on:currSensorName=%s\n",currSensorName);
		PK_DBG(" kdCISModulePowerOn -on:pinSetIdx=%d\n",pinSetIdx);
		if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5647MIPI_RAW,currSensorName)))
		{
			int sensorID;

				PK_ERR("XXXXXXXXXXXX kdCISModulePowerOn get in---  SENSOR_DRVNAME_OV5647MIPI_RAW \n");
				PK_ERR("[ON_general 1.8V]sensorIdx:%d \n",SensorIdx);
			if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
				{
					PK_ERR("[CAMERA SENSOR] Fail to enable digital power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}


			if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
			{
				PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D,VOL_1800,mode_name))
			{
				 PK_ERR("[CAMERA SENSOR] Fail to enable digital power\n");
				 //return -EIO;
				 goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name))
			{
				PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}
			msleep(5); 

		mt_set_gpio_pull_enable(GPIO133, GPIO_PULL_DISABLE);
                mt_set_gpio_mode(GPIO133,GPIO_MODE_00);
                mt_set_gpio_dir(GPIO133,GPIO_DIR_IN);
                ret = mt_get_gpio_in(GPIO133);
		if (1 == ret) {
			printk("XXXXXXXXXXXXX sunwin DVDD == 1.5V XXXXXXXXXXXX\n");
			
			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
				PK_ERR("[CAMERA SENSOR] Fail to OFF digital power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D,VOL_1500,mode_name))
			{
				 PK_ERR("[CAMERA SENSOR] Fail to enable digital power\n");
				 //return -EIO;
				 goto _kdCISModulePowerOn_exit_;
			}
		}
		else {
			printk("XXXXXXXXXXXXX turly DVDD == 1.8V XXXXXXXXXXXX\n");
		}
			
                     /*end*/
			//PDN/STBY pin
			if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
			{
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_ERR("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");}
				msleep(10);
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");}
				msleep(1);

				//RST pin
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");}
				mdelay(10);
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");}
				mdelay(1);
			}

                        if (0 == pinSetIdx) {
                                mdelay(20);
                                sensorID=((__OV5647MIPI_read_cmos_sensor(0x300A) << 8) | __OV5647MIPI_read_cmos_sensor(0x300B));
                                if (0x5647 != sensorID) {
                                        mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON]);
                                        mdelay(10);
                                	sensorID=((__OV5647MIPI_read_cmos_sensor(0x300A) << 8) | __OV5647MIPI_read_cmos_sensor(0x300B));
					if (0x5647 == sensorID) {
                                        	ov5647_version = 1;
						ov5647_true = 1;
                                        	printk("XXXXXXXXXXXXXXX  power on genan ov5647_version %d XXXXXXXXXXXXXXX\n", ov5647_version);
					}
                                }
                                else {
                                        printk("XXXXXXXXXXXXXXX  power on genan ov5647_version %d XXXXXXXXXXXXXXX\n", ov5647_version);
					ov5647_true = 1;
				}
                        }

        //disable inactive sensor
        if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
	        if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_ERR("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
	            if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_ON])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	        }
        }
		else {
	        if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_MODE])){PK_ERR("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
                if (ov5647_version) {
                    if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_OFF])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
                }
                else {
                    if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_ON])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
                }
	        }
		}
		if(mt_set_gpio_mode(GPIO139,GPIO_MODE_00)){PK_DBG("[camera_af_en_gpio] set gpio mode failed!! \n");} //dongteng add enable AF_VIB pin.
    		if(mt_set_gpio_dir(GPIO139,1)){PK_DBG("[camera_af_en_gpio] set gpio dir failed!! \n");}
    		if(mt_set_gpio_out(GPIO139,1)){PK_DBG("[camera_af_en_gpio] set gpio failed!! \n");}


              }else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_A5142_MIPI_RAW,currSensorName)))
		{
				PK_ERR(" kdCISModulePowerOn get in---  SENSOR_DRVNAME_A5142_MIPI_RAW \n");
				PK_ERR("[ON_general 1.8V]sensorIdx:%d \n",SensorIdx);
			if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
				{
					PK_ERR("[CAMERA SENSOR] Fail to enable digital power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}
			if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D,VOL_1800,mode_name))
			{
				 PK_ERR("[CAMERA SENSOR] Fail to enable digital power\n");
				 //return -EIO;
				 goto _kdCISModulePowerOn_exit_;
			}


			if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
			{
				PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			msleep(5); 
                     /*end*/
			//PDN/STBY pin
			if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
			{
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_ERR("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");}
				msleep(10);
				//RST pin
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");}
				mdelay(10);
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");}
				mdelay(1);
			}

               //disable inactive sensor
              if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
	        if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
                    PK_ERR("lilonghui call the [CAMERA SENSOR] pinSetIdx == 0 || pinSetIdx == 2  \n");

	            if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_ERR("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
	            if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_ON])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	        }
        }
		else {
	        if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_MODE])){PK_ERR("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
	            if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_OFF])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	        }

		}

	}else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC0329_YUV,currSensorName)))
	{
		PK_ERR("CISModulePowerOn get in---  SENSOR_DRVNAME_GC0329_YUV \n");
		PK_ERR(" [ON_general 2.8V]sensorIdx:%d \n",SensorIdx);
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
				{
					PK_ERR("[CAMERA SENSOR] Fail to enable digital power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D,VOL_1800,mode_name))
			{
				 PK_ERR("[CAMERA SENSOR] Fail to enable digital power\n");
				 //return -EIO;
				 goto _kdCISModulePowerOn_exit_;
			}


		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
			{
				PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}
		//PDN/STBY pin
		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
		{
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_ERR("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");}
			msleep(10);
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");}
			msleep(5);

			//RST pin
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");}
			mdelay(10);
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");}
			mdelay(5);
		}

        //disable inactive sensor
        if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
	        if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
                
	            if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_ERR("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
	            if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_ON])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	        }
        }
	}
    else {
		printk("XXXXXXXXXXXXXXXXXXXXX S5K4E1 Power Up XXXXXXXXXXXXXXX\n");
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D,VOL_1800,mode_name))
		{
			PK_ERR("[CAMERA SENSOR] Fail to enable digital power\n");
			//return -EIO;
			goto _kdCISModulePowerOn_exit_;
		}
		mdelay(5);

		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
		{
			PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
			//return -EIO;
			goto _kdCISModulePowerOn_exit_;
		}

		mdelay(5);

		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
		{
			PK_ERR("[CAMERA SENSOR] Fail to enable digital power\n");
			//return -EIO;
			goto _kdCISModulePowerOn_exit_;
		}

		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name))
		{
			PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
			//return -EIO;
			goto _kdCISModulePowerOn_exit_;
		}
		msleep(5);

		//PDN/STBY pin
		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
		{
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_ERR("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");}
			msleep(10);
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");}
			msleep(1);

			//RST pin
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");}
			mdelay(10);
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");}
			mdelay(1);
		}

		//disable inactive sensor
		if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
			if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
				PK_ERR("lilonghui call the [CAMERA SENSOR] pinSetIdx == 0 || pinSetIdx == 2  \n");

				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_ERR("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
				if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_ON])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
				}
			}
		else {
			if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_MODE])){PK_ERR("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
				if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_ON])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
			}
		}
	}
    	}
  else {//power OFF

		PK_ERR("XXXXXXXXXXXXXX kdCISModulePowerOn -off:currSensorName=%s\n",currSensorName);

		if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5647MIPI_RAW,currSensorName)))
		{
			PK_ERR("XXXXXXXXXX kdCISModulePower--off get in---SENSOR_DRVNAME_OV5647MIPI_RAW \n");

            //reset pull down
			if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
                                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
                                if (0 == pinSetIdx) {
                                if (ov5647_version) {
                                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //high == pwnd sensor
                                }
                                else {
                                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //high == pwnd sensor
                                }
                                }
                                else {
                                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //high == pwnd sensor
                                }

			}
    			if(mt_set_gpio_mode(GPIO139,GPIO_MODE_00)){PK_DBG("[camera_af_en_gpio] set gpio mode failed!! \n");} //dongteng add disable AF_VIB pin.
    			if(mt_set_gpio_dir(GPIO139,1)){PK_DBG("[camera_af_en_gpio] set gpio dir failed!! \n");}
    			if(mt_set_gpio_out(GPIO139,0)){PK_DBG("[camera_af_en_gpio] set gpio failed!! \n");}


			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
				PK_ERR("[CAMERA SENSOR] Fail to OFF digital power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
				PK_ERR("[CAMERA SENSOR] Fail to OFF analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
			{
				PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2,mode_name))
			{
				PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

		}else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_A5142_MIPI_RAW,currSensorName)))
		{
			
            //reset pull down
#if 1
			if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {

                               PK_ERR("lilonghi call the gpio off \n");
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
                                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //high == pwnd sensor
			}
#endif
			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
				PK_ERR("[CAMERA SENSOR] Fail to OFF digital power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
				PK_ERR("[CAMERA SENSOR] Fail to OFF analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
			{
				PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}


		}else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC0329_YUV,currSensorName)))
		{
			PK_ERR("kdCISModulePower--off get in---SENSOR_DRVNAME_GC0329_YUV \n");

			if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor

				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_ERR("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA LENS] set gpio dir failed!! \n");}
                                if (0 == pinSetIdx) { 
                                if (ov5647_version) { 
                                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //high == pwnd sensor 
                                } 
                                else { 
                                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //high == pwnd sensor 
                                } 
                                } 
                                else { 
                                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module 
                                }
			}
			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
				PK_ERR("[CAMERA SENSOR] Fail to OFF digital power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
				PK_ERR("[CAMERA SENSOR] Fail to OFF analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
			{
				PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

		}

		else
	    {
	    	
			if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
				PK_ERR("lilonghi call the gpio off \n");
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_ERR("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_ERR("[CAMERA SENSOR] set gpio dir failed!! \n");}
                                if (0 == pinSetIdx) { 
                                if (ov5647_version) { 
                                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //high == pwnd sensor 
                                } 
                                else { 
                                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_ERR("[CAMERA SENSOR] set gpio failed!! \n");} //high == pwnd sensor 
                                } 
                                } 
                                else { 
                                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_ERR("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module 
                                }

			}
			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
				PK_ERR("[CAMERA SENSOR] Fail to OFF analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
				PK_ERR("[CAMERA SENSOR] Fail to OFF digital power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2,mode_name))
			{
				PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
			{
				PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
				//return -EIO;
				goto _kdCISModulePowerOn_exit_;
			}

		}
  }
	return 0;

_kdCISModulePowerOn_exit_:
    return -EIO;
}

EXPORT_SYMBOL(kdCISModulePowerOn);


//!--
//




