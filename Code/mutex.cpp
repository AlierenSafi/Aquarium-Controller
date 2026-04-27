#include "mutex.h"

/* FreeRTOS'suz versiyon - mutex'ler gercek degil */
SemaphoreHandle_t xI2CMutex, xOneWireMutex,
                  xEEPROMMutex, xSerialMutex,
                  xRTCMutex, xTempMutex, xRelayMutex;

void initMutexes() {
    /* FreeRTOS'suz versiyonda mutex'lere gerek yok */
    /* Tum handle'lari NULL olarak ayarla */
    xI2CMutex = xOneWireMutex = xEEPROMMutex = xSerialMutex = 
    xRTCMutex = xTempMutex = xRelayMutex = nullptr;
}
