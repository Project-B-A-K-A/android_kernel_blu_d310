# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.


# Copyright 2006 The Android Open Source Project


ifeq ($(GOOGLE_RELEASE_RIL), yes)
# do nothing
else
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	rild.c

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	librilmtk

LOCAL_STATIC_LIBRARIES := \
	rild-prop-md1
	
ifeq ($(TARGET_ARCH),arm)
LOCAL_SHARED_LIBRARIES += libdl
endif # arm

ifeq ($(GEMINI),yes)
  LOCAL_CFLAGS := -DRIL_SHLIB -DMTK_RIL -DMTK_GEMINI
else
  LOCAL_CFLAGS := -DRIL_SHLIB -DMTK_RIL

  ifneq ($(MTK_SHARE_MODEM_SUPPORT),1)
        LOCAL_CFLAGS += -DMTK_GEMINI     
  endif
endif

ifeq ($(MTK_DT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_DT_SUPPORT
endif

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
LOCAL_CFLAGS += -DENG_BUILD
endif

LOCAL_CFLAGS += -DMTK_RIL_MD1

LOCAL_MODULE:= rild

include $(BUILD_EXECUTABLE)


#========================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	rild.c

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	librilmtkmd2

LOCAL_STATIC_LIBRARIES := \
	rild-prop-md2
	
ifeq ($(TARGET_ARCH),arm)
LOCAL_SHARED_LIBRARIES += libdl
endif # arm

ifeq ($(GEMINI),yes)
  LOCAL_CFLAGS := -DRIL_SHLIB -DMTK_RIL -DMTK_GEMINI
else
  LOCAL_CFLAGS := -DRIL_SHLIB -DMTK_RIL

  ifneq ($(MTK_SHARE_MODEM_SUPPORT),1)
        LOCAL_CFLAGS += -DMTK_GEMINI     
  endif
endif

ifeq ($(MTK_DT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_DT_SUPPORT
endif

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
LOCAL_CFLAGS += -DENG_BUILD
endif

LOCAL_CFLAGS += -DMTK_RIL_MD2

LOCAL_MODULE:= rildmd2

include $(BUILD_EXECUTABLE)

# For radiooptions binary
# =======================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	radiooptions.c

LOCAL_SHARED_LIBRARIES := \
	libcutils \

LOCAL_CFLAGS := \

LOCAL_MODULE:= radiooptions
LOCAL_MODULE_TAGS := debug

include $(BUILD_EXECUTABLE)

endif # GOOGLE_RELEASE_RIL
