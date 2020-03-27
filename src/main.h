#ifndef _MAIN_H_
#define _MAIN_H_

#include "common/types.h"
#include "common/common.h"
#include "dynamic_libs/os_functions.h"

#define NAND_FILE_PATH "sd:/vwiinandrestorer/slccmpt.bin"
#define EXTRACTED_NAND_PATH "sd:/vwiinandrestorer/extracted"
#define LOG_PATH "sd:/vwiinandrestorer/latest.log"

#define SLCCMPT_DEV_PATH "/dev/slccmpt01"
#define SLCCMPT_MOUNT_PATH "/vol/storage_slccmpt01"

#define SLCCMPT_MODE 0x666

/* Main */
#ifdef __cplusplus
extern "C" {
#endif

int Menu_Main(void);
void console_printf(int newline, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
