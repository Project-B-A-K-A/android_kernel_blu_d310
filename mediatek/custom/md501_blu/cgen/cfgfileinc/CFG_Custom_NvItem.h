#ifndef _CFG_CUSTOM_NVITEM_H
#define _CFG_CUSTOM_NVITEM_H

//silongjun  add for MDWLYBLU-999  20130329 (begin)

typedef   struct  
{
	unsigned char  array[1024];              //0-----5      saved for OTA 
								//6-----10   for new feature 
									

}TEST_NV_DEFAULT;


#define CFG_FILE_TEST_NV_SIZE      sizeof(TEST_NV_DEFAULT)
#define CFG_FILE_TEST_NV_TOTAL   1





//silongjun  add for MDWLYBLU-999  20130329 (end)



#endif
