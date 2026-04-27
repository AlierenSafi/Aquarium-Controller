#pragma once

/* FreeRTOS'suz versiyon - mutex'ler devre disi */
/* Mutex handle'lari (geri uyumluluk icin) */
typedef void* SemaphoreHandle_t;
extern SemaphoreHandle_t xI2CMutex, xOneWireMutex,
                          xEEPROMMutex, xSerialMutex,
                          xRTCMutex, xTempMutex, xRelayMutex;

void initMutexes();

/* Mutex makrolari - FreeRTOS'suz versiyonda her zaman basarili */
#define MUTEX_TAKE(m)  (true)
#define MUTEX_GIVE(m)  do {} while(0)
