# vWii-NAND-Restorer
Restore your vWii NAND backups from your Wii U

## Note: I recommend using [vWii Decaffeinator](https://github.com/GaryOderNichts/vWii-Decaffeinator) to restore (partially) bricked vWii's instead

## How to use
- Copy your nand backup to `sd:/vwiinandrestorer/slccmpt.bin` and place your `keys.bin/otp.bin` in the same folder
- Copy the .elf to sd:/wiiu/apps
- Run cfw (mocha or haxchi)

I recommend doing a slccmpt + otp backup with [Wii U Nand dumper](https://github.com/koolkdev/wiiu-nanddumper/releases) before using this tool.  
Please make sure that the `keys.bin/otp.bin` are from the console where the NAND Backup is from or else decryption will fail!

###### Restore NAND
This will clear your SLCCMPT, extract the nand backup to your slccmpt and fix permissions

##### Additional Options:

###### Extract NAND
Extracts your NAND to `sd:/vwiinandrestorer/extracted`

###### Clear SLCCMPT
This clears your SLCCMPT

###### Copy extracted NAND to SLCCMPT
Copies everything from `sd:/vwiinandrestorer/extracted` to your SLCCMPT  
This can be useful if you have an already extracted NAND Backup that you want to restore

###### Fix modes
This fixes the modes of your SLCCMPT  
Needed after copying file to the SLCCMPT

### Logging
A log of the console output gets written to `sd:/vwiinandrestorer/latest.log`
If you want to report an issue please also send that logfile

## Changelog
```
    v2
 - Add support for 'Dump Mii Nand' Nand Dumps
 - Fix issue that may have resulted in incorrect file sizes for a few files

    v1
 - Initial release
```

## Disclaimer
I am not responsible for any damage while using this tool or any additional bricks done to your device!

## Build
Download devkitpro ppc, portlibs, libogc and build and install latest libiosuhax.
Then run `make`
