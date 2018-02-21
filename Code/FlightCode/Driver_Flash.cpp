#include "Driver_Flash.h"

#include <SPI.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_SPIFlash_FatFs.h>

#define FLASH_TYPE     SPIFLASHTYPE_W25Q16BV
#define FLASH_SS       SS1
#define FLASH_SPI_PORT SPI1
Adafruit_SPIFlash flash(FLASH_SS, &FLASH_SPI_PORT);
Adafruit_W25Q16BV_FatFs fatfs(flash);
#define FILE_NAME      "data.txt"

// IMPORTANT: before running this code, run fatfs_format.ino (example in the 
// the Adafruit_SPIFlash library) to add the FAT file system. If a "log cache
// has run out of memory" error occurs, increase CACHE_SIZE to match. 

#define CACHE_SIZE      1000
String cache[CACHE_SIZE];
int cache_head = 0;

void Flash_Init()
{
    Serial.begin(9600);
    cache_head = 0;
}

void Flash_Log(String message)
{
    if (cache_head < CACHE_SIZE)
    {
        cache[cache_head] = message;
        cache_head++;
    }
    else
    {
        Serial.println("Log cache has run out of memory! See Driver_Flash.cpp");
    }
    
}

void Flash_Close()
{
  if (!flash.begin(FLASH_TYPE)) 
  {
    Serial.println("Could not initialize flash! See Driver_Flash.cpp");
    return;
  }
  if (!fatfs.begin()) 
  {
    Serial.println("Could not mount FS!");
    return;
  }
  File flashDataFile = fatfs.open(FILE_NAME, FILE_WRITE);
  
  if (!flashDataFile) 
  {
    Serial.println("Could not open file to log to! See Driver_Flash.cpp");
    return;
  }
  
  for (int i = 0; i < cache_head; i++)
  {
    String payload = "s";
    Serial.println(cache[i]);
    
    flashDataFile.println(payload);
  }
  flashDataFile.close();
}
