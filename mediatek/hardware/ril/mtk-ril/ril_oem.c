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

/* //hardware/ril/reference-ril/ril_oem.c
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <telephony/ril.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#include <termios.h>

#include <ril_callbacks.h>

#include "ril_nw.h"

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL"
#else
#define LOG_TAG "RILMD2"
#endif

#include <utils/Log.h>

#define OEM_CHANNEL_CTX getRILChannelCtxFromToken(t)

#ifdef MTK_GEMINI
extern int isModemResetStarted;
#endif

extern int s_md_off;
extern int s_main_loop;

extern char* s_imei[MTK_RIL_SOCKET_NUM];
extern char* s_imeisv[MTK_RIL_SOCKET_NUM];
extern char* s_basebandVersion[MTK_RIL_SOCKET_NUM];
extern char* s_projectFlavor[MTK_RIL_SOCKET_NUM];

void requestSetMute(void * data, size_t datalen, RIL_Token t)
{
    char * cmd;
    ATResponse *p_response = NULL;
    int err;

    asprintf(&cmd, "AT+CMUT=%d", ((int *)data)[0]);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void requestGetMute(void * data, size_t datalen, RIL_Token t)
{
	ATResponse *p_response = NULL;
	int err;
	int response;
	char *line;
	
	err = at_send_command_singleline("AT+CMUT?", "+CMUT:", &p_response, OEM_CHANNEL_CTX);

	if (err < 0 || p_response->success == 0) {
		RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
		goto error;
	}
	
	line = p_response->p_intermediates->line;
	
	err = at_tok_start(&line);
	if (err < 0) goto error;
	
	err = at_tok_nextint(&line, &response);
	if (err < 0) goto error;
	
	RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));

	at_response_free(p_response);
	return;
	
error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	at_response_free(p_response);
}

void requestResetRadio(void * data, size_t datalen, RIL_Token t)
{
    int err;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = NULL;
    if (t)
        p_channel = getRILChannelCtxFromToken(t);
    else
        p_channel = getChannelCtxbyProxy(MTK_RIL_SOCKET_1);

    // only do power off when it is on
    if (s_md_off != 1)
    {
        //power off modem
		if (isFlightModePowerOffModem())
			s_md_off = 1;

		/* Reset the modem, we will do the following steps
		 * 1. AT+EPOF, 
		 *      do network detach and power-off the SIM
		 *      By this way, we can protect the SIM card
		 * 2. AT+EPON
		 *      do the normal procedure of boot up
		 * 3. stop muxd 
		 *      because we will re-construct the MUX channels
		 * 4. The responsibility of Telephony Framework
		 *    i. stop rild
		 *    ii. start muxd to re-construct the MUX channels and start rild
		 *    iii. make sure that it is OK if there is any request in the request queue
		 */
#ifdef MTK_GEMINI
		isModemResetStarted = 1;
		err = at_send_command("AT+EPOF", &p_response, p_channel);
#else
		err = at_send_command("AT+EPOF", &p_response, p_channel);
#endif

		if (err != 0 || p_response->success == 0)
		LOGW("There is something wrong with the exectution of AT+EPOF");
		at_response_free(p_response);

		if (isFlightModePowerOffModem()) {
			LOGD("Flight mode power off modem, trigger CCCI power on modem");
			triggerCCCIIoctl(CCCI_IOC_ENTER_DEEP_FLIGHT);
		}
	}

    resetPhbReady(MTK_RIL_SOCKET_1);
#ifdef MTK_GEMINI
    resetPhbReady(MTK_RIL_SOCKET_2);
#endif

	//power on modem
    if (isFlightModePowerOffModem()) {
        LOGD("Flight mode power on modem, trigger CCCI power on modem");
        triggerCCCIIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT);
#ifdef MTK_GEMINI
        isModemResetStarted = 0;
#endif
    } else {
#ifdef MTK_GEMINI
        err = at_send_command("AT+EPON", &p_response, p_channel);
        isModemResetStarted = 0;
#else
        err = at_send_command("AT+EPON", &p_response, p_channel);
#endif
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+EPON");
        at_response_free(p_response);
    }
    s_main_loop = 0;

    //property_set((const char *) "ctl.stop", (const char *)"gsm0710muxd");

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

// TODO: requestOemHookRaw
void requestOemHookRaw(void * data, size_t datalen, RIL_Token t)
{
    /* atci start */
    ATResponse * p_response = NULL;
    ATLine* p_cur = NULL;
    const char* buffer = (char*)data;
    char** line;
    int i;
    int strLength = 0;
    int err = -1;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    LOGD("data = %s, length = %d", buffer, datalen);

    err = at_send_command_raw(buffer, &p_response, OEM_CHANNEL_CTX);

    if (err < 0) {        
        LOGE("OEM_HOOK_RAW fail");
        goto error;
    }

    LOGD("p_response->success = %d", p_response->success);
    LOGD("p_response->finalResponse = %s", p_response->finalResponse);

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next) {
        LOGD("p_response->p_intermediates = %s", p_cur->line);
        strLength++;
    }
    strLength++;
    LOGD("strLength = %d", strLength);

    line = (char **) alloca(strLength * sizeof(char *));
    if (strLength == 1) {
        line[0] =  p_response->finalResponse;
        LOGD("line[0] = %s", line[0]);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strLength * sizeof(char *));
    } else {
        for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next, i++)
        {
            line[i] = p_cur->line;
            LOGD("line[%d] = %s", i, line[i]);
        }
        line[i] = p_response->finalResponse;
        LOGD("line[%d] = %s", i, line[i]);

        RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strLength * sizeof(char *));
    }

     at_response_free(p_response);
     return;

error:  
    line = (char **) alloca(sizeof(char *));
    line[0] = "ERROR";
    LOGD("line[0] = %s", line[0]);
    RIL_onRequestComplete(t, ret, line, sizeof(char *));

    at_response_free(p_response);
    /* atci end */
}

void requestOemHookStrings(void * data, size_t datalen, RIL_Token t)
{
    int i;
    const char ** cur;
    ATResponse *    p_response = NULL;
    int             err = -1;
    ATLine*         p_cur = NULL;
    char**          line;
    int             strLength = datalen / sizeof(char *);
    RIL_Errno       ret = RIL_E_GENERIC_FAILURE;
    RILId rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    
    LOGD("got OEM_HOOK_STRINGS: 0x%8p %lu", data, (long)datalen);
    
    for (i = strLength, cur = (const char **)data ;
         i > 0 ; cur++, i --) {
            LOGD("> '%s'", *cur);
    }
    

    if (strLength != 2) {
        /* Non proietary. Loopback! */
 
        RIL_onRequestComplete(t, RIL_E_SUCCESS, data, datalen);

        return;
    
    } 

    /* For AT command access */
    cur = (const char **)data;    

    if (NULL != cur[1] && strlen(cur[1]) != 0) {

        if ((strncmp(cur[1],"+CIMI",5) == 0) ||(strncmp(cur[1],"+CGSN",5) == 0)) {

            err = at_send_command_numeric(cur[0], &p_response, OEM_CHANNEL_CTX);

        } else {

        err = at_send_command_multiline(cur[0],cur[1], &p_response, OEM_CHANNEL_CTX);        

        }

    } else {
        
        err = at_send_command(cur[0],&p_response,OEM_CHANNEL_CTX);
        
    }

    if (strncmp(cur[0],"AT+EPIN2",8) == 0) {
        SimPinCount retryCounts;
        LOGW("OEM_HOOK_STRINGS: PIN operation detect");
        getPINretryCount(&retryCounts, t, rid);
    }

    if (err < 0 || NULL == p_response) {        
            LOGE("OEM_HOOK_STRINGS fail");
            goto error;
    }
    
    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_INCORRECT_PASSWORD:
            ret = RIL_E_PASSWORD_INCORRECT;
            break;
        case CME_SIM_PIN_REQUIRED:
        case CME_SIM_PUK_REQUIRED:
            ret = RIL_E_PASSWORD_INCORRECT;
            break;
        case CME_SIM_PIN2_REQUIRED:
            ret = RIL_E_SIM_PIN2;
            break;
        case CME_SIM_PUK2_REQUIRED:
            ret = RIL_E_SIM_PUK2;
            break;
        case CME_DIAL_STRING_TOO_LONG:
            ret = RIL_E_DIAL_STRING_TOO_LONG;
            break;
        case CME_TEXT_STRING_TOO_LONG:
            ret = RIL_E_TEXT_STRING_TOO_LONG;
            break;
        case CME_MEMORY_FULL:
            ret = RIL_E_SIM_MEM_FULL;
            break;
        case CME_BT_SAP_UNDEFINED:
            ret = RIL_E_BT_SAP_UNDEFINED;
            break;
        case CME_BT_SAP_NOT_ACCESSIBLE:
            ret = RIL_E_BT_SAP_NOT_ACCESSIBLE;
            break;
        case CME_BT_SAP_CARD_REMOVED:
            ret = RIL_E_BT_SAP_CARD_REMOVED;
            break;
        default:
            ret = RIL_E_GENERIC_FAILURE;
            break;
    }

    if (ret != RIL_E_SUCCESS) {
        goto error;
    }

    if (strncmp(cur[0],"AT+ESSP",7) == 0) {
        LOGI("%s , %s !",cur[0], cur[1]); 
        if(strcmp(cur[1], "") == 0) {
            updateCFUQueryType(cur[0]);
        }
    } 

    /* Count response length */
    strLength = 0;

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next)
        strLength++;

    if (strLength == 0) {

        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

    } else {   
    
        LOGI("%d of %s received!",strLength, cur[1]);     
    
        line = (char **) alloca(strLength * sizeof(char *));
        
        for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next, i++)
        {
            line[i] = p_cur->line;
        }
                
        RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strLength * sizeof(char *));
    }
    
    at_response_free(p_response);
    
    return;

error:  
    RIL_onRequestComplete(t, ret, NULL, 0);

    at_response_free(p_response);

}

void updateCFUQueryType(char *cmd)
{
    int fd;
	int n;
	struct env_ioctl en_ctl;
	char *name = NULL;
	char *value = NULL;

    do {
    	memset(&en_ctl,0x00,sizeof(struct env_ioctl));
    	
    	fd= open("/proc/lk_env",O_RDWR);

    	if(fd<= 0) {
    	    LOGE("ERROR open fail %d\n", fd);
    		break;
    	}

    	name = calloc(1, BUF_MAX_LEN);
    	value = calloc(1, BUF_MAX_LEN);

    	strncpy(name,SETTING_QUERY_CFU_TYPE, strlen(SETTING_QUERY_CFU_TYPE));

    	value[0] = *(cmd+8);

    	en_ctl.name = name;
    	en_ctl.value = value;
    	en_ctl.name_len = strlen(name)+1;

    	en_ctl.value_len = strlen(value)+1;
    	LOGD("write %s = %s\n",name,value);
    	n=ioctl(fd,ENV_WRITE,&en_ctl);
    	if(n<0) {
    		printf("ERROR write fail %d\n",n);
    	}
    	else {   	    
    	    property_set(SETTING_QUERY_CFU_TYPE, value);   	    
    	}
        free(name);
        free(value);
        close(fd);
    }while(0);
}

void initCFUQueryType()
{
    int fd;
    int n;
    struct env_ioctl en_ctl;
    char *name = NULL;
    char *value = NULL;
    
    do {
        memset(&en_ctl,0x00,sizeof(struct env_ioctl));

        fd= open("/proc/lk_env",O_RDONLY);

        if(fd<= 0) {
            LOGE("ERROR open fail %d\n", fd);
        	break;
        }

        name = calloc(1, BUF_MAX_LEN);
        value = calloc(1, BUF_MAX_LEN);

        memcpy(name,SETTING_QUERY_CFU_TYPE, strlen(SETTING_QUERY_CFU_TYPE));	

        en_ctl.name = name;
        en_ctl.value = value;
        en_ctl.name_len = strlen(name)+1;
        en_ctl.value_len = BUF_MAX_LEN;	

        LOGD("read %s \n",name);

        n=ioctl(fd,ENV_READ,&en_ctl);
        if(n<0){
            LOGE("ERROR read fail %d\n",n);
        }
        else {
            property_set(name, value);
        }
        
        free(name);
        free(value);
        close(fd);
    }while(0);
}

void updateSignalStrength(RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    //MTK-START [ALPS00506562][ALPS00516994]
    //int response[12]={0};
    int response[16]={0};
    //MTK-START [ALPS00506562][ALPS00516994]
    
    char *line;

    memset(response, 0, sizeof(response));

    err = at_send_command_singleline("AT+ECSQ", "+ECSQ:", &p_response, OEM_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0 ||
            p_response->p_intermediates  == NULL) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = getSingnalStrength(line, &response[0], &response[1], &response[2], &response[3], &response[4]);

    if (err < 0) goto error;

    RIL_onUnsolicitedResponse (RIL_UNSOL_SIGNAL_STRENGTH,
     							response,
       							sizeof(response),
       							getRILIdByChannelCtx(getRILChannelCtxFromToken(t)));

    at_response_free(p_response);
    return;

error:
    at_response_free(p_response);	
    LOGE("updateSignalStrength ERROR: %d", err);
}

void requestScreenState(void * data, size_t datalen, RIL_Token t)
{
    /************************************
    * Disable the URC: ECSQ,CREG,CGREG,CTZV
    * For the URC +CREG and +CGREG
    * we will buffer the URCs when the screen is off
    * and issues URCs when the screen is on
    * So we can reflect the ture status when screen is on
    *************************************/

    int on_off, err;
    ATResponse *p_response = NULL;

    on_off = ((int*)data)[0];

    if (on_off)
    {
        // screen is on

         /* Disable Network registration events of the changes in LAC or CID */
        err = at_send_command("AT+CREG=2", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+CREG=2");
        at_response_free(p_response);

        err = at_send_command("AT+CGREG=2", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+CGREG=2");
        at_response_free(p_response);

        /* Enable get ECSQ URC */
        at_send_command("AT+ECSQ=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECSQ=2");
        at_response_free(p_response);
        updateSignalStrength(t);

        /* Enable PSBEARER URC */
        at_send_command("AT+PSBEARER=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+PSBEARER=1");
        at_response_free(p_response);
    }
    else
    {
        // screen is off

        /* Disable Network registration events of the changes in LAC or CID */
        err = at_send_command("AT+CREG=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+CREG=1");
        at_response_free(p_response);

        err = at_send_command("AT+CGREG=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+CGREG=1");
        at_response_free(p_response);

        /* Disable get ECSQ URC */
        at_send_command("AT+ECSQ=0", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECSQ=0");
        at_response_free(p_response);

        /* Disable PSBEARER URC */
        at_send_command("AT+PSBEARER=0", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+PSBEARER=0");
        at_response_free(p_response);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

extern void requestGet3GCapability(void * data, size_t datalen, RIL_Token t)
{
    /*ATResponse *p_response = NULL;
	int err;
	int response;
	char *line;
	
	err = at_send_command_singleline("AT+ES3G?", "+ES3G:", &p_response, OEM_CHANNEL_CTX);

	if (err < 0 || p_response->success == 0) {
		RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
		goto error;
	}
	
	line = p_response->p_intermediates->line;
	
	err = at_tok_start(&line);
	if (err < 0) goto error;
	
	err = at_tok_nextint(&line, &response);
	if (err < 0) goto error;*/
	int response = RIL_is3GSwitched() ? 2 : 1;
	RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));

	//at_response_free(p_response);
	return;
	
error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	//at_response_free(p_response);
}

extern int get3GCapabilitySim(RIL_Token t)
{
    ATResponse *p_response = NULL;
	int err;
	int response = 1;
	char *line;

    if (t) {
        err = at_send_command_singleline("AT+ES3G?", "+ES3G:", &p_response, OEM_CHANNEL_CTX);
    } else {
        LOGI("The ril token is null, use URC instead");
    	err = at_send_command_singleline("AT+ES3G?", "+ES3G:", &p_response, getChannelCtxbyProxy(MTK_RIL_SOCKET_1));
    }

	if (err < 0 || p_response->success == 0) goto error;
	
	line = p_response->p_intermediates->line;
	
	err = at_tok_start(&line);
	if (err < 0) goto error;
	
	err = at_tok_nextint(&line, &response);
	if (err < 0) goto error;

	at_response_free(p_response);
	return response;
	
error:
	at_response_free(p_response);
	return 0;
}

extern void set3GCapability(RIL_Token t, int setting)
{
    char * cmd;
    ATResponse *p_response = NULL;
    int err = 0;
    asprintf(&cmd, "AT+ES3G=%d, %d", setting, NETWORK_MODE_WCDMA_PREF);
    if (t) {
        err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    } else {
        LOGI("The ril token is null, use URC instead");
    	err = at_send_command(cmd, &p_response, getChannelCtxbyProxy(MTK_RIL_SOCKET_1));
    }

    free(cmd);
    if (err < 0 || p_response->success == 0) {
        LOGI("Set 3G capability to [%d] failed", setting);
    } else {
        LOGI("Set 3G capability to [%d] successfully", setting);
    }
    at_response_free(p_response);
}

extern void requestSet3GCapability(void * data, size_t datalen, RIL_Token t)
{
    char * cmd;
    ATResponse *p_response = NULL;
    int err;

    int setting = ((int *)data)[0];
    int old3GSim = get3GCapabilitySim(t);
    if (setting == -1) {
        // rat must be 1 (for GSM only)
        setting = old3GSim;
        asprintf(&cmd, "AT+ES3G=%d, %d", setting, NETWORK_MODE_GSM_ONLY);
    } else {
        // Set the network RAT mode of modem be Dual Mode (GSM and UMTS)
        asprintf(&cmd, "AT+ES3G=%d, %d", setting, NETWORK_MODE_WCDMA_PREF);
    }
    
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        //we set property here to let RILJ could get the correct settings
        //when retrying making connection to rild
        char* value;
        if (setting == 1)
            asprintf(&value, "%d", CAPABILITY_3G_SIM1);
        else if (setting == 2)
            asprintf(&value, "%d", CAPABILITY_3G_SIM2);
        else
            asprintf(&value, "%d", CAPABILITY_NO_3G);

        property_set(PROPERTY_3G_SIM, value);
        free(value);

        if (old3GSim != setting)
            rilSwitchSimProperty();

        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

extern void flightModeBoot()
{
    ATResponse *p_response = NULL;
    LOGI("Start flight modem boot up");
#ifdef MTK_GEMINI
    int err = at_send_command("AT+EFUN=0", &p_response, getChannelCtxbyProxy(MTK_RIL_SOCKET_1));
#else
    int err = at_send_command("AT+CFUN=4", &p_response, getChannelCtxbyProxy(MTK_RIL_SOCKET_1));
#endif
    if (err != 0 || p_response->success == 0)
        LOGW("Start flight modem boot up failed");
    at_response_free(p_response);

    int telephonyMode = getTelephonyMode();
    switch (telephonyMode) {
        case TELEPHONY_MODE_6_TGNG_DUALTALK:
#ifdef MTK_RIL_MD1
            err = at_send_command("AT+ERAT=0", &p_response, getChannelCtxbyProxy(MTK_RIL_SOCKET_1));
            if (err != 0 || p_response->success == 0)
                LOGW("Set default RAT mode failed");
            at_response_free(p_response);
#endif
            break;
        case TELEPHONY_MODE_7_WGNG_DUALTALK:
#ifdef MTK_RIL_MD2
            err = at_send_command("AT+ERAT=0", &p_response, getChannelCtxbyProxy(MTK_RIL_SOCKET_1));
            if (err != 0 || p_response->success == 0)
                LOGW("Set default RAT mode failed");
            at_response_free(p_response);
#endif
            break;
        case TELEPHONY_MODE_8_GNG_DUALTALK:
            err = at_send_command("AT+ERAT=0", &p_response, getChannelCtxbyProxy(MTK_RIL_SOCKET_1));
            if (err != 0 || p_response->success == 0)
                LOGW("Set default RAT mode failed");
            at_response_free(p_response);
            break;
    }
}

extern int getMappingSIMByCurrentMode(RILId rid) {
    if (isDualTalkMode()) {
        if (getFirstModem() == FIRST_MODEM_MD1) {
            #ifdef MTK_RIL_MD1
                return GEMINI_SIM_1;
            #else
                return GEMINI_SIM_2;
            #endif
        } else {
            #ifdef MTK_RIL_MD1
                return GEMINI_SIM_2;
            #else
                return GEMINI_SIM_1;
            #endif
        }
    } else {
        if (RIL_is3GSwitched()) {
            if (rid == MTK_RIL_SOCKET_1) {
                return GEMINI_SIM_2;
            } else {
                return GEMINI_SIM_1;
            }
        } else {
            if (rid == MTK_RIL_SOCKET_1) {
                return GEMINI_SIM_1;
            } else {
                return GEMINI_SIM_2;
            }
        }
    }
}

extern void upadteSystemPropertyByCurrentMode(int rid, char* key1, char* key2, char* value) {
    if (isDualTalkMode()) {
        if (getFirstModem() == FIRST_MODEM_MD1) {
            #ifdef MTK_RIL_MD1
                LOGI("Update property SIM1 (dt)[%s, %s]", key1, value != NULL ? value : "");
                property_set(key1, value);
            #else
                LOGI("Update property SIM2 (dt) [%s, %s]", key2, value != NULL ? value : "");
                property_set(key2, value);
            #endif
        } else {
            #ifdef MTK_RIL_MD1
                LOGI("Update property SIM2 (dt switched) [%s, %s]", key2, value != NULL ? value : "");
                property_set(key2, value);
            #else
                LOGI("Update property SIM1 (dt switched) [%s, %s]", key1, value != NULL ? value : "");
                property_set(key1, value);
            #endif
        }
    } else {
        if (RIL_is3GSwitched()) {
            if (rid == MTK_RIL_SOCKET_1) {
                LOGI("Update property SIM2 (3g switched) [%s, %s]", key2, value != NULL ? value : "");
                property_set(key2, value);
            } else {
                LOGI("Update property SIM1 (3g switched) [%s, %s]", key1, value != NULL ? value : "");
                property_set(key1, value);
            }
        } else {
            if (rid == MTK_RIL_SOCKET_1) {
                LOGI("Update property SIM1 [%s, %s]", key1, value != NULL ? value : "");
                property_set(key1, value);
            } else {
                LOGI("Update property SIM2 [%s, %s]", key2, value != NULL ? value : "");
                property_set(key2, value);
            }
        }
    }
}

extern void bootupGetIccid(RILId rid) {
    int result = 0;
    ATResponse *p_response = NULL;
    int err = at_send_command_singleline("AT+ICCID?", "+ICCID:", &p_response, getDefaultChannelCtx(rid));

    if (err >= 0 && p_response != NULL) {
        if (at_get_cme_error(p_response) == CME_SUCCESS) {
            char *line;
            char *iccId;
            line = p_response->p_intermediates->line;
            err = at_tok_start (&line);	
            if (err >= 0) {
                err = at_tok_nextstr(&line, &iccId);
                if (err >= 0) {
                    LOGD("bootupGetIccid[%d] iccid is %s", rid, iccId);
                    upadteSystemPropertyByCurrentMode(rid, PROPERTY_ICCID_SIM1, PROPERTY_ICCID_SIM2, iccId);
                    result = 1;
                } else
                    LOGD("bootupGetIccid[%d]: get iccid error 2", rid);
            } else {
                LOGD("bootupGetIccid[%d]: get iccid error 1", rid);
            }
        } else {
            LOGD("bootupGetIccid[%d]: Error or no SIM inserted!", rid);
        }
    } else {
        LOGE("bootupGetIccid[%d] Fail", rid);
    }
    at_response_free(p_response);

    if (!result) {
        LOGE("bootupGetIccid[%d] fail and write default string", rid);
        upadteSystemPropertyByCurrentMode(rid, PROPERTY_ICCID_SIM1, PROPERTY_ICCID_SIM2, "N/A");
    }
}

extern void bootupGetImei(RILId rid) {
    LOGE("bootupGetImei[%d]", rid);
    ATResponse *p_response = NULL;
    int err = at_send_command_numeric("AT+CGSN", &p_response, getDefaultChannelCtx(rid));

    if (err >= 0 && p_response->success != 0) {
        err = asprintf(&s_imei[rid], "%s", p_response->p_intermediates->line);
        if(err < 0)
            LOGE("bootupGetImei[%d] set fail", rid);
    } else {
        LOGE("bootupGetImei[%d] Fail", rid);
    }
    at_response_free(p_response);
}

extern void bootupGetImeisv(RILId rid) {
    LOGE("bootupGetImeisv[%d]", rid);
    ATResponse *p_response = NULL;
    int err = at_send_command_singleline("AT+EGMR=0,9", "+EGMR:",&p_response, getDefaultChannelCtx(rid));

    if (err >= 0 && p_response->success != 0) {
        char* sv = NULL;
        char* line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if(err >= 0) {
            err = at_tok_nextstr(&line, &sv);
            if(err >= 0) {
                err = asprintf(&s_imeisv[rid], "%s", sv);
                if(err < 0)
                    LOGE("bootupGetImeisv[%d] set fail", rid);
            } else {
                LOGE("bootupGetImeisv[%d] get token fail", rid);
            }
        } else {
            LOGE("bootupGetImeisv[%d] AT CMD fail", rid);
        }
    } else {
        LOGE("bootupGetImeisv[%d] Fail", rid);
    }
    at_response_free(p_response);
}

void bootupGetProjectFlavor(RILId rid){    
    LOGE("bootupGetProjectAndMdInfo[%d]", rid);
    ATResponse *p_response = NULL;
    ATResponse *p_response2 = NULL;
    char* line = NULL; 
    char* line2 = NULL;
    char* projectName= NULL;
    char* flavor= NULL;
    int err; 

    // Add for MD-team query project/flavor properties : project name(flavor)
    err = at_send_command_singleline("AT+EGMR=0,4", "+EGMR:",&p_response, getDefaultChannelCtx(rid));
    if (err >= 0 && p_response->success != 0) {
        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if(err >= 0) {
            err = at_tok_nextstr(&line, &projectName);
            if(err < 0)
                LOGE("bootupGetProjectName[%d] get token fail", rid);
        } else {
            LOGE("bootupGetProjectName[%d] AT CMD fail", rid);
        }
    } else {
        LOGE("bootupGetProjectName[%d] Fail", rid);
    }

    //query project property (flavor) 
    err = at_send_command_singleline("AT+EGMR=0,13", "+EGMR:",&p_response2, getDefaultChannelCtx(rid));
    if (err >= 0 && p_response2->success != 0) {
        line2 = p_response2->p_intermediates->line;
        err = at_tok_start(&line2);
        if(err >= 0) {
            err = at_tok_nextstr(&line2, &flavor);
            if(err < 0)
                LOGE("bootupGetFlavor[%d] get token fail", rid);
        } else {
            LOGE("bootupGetFlavor[%d] AT CMD fail", rid);
        }
    } else {
        LOGE("bootupGetFlavor[%d] Fail", rid);
    }

    //combine string: projectName(flavor)
    err = asprintf(&s_projectFlavor[rid], "%s(%s)",projectName ,flavor);
    if(err < 0) LOGE("bootupGetProject[%d] set fail", rid);

    if (getMappingSIMByCurrentMode(rid) == GEMINI_SIM_2){
        err = property_set("gsm.project.baseband.2",s_projectFlavor[rid]);
        if(err < 0) LOGE("SystemProperty: PROPERTY_PROJECT_2 set fail");
    }else{
        err = property_set("gsm.project.baseband" ,s_projectFlavor[rid]);
        if(err < 0) LOGE("SystemProperty: PROPERTY_PROJECT set fail");
    }

    at_response_free(p_response);
    at_response_free(p_response2);

}

extern void bootupGetBasebandVersion(RILId rid) {
    LOGE("bootupGetBasebandVersion[%d]", rid);
    ATResponse *p_response = NULL;
    int err, i, len;
    char *line, *ver, null;

    ver = &null;
    ver[0] = '\0';

    //Add for MD-team query project/flavor properties : project name(flavor)
    bootupGetProjectFlavor(rid);

    err = at_send_command_multiline("AT+CGMR", "+CGMR:",&p_response, getDefaultChannelCtx(rid));

    if (err < 0 || p_response->success == 0)
    {
        goto error;
    }
    else if (p_response->p_intermediates != NULL)
    {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if(err < 0) goto error;

        //remove the white space from the end
        len = strlen(line);
        while( len > 0 && isspace(line[len-1]) )
            len --;
        line[len] = '\0';

        //remove the white space from the beginning
        while( (*line) != '\0' &&  isspace(*line) )
            line++;

        ver = line;
    }
    else
    {
        // ALPS00295957 : To handle AT+CGMR without +CGMR prefix response
        at_response_free(p_response);
        p_response = NULL;

        LOGE("Retry AT+CGMR without expecting +CGMR prefix");		

        err = at_send_command_raw("AT+CGMR", &p_response, getDefaultChannelCtx(rid));
		
        if (err < 0) {        
            LOGE("Retry AT+CGMR ,fail");		
            goto error;
    	}		

        if(p_response->p_intermediates != NULL)
        {
            line = p_response->p_intermediates->line;
        
            LOGD("retry CGMR response = %s", line);		

            //remove the white space from the end
            len = strlen(line);
            while( len > 0 && isspace(line[len-1]) )
                len --;
            line[len] = '\0';

            //remove the white space from the beginning
            while( (*line) != '\0' &&  isspace(*line) )
                line++;

            ver = line;
        }			
    }
    asprintf(&s_basebandVersion[rid], "%s", ver);
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
}

extern int triggerCCCIIoctl(int request)
{
	int ccci_sys_fd = -1;
#ifdef MTK_RIL_MD2
        ccci_sys_fd = open(CCCI_MD2_POWER_IOCTL_PORT, O_RDWR | O_NONBLOCK);
#else
        ccci_sys_fd = open(CCCI_MD1_POWER_IOCTL_PORT, O_RDWR | O_NONBLOCK);
#endif
	if (ccci_sys_fd < 0) {
		LOGD("open CCCI ioctl port failed [%d]", ccci_sys_fd);
        return -1;
	}

    int ret_ioctl_val = ioctl(ccci_sys_fd, request);
	LOGD("CCCI ioctl result: ret_val=%d, request=%d", ret_ioctl_val, request);

	close(ccci_sys_fd);
	return ret_ioctl_val;
}

extern int rilOemMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
        case RIL_REQUEST_OEM_HOOK_RAW:
            requestOemHookRaw(data, datalen, t);
            // echo back data
            //RIL_onRequestComplete(t, RIL_E_SUCCESS, data, datalen);
            break;
        case RIL_REQUEST_OEM_HOOK_STRINGS:
            requestOemHookStrings(data,datalen,t);
            break;
        case RIL_REQUEST_SCREEN_STATE:
            requestScreenState(data, datalen, t);
            break;
        case RIL_REQUEST_SET_MUTE:
            requestSetMute(data,datalen,t);
            break;
        case RIL_REQUEST_GET_MUTE:
            requestGetMute(data, datalen, t);
            break;
        case RIL_REQUEST_RESET_RADIO:
            requestResetRadio(data, datalen, t);
            break;
        case RIL_REQUEST_GET_3G_CAPABILITY:
            requestGet3GCapability(data, datalen, t);
            break;
        case RIL_REQUEST_SET_3G_CAPABILITY:
            requestSet3GCapability(data, datalen, t);
            break;
        default:
            return 0;  /* no matched request */
            break;
    }

    return 1; /* request found and handled */
}

extern int rilOemUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    return 0; 
}




