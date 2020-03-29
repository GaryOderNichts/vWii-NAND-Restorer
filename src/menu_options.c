#include "menu_options.h"
#include "extractor.h"
#include "utils/fsutils.h"
#include "main.h"

uint8_t OPTION_restoreNand(int fsaFd)
{
	console_printf(1, "Restoring NAND...");
	sleep(1);

	if (!initNand(NAND_FILE_PATH))
	{
		console_printf(1, "Error initializing NAND!");
		return 0;
	}
	console_printf(1, "Nand initialized successful");

	if (!OPTION_clearSlccmpt())
	{
		console_printf(1, "Error clearing SLCCMPT!");
		return 0;
	}
	
	console_printf(1, "Extracting NAND to SLCCMPT");

	extractNand("dev:", 1, fsaFd);
	cleanup();

	// if (OPTION_fixModes(fsaFd))
	// 	return 1;

	// console_printf(1, "Error fixing modes!");
	return 1;
}

uint8_t OPTION_extractNand(void)
{
	console_printf(1, "Extracting NAND to %s", EXTRACTED_NAND_PATH);
	sleep(1);
	if (initNand(NAND_FILE_PATH))
	{
		console_printf(1, "Nand initialized successful");
		if (dirExists(EXTRACTED_NAND_PATH))
			if (!removeRecursive(EXTRACTED_NAND_PATH))
				return 0;
		
		extractNand(EXTRACTED_NAND_PATH, 0, 0);
		cleanup();
		return 1;
	}
	console_printf(1, "Error extracting NAND!\n");
	return 0;
}

uint8_t OPTION_clearSlccmpt()
{
	console_printf(1, "Clearing SLCCMPT...");
	sleep(1);

	return removeRecursive("dev:");
}

uint8_t OPTION_fixModes(int fsaFd)
{
	console_printf(1, "Setting correct modes...");
	sleep(1);

	return chmodRecursive(fsaFd, "dev:", SLCCMPT_MODE);
}

uint8_t OPTION_copyNAND()
{   
	console_printf(1, "Copying NAND to SLCCMPT...");
	sleep(1);

	return copyDir(EXTRACTED_NAND_PATH, "dev:");
}