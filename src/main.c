#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "fs/sd_fat_devoptab.h"
#include "menu_options.h"
#include <iosuhax.h>
#include <iosuhax_devoptab.h>
#include "utils/fsutils.h"

#define MAX_CONSOLE_LINES_TV    27
#define MAX_CONSOLE_LINES_DRC   18

static char* consoleArrayTv[MAX_CONSOLE_LINES_TV];
static char* consoleArrayDrc[MAX_CONSOLE_LINES_DRC];

static void console_print_pos(int x, int y, const char *format, ...)
{
	char* tmp = NULL;

	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
		if(strlen(tmp) > 79)
			tmp[79] = 0;

		OSScreenPutFontEx(0, x, y, tmp);
		OSScreenPutFontEx(1, x, y, tmp);

	}
	va_end(va);

	if(tmp)
		free(tmp);
}

uint8_t enableLogToFile = 0;

void console_printf(int newline, const char *format, ...)
{
	char* tmp = NULL;

	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
		if(newline)
		{
			if(consoleArrayTv[0])
				free(consoleArrayTv[0]);
			if(consoleArrayDrc[0])
				free(consoleArrayDrc[0]);

			for(int i = 1; i < MAX_CONSOLE_LINES_TV; i++)
				consoleArrayTv[i-1] = consoleArrayTv[i];

			for(int i = 1; i < MAX_CONSOLE_LINES_DRC; i++)
				consoleArrayDrc[i-1] = consoleArrayDrc[i];
		}
		else
		{
			if(consoleArrayTv[MAX_CONSOLE_LINES_TV-1])
				free(consoleArrayTv[MAX_CONSOLE_LINES_TV-1]);
			if(consoleArrayDrc[MAX_CONSOLE_LINES_DRC-1])
				free(consoleArrayDrc[MAX_CONSOLE_LINES_DRC-1]);

			consoleArrayTv[MAX_CONSOLE_LINES_TV-1] = NULL;
			consoleArrayDrc[MAX_CONSOLE_LINES_DRC-1] = NULL;
		}

		if(strlen(tmp) > 79)
			tmp[79] = 0;

		consoleArrayTv[MAX_CONSOLE_LINES_TV-1] = (char*)malloc(strlen(tmp) + 1);
		if(consoleArrayTv[MAX_CONSOLE_LINES_TV-1])
			strcpy(consoleArrayTv[MAX_CONSOLE_LINES_TV-1], tmp);

		consoleArrayDrc[MAX_CONSOLE_LINES_DRC-1] = (tmp);

		if (enableLogToFile)
		{
			FILE* logfile = fopen(LOG_PATH, "a");
			if (logfile != NULL)
			{
				fprintf(logfile, "%s\n", tmp);
			}
			fclose(logfile);
		}
		
	}
	va_end(va);

	// Clear screens
	OSScreenClearBufferEx(0, 0);
	OSScreenClearBufferEx(1, 0);


	for(int i = 0; i < MAX_CONSOLE_LINES_TV; i++)
	{
		if(consoleArrayTv[i])
			OSScreenPutFontEx(0, 0, i, consoleArrayTv[i]);
	}

	for(int i = 0; i < MAX_CONSOLE_LINES_DRC; i++)
	{
		if(consoleArrayDrc[i])
			OSScreenPutFontEx(1, 0, i, consoleArrayDrc[i]);
	}

	OSScreenFlipBuffersEx(0);
	OSScreenFlipBuffersEx(1);
}

//just to be able to call async
void someFunc(void *arg)
{
	(void)arg;
}

static int mcp_hook_fd = -1;
int MCPHookOpen()
{
	//take over mcp thread
	mcp_hook_fd = MCP_Open();
	if(mcp_hook_fd < 0)
		return -1;
	IOS_IoctlAsync(mcp_hook_fd, 0x62, (void*)0, 0, (void*)0, 0, someFunc, (void*)0);
	//let wupserver start up
	sleep(1);
	if(IOSUHAX_Open("/dev/mcp") < 0)
		return -1;
	return 0;
}

void MCPHookClose()
{
	if(mcp_hook_fd < 0)
		return;
	//close down wupserver, return control to mcp
	IOSUHAX_Close();
	//wait for mcp to return
	sleep(1);
	MCP_Close(mcp_hook_fd);
	mcp_hook_fd = -1;
}

int Menu_Main(void)
{
	InitOSFunctionPointers();
	InitSocketFunctionPointers();
	InitFSFunctionPointers();
	InitVPadFunctionPointers();

	VPADInit();

	for(int i = 0; i < MAX_CONSOLE_LINES_TV; i++)
		consoleArrayTv[i] = NULL;

	for(int i = 0; i < MAX_CONSOLE_LINES_DRC; i++)
		consoleArrayDrc[i] = NULL;

	// Prepare screen
	int screen_buf0_size = 0;

	// Init screen and screen buffers
	OSScreenInit();
	screen_buf0_size = OSScreenGetBufferSizeEx(0);
	OSScreenSetBufferEx(0, (void *)0xF4000000);
	OSScreenSetBufferEx(1, (void *)(0xF4000000 + screen_buf0_size));

	OSScreenEnableEx(0, 1);
	OSScreenEnableEx(1, 1);

	// Clear screens
	OSScreenClearBufferEx(0, 0);
	OSScreenClearBufferEx(1, 0);

	// Flip buffers
	OSScreenFlipBuffersEx(0);
	OSScreenFlipBuffersEx(1);

	int mountsdres = mount_sd_fat("sd");
	if (mountsdres < 0)
	{
		console_printf(1, "Error mounting sd: %i\n", mountsdres);
		sleep(2);
		return 0;
	}
	
	if (fileExists(LOG_PATH))
	{
		remove(LOG_PATH);
	}

	enableLogToFile = 1;

	int res = IOSUHAX_Open(NULL);
	if (res < 0)
		res = MCPHookOpen();
	if(res < 0)
	{
		console_printf(1, "IOSUHAX_open failed: %i\n", res);
		console_printf(1, "Did you forget to run CFW?\n");
		sleep(4);
		return 0;
	}

	int fsaFd = IOSUHAX_FSA_Open();
	if(fsaFd < 0)
	{
		console_printf(1, "IOSUHAX_FSA_Open failed: %i\n", fsaFd);
		sleep(2);
		return 0;
	}

	int mountres = mount_fs("dev", fsaFd, SLCCMPT_DEV_PATH, SLCCMPT_MOUNT_PATH);
	if(mountres < 0)
	{
		console_printf(1, "%i Mount of %s to %s failed", mountres, SLCCMPT_DEV_PATH, SLCCMPT_MOUNT_PATH);
		sleep(5);
		return 0;
	}

	int vpadError = -1;
	VPADData vpad;

	int initScreen = 1;

	static const char* default_menu_options[] =
	{
		"Restore NAND",
	};

	static const char* additional_menu_options[] =
	{
		"Extract NAND",
		"Clear SLCCMPT",
		"Copy extracted NAND to SLCCMPT",
		"Fix modes",
	};

	int selectedItem = 0;

	while (1)
	{
		VPADRead(0, &vpad, 1, &vpadError);

		if (initScreen)
		{
			initScreen = 0;	

			//! free memory
			for(int i = 0; i < MAX_CONSOLE_LINES_TV; i++)
			{
				if(consoleArrayTv[i])
					free(consoleArrayTv[i]);
				consoleArrayTv[i] = 0;
			}

			for(int i = 0; i < MAX_CONSOLE_LINES_DRC; i++)
			{
				if(consoleArrayDrc[i])
					free(consoleArrayDrc[i]);
				consoleArrayDrc[i] = 0;
			}

			// Clear screens
			OSScreenClearBufferEx(0, 0);
			OSScreenClearBufferEx(1, 0);

			console_print_pos(0, 0, "vWii NAND Restorer v1 by GaryOderNichts");

			console_print_pos(0, 2, "Place your vWii NAND Backup to %s", NAND_FILE_PATH);

			uint32_t defi;
			for (defi = 0; defi < (sizeof(default_menu_options) / 4); defi++)
			{
				if(selectedItem == (int)defi)
				{
					console_print_pos(0, 4 + defi, "--> %s", default_menu_options[defi]);
				}
				else
				{
					console_print_pos(0, 4 + defi, "    %s", default_menu_options[defi]);
				}
			}

			uint32_t addstart = 4 + defi + 1;
			console_print_pos(0, addstart, "Additional Options:");

			uint32_t addi;
			for (addi = 0; addi < (sizeof(additional_menu_options) / 4); addi++)
			{
				if(selectedItem == (int) (defi + addi))
				{
					console_print_pos(0, addstart + 2 + addi, "--> %s", additional_menu_options[addi]);
				}
				else
				{
					console_print_pos(0, addstart + 2 + addi, "    %s", additional_menu_options[addi]);
				}
			}

			if(selectedItem == (int) (defi + addi))
			{
				console_print_pos(0, addstart + addi + 3, "--> Exit");
			}
			else
			{
				console_print_pos(0, addstart + addi + 3, "    Exit");
			}

			OSScreenFlipBuffersEx(0);
			OSScreenFlipBuffersEx(1);
		}

		if(vpadError == 0 && vpad.btns_d & VPAD_BUTTON_DOWN)
		{
			selectedItem = (selectedItem + 1) % ((sizeof(default_menu_options) / 4) + ((sizeof(additional_menu_options) / 4) + 1));
			initScreen = 1;
		}

		if(vpadError == 0 && vpad.btns_d & VPAD_BUTTON_UP)
		{
			selectedItem--;
			if(selectedItem < 0)
				selectedItem =  ((sizeof(default_menu_options) / 4) + ((sizeof(additional_menu_options) / 4) + 1)) - 1;
			initScreen = 1;
		}

		if(vpadError == 0 && vpad.btns_d & VPAD_BUTTON_A)
		{
			switch (selectedItem)
			{
			case 0: // Restore nand
				if (OPTION_restoreNand(fsaFd))
					sleep(2);
				else
					sleep(6);
				break;
			case 1: // Extract nand
				if (OPTION_extractNand())
					sleep(2);
				else 
					sleep(6);
				break;
			case 2: // Clear SLCCMPT
				if (OPTION_clearSlccmpt())
					sleep(2);
				else
					sleep(6);
				break;
			case 3: // Copy extracted NAND to SLCCMPT
				if (OPTION_copyNAND())
					sleep(2);
				else
					sleep(6);
				break;
			case 4: // Fix modes
				if (OPTION_fixModes(fsaFd))
					sleep(2);
				else
					sleep(6);
				break;
			case 5: // exit
				goto exit;
			default:
				break;
			}
			initScreen = 1;
		}

		if(vpadError == 0 && ((vpad.btns_d | vpad.btns_h) & VPAD_BUTTON_HOME))
		{
			break;
		}

		usleep(20000);
	}
	exit: 
	
	console_printf(1, "Exiting...");

	for(int i = 0; i < MAX_CONSOLE_LINES_TV; i++)
	{
		if(consoleArrayTv[i])
			free(consoleArrayTv[i]);
	}

	for(int i = 0; i < MAX_CONSOLE_LINES_DRC; i++)
	{
		if(consoleArrayDrc[i])
			free(consoleArrayDrc[i]);
	}

	unmount_fs("dev");

	unmount_sd_fat("sd");

	IOSUHAX_FSA_Close(fsaFd);

	if(mcp_hook_fd >= 0)
		MCPHookClose();
	else
		IOSUHAX_Close();

	sleep(1);

	return 0;
}

