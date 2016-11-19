/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "microSD.h"	/* Example: Header file of existing MMC/SDC contorl module */
#include "spi_SD.h"
//#include "RTC/rtc.h"

/* Definitions of physical drive number for each drive */
#define ATA		0	/* Example: Map ATA harddisk to physical drive 0 */
#define MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	if(sd_status()) return 0;

	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
	int i;

	for (i=0; i<10; i++)
	{
		//error = SD_init();
		result = microSDInit();
		if(result) return 0;
	}

	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	long endSector = sector+count;
	DRESULT res;
	int result;
	int tries = 0;

	while(sector<endSector)
	{
		do {
			result = microSDRead(sector,buff);
			if(++tries>10) break;
		} while(!result);
		if(!result) return RES_ERROR;
		sector++;
		buff+=512;
	}

	return RES_OK;
	//return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	long endSector = sector+count;
	DRESULT res;
	int result;
	int tries = 0;

	//for(tries=0;tries<1e6;tries++);
	//tries = 0;
	while(sector<endSector)
	{
		do {
			result = microSDWrite(sector,buff);
			if(++tries>10) break;
		} while(!result);
		if(!result) return RES_ERROR;
		sector++;
		buff+=512;
	}


	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch(cmd)
	{
	case CTRL_SYNC:
		return RES_OK;
	case GET_SECTOR_COUNT:
		*(DWORD *)buff = 3836436;//1e6;
		return RES_OK;
	case GET_SECTOR_SIZE:
		*(WORD *)buff = 512;
		return RES_OK;
	case GET_BLOCK_SIZE:
		*(DWORD *)buff = 512;
		return RES_OK;
	default:
		break;
	}

	return RES_PARERR;
}

DWORD get_fattime (void)
{
	//return 673251328;
	DWORD fattime = 0;
	/*struct minmea_date cDate;
	struct minmea_time cTime;
	RTCgetDate(&cDate);
	RTCgetTime(&cTime);

	fattime = (cDate.year-1980) << 25;
	fattime |= cDate.month << 21;
	fattime |= cDate.day << 16;
	fattime |= cTime.hours << 11;
	fattime |= cTime.minutes << 5;
	fattime |= cTime.seconds >> 1;*/

	return fattime;
}
