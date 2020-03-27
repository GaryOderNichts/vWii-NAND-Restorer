#ifndef _MENU_OPTIONS_H_
#define _MENU_OPTIONS_H_

#include <stdint.h>

uint8_t OPTION_restoreNand(int fsaFd);
uint8_t OPTION_extractNand(void);
uint8_t OPTION_clearSlccmpt();
uint8_t OPTION_fixModes(int fsaFd);
uint8_t OPTION_copyNAND();

#endif