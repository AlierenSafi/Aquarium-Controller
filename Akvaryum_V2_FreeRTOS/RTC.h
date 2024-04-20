#ifndef __RTC_H
#define __RTC_H

//#include <TimeLib.h>
//#include <Wire.h>
//#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include "RTClib.h"
RTC_DS1307 rtc;

enum zaman { YEAR,
             MONTH,
             DAY,
             HOUR,
             MINUTE,
             SECOND
           };
enum dt { TIME,
          DATE
        };

char HaftaninGunleri[7][12] = { "Pazar", "Pazartesi", "Salı",
                                "Çarşamba", "Perşembe", "Cuma", "Cumartesi"
                              };


int _saniye = 0, _dakika = 0, _saat = 0, _gun = 0, _ay = 0, _yil = 0;


void rtcBaslat() {
  if (!rtc.begin()) {  // Modüle bağlantı kontrol ediliyor
    Serial.println("RTC Bulunamadı!");
    while (1)
      ;
  }
  if (!rtc.isrunning()) {  // RTC'nin ayarlı olup olmadığı kontrol ediliyor
    Serial.println("RTC çalışmıyor!");
    // Alttaki satır Arduino IDE'den gelen saat ve tarihi alarak RTC'yi ayarlar
    // Bu halde bir defalığına Arduino'ya yükledikten sonra satırı başına //
    // koyarak yorum yapıp tekrar yüklemeliyiz.
    // Aksi halde her açılışta tekrar saat yüklemeye çalışır.
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}


#endif
