#include "main.h"
#include "SPI_VOC.h"


// ----------------------------------------------
// this function receive on SPI the VOC data
// ----------------------------------------------

void VOCReceive(SPI_HandleTypeDef hspi3,uint8_t* vocReceivedSPI1) {
      if( HAL_SPI_Receive(&hspi3,(uint8_t*) (vocReceivedSPI1),1,1000) != HAL_OK) {
            Print_DEBUG("Error in data reception SPI3");
          Error_Handler();
      }

}