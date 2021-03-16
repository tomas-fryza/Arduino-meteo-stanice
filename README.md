# Jednoduchá meteo stanice

Cílem tohoto projektu je seznámení s populární platformou Arduino Uno, která obsahuje 8bitový mikrokontrolér AVR a je vhodná pro celou řadu řídicích aplikací. Bude vytvořen jednoduchý bod v síti Internet of Things, který bude představovat meteo stanici se senzorem teploty a relativní vlhkosti vzduchu. Pro odesílání dat na server ThingSpeak je použita WiFi komunikace.


## Použité komponenty

* Vývojová deska [Arduino Uno](https://arduino-shop.cz/arduino/1353-klon-arduino-uno-r3-atmega328p-ch340-mini-usb-1466635561.html)
* I2C senzor vlhkosti a teploty DHT12 nebo [DHT11](https://dratek.cz/arduino/1360-dht11-dps-vodice-mereni-teploty-a-relativni-vlhkosti-modul-pro-arduino.html): [data sheet DHT12](Docs/dht12_manual.pdf), [návod DHT11](https://navody.dratek.cz/navody-k-produktum/teplotni-senzor-dht11.html)
* WiFi modul [ESP8266](https://dratek.cz/arduino/911-internet-veci-je-tady-tcp-ip-wifi-esp8266-esp-01.html): [AT příkazy](Docs/esp8266_at_instruction_set.pdf)
* [Nepájivé pole](https://dratek.cz/302-nepajive-pole/), propojující [vodiče](https://dratek.cz/arduino/827-vodice-samec-samec-65-kusu.html)
* Prostředí [Arduino IDE](https://www.arduino.cc/en/software) pro Windows, Linux, Mac OS X
* Server [ThingSpeak](https://thingspeak.com/)

Pro ověření komunikace je vhodnou komponentou také:
* 24MHz 8-kanálový [logický analyzátor](https://www.ebay.com/sch/i.html?LH_CAds=&_ex_kw=&_fpos=&_fspt=1&_mPrRngCbx=1&_nkw=24mhz%20logic%20analyzer&_sacat=&_sadis=&_sop=12&_udhi=&_udlo=)
* [Ovládací software](https://www.saleae.com/downloads/) logického analyzátoru pro Windows, Linux, Mac OS X

![Zapojení na nepájivém poli](Images/arduino_komponenty.jpg)



## Blokové zapojení celého zařízení

![Blokový diagram](Images/block_diagram.jpg)



## Arduino Uno

Arduino je projekt vyvíjející otevřené platformy založené na 8bitovém mikrokontroléru s architekturou AVR (konkrétně [ATmega328P](https://www.microchip.com/wwwproducts/en/ATmega328P)). Podrobný popis HW modulů, včetně dostupných SW nástrojů, knihoven a manuálů naleznete na [webových stránkách](http://www.arduino.cc). Kromě originálu lze použít i levnější klony, které jsou plně kompatibilní.

![Vývojová deska Arduino Uno](Images/cv_arduino_uno2.jpg)

Řídicí program mikrokontroléru (tzv. firmware) je vyvíjen ve zjednodušené verzi jazyka C++ v prostředí [Arduino](https://www.arduino.cc/en/software), které je zdarma dostupné pro Windows, Mac OS X i Linuxové distribuce. Lze použít online verzi Arduino Web Editor (ke kterému je potřeba doinstalovat Arduino Create Agent pro programování hardwaru) nebo si stáhnout a lokálně nainstalovat prostředí Arduino IDE.

Toto prostředí je velmi jednoduché a kromě textového editoru nabízí jen několik ovládacích prvků. My využijeme pouze: **Verify** pro přeložení zdrojového kódu, **Upload** pro nahrání binární verze kódu do vývojové desky a **Serial Monitor** k otevření nového okna s asynchronní komunikaci mezi mikrokontrolérem a počítačem po USB kabelu.

![Vývojové prostředí Arduino IDE](Images/arduino_ide_example.png)



## Sériová komunikace se senzorem a UART

### Sběrnice I2C

Pomocí nepájivého pole a vodičů připojte na datový (SDA) a hodinový (SCL) signál sběrnice I2C modul pro měření teploty/vlhkosti DHT12. Pro napájení použijte napětí 3.3 V a GND z vývojové desky Arduina.

   | **DHT12** | **Arduino Uno** |
   | :-: | :-: |
   | + | 3.3V |
   | SDA | SDA |
   | - | GND |
   | SCL | SCL |

Kombinované čidlo komunikuje po digitální sériové sběrnici I2C (Inter-integrated Circuit). Ta obsahuje dva vodiče: pro přenos dat s označením SDA a pro přenos hodinových impulzů SCL a umožňuje snadné propojení jednoho nadřízeného obvodu (tzv. master) s více podřízenými obvody (slave). Jako master je použit mikrokontrolér AVR na vývojové desce Arduino Uno a slave obvod je čidlo teploty/vlhkosti DHT12 (lze ale připojit desítky dalších slave obvodů).

Při komunikaci na sběrnici I2C se nejprve adresuje podřízený obvod jediněčnou adresou, která je zadaná od výrobce (pro obvod DHT12 je to adresa 184 pro zápis a 185 pro čtení). Následně může proběhnout výměna dat jedním nebo druhým směrem.

Vnitřní uložení dat v senzoru DHT12:

   | **Adresa registru** | **Popis** |
   | :-: | :-- |
   | 0 | Celočíselná část vlhkosti |
   | 1 | Desetinná část vlhkosti |
   | 2 | Celočíselná část teploty |
   | 3 | Desetinná část teploty |
   | 4 | Kontrolní součet |

Hodnota relativní vlhkosti vzduchu i teploty se skládá z celočíselné a desetinné části. Pro zjištění obou veličin je tak potřebné přijmout ze senzoru 4 datové byty uložené od vnitřní adresy 0 až 3.

![I2C komunikace se senzorem](Images/analyser_humid_temp.png)

Pro ovládání komunikace I2C jsou využity funkce z knihovny [`Wire.h`](https://www.arduino.cc/en/reference/wire).

Detailnější informace o sériové komunikaci I2C je možné nalézt v materiálech předmětu [Digitální elektronika 2](https://github.com/tomas-fryza/Digital-electronics-2/tree/master/Labs/08-i2c).


### Asynchronní komunikace UART

Jednotka UART (Universal Asynchronous Receiver Transmiter) umožňuje převádět paralelní data na sériová a opačně. Obsahuje vysílací signál Tx i přijímací Rx a rychlost komunikace se udává v Bd (bódech). Typickou hodnotou je např. 9600 Bd.

Užitečná data jsou zabalena do komunikačního rámce, který začíná start bitem, následuje pět až devět datových bitů seřazených od LSB k~MSB, může být použit paritní bit pro základní ověření korektního příjmu (sudá nebo lichá). Komunikace je zakončena jedním nebo dvěma stop bity. Struktura rámce musí být dopředu domluvena a označuje se symbolicky: 7E2 (sedm datových bitů, sudá parita, dva stop bity), 8N1 (osm dat, bez parity, jeden stop), apod.

![UART komunikace](Images/uart_frame_8n1.png)

Ukázka UART signálu pro přenos hodnoty `0100_0101` (0x45, 69, ASCII kód písmene `E`) v módu 8N1:

![Přenos ASCII kódu písmene E](Images/uart_capture_E.png)

Pro ovládání sériové komunikace UART jsou využity interní funkce Arduina [`Serial`](https://www.arduino.cc/reference/en/language/functions/communication/serial/).

Detailnější informace o sériové komunikaci UART je možné nalézt v materiálech předmětu [Digitální elektronika 2](https://github.com/tomas-fryza/Digital-electronics-2/tree/master/Labs/07-uart).


### Zdrojový kód: zobrazení dat ze senzoru v Serial monitoru

Kompletní zdrojový kód aplikace pro čtení dat ze senzoru a jejich zobrazení v **Sériovém monitoru** naleznete v adresáři [Arduino > meteo_ver1](https://github.com/tomas-fryza/Arduino-meteo-stanice/blob/main/Arduino/meteo_ver1/meteo_ver1.ino). Pozor, v monitoru je potřeba nastavit stejnou symbolovou rychlost s jakou mikrokontrolér data vysílá, tj. 9600 baud.

![Serial monitor](Images/monitor_ver1_temp_humid.png)



## Server ThingSpeak

Pro vytvoření komunikačního kanálu pro sběr dat z vašeho senzoru postupujte podle následujících kroků:

1. V prohlížeči otevřete stránku https://thingspeak.com/ a stiskněte tlačítko **Get Started For Free**.

   ![Web ThingSpeak](Images/ts_webpage.png)

2. Vytvořte si účet--který je zdarma--pomocí vašeho emailu. Žádnou reklamu na email nerozesílají. Po vytvoření se do systému nalogujte.
3. Vytvořte nový komunikační kanál tlačítkem **New Channel**. Seznam kanálů naleznete kdykoliv v menu **Channels > My Channels**.
4. Vyplňte jméno kanálu, jeho případný popis a vyberte kolik hodnot budete v kanálu posílat. V části **Tags** můžete uvést klíčová slova, podle kterých bude váš kanál lépe dosažitelný. Vše uložte tlačítkem **Save Channel**.

   ![Tvorba nového kanálu](Images/ts_novy_kanal.png)

5. Nastavení kanálu můžete kdykoliv změnit a doplnit v záložce **Channel Settings**. Volbou **Sharing** lze nastavit, zda vaše data budou dostupná všem, nebo jen vám po nalogování. V záložce **API Keys** naleznete identifikátory, které byly pro váš kanál vygenerovány a jsou jeho jedinečným označením. V naší aplikaci budeme na server pouze zapisovat a budeme tak potřebovat pouze **Write API Key**, který později vložíme do zdrojového kódu aplikace.

   ![Write API Key](Images/ts_apikey.png)

6. Data je možné zobrazit odděleně pro vás (**Private View**) nebo pro všechny ostatní (**Public View**).



## WiFi modul ESP8266

Komunikační modul ESP8266 umožňuje využít bezdrátovou síť WiFi, připojit se k ní a odesílát data na server. Ovládání modulu probíhá prostřednictvím tzv. AT příkazů.

Pomocí nepájivého a vodičů pole připojte WiFi modul ESP8266 ESP-01 a Arduino Uno dle tabulky. Výměna dat mezi modulem a řídicím mikrokontrolérem probíhá prostřednictvím asynchronní komunikace. Všimněte si, že vysílací pin modulu je proto spojen s přijímacím pinem Arduino a obráceně.

![Připojení modulu ESP8266](Images/cv_esp8266_foto_description.jpg)

   | **ESP8266** | **Arduino Uno** |
   | :-: | :-: |
   | U0TXD | Rx (pin 0) |
   | CHIP_EN | 3.3V |
   | EXT_RSTB | Nepřipojeno |
   | 3.3V | 3.3V |
   | GND | GND |
   | GPIO2 | Nepřipojeno |
   | GPIO0 | Nepřipojeno |
   | U0RXD | Tx (pin 1) |



### Zdrojový kód: odeslání dat na server ThingSpeak

Použijte zdrojový kód z adresáře [Arduino > meteo_ver2](https://github.com/tomas-fryza/Arduino-meteo-stanice/blob/main/Arduino/meteo_ver2/meteo_ver2.ino) a překopírujte jej do prostředí Arduino IDE. Do kódu doplňte informace o vaší WiFi síti a APIWrite kód z předchozího bodu.

```c
String ssid = "xxx";        // SSID of your WiFi network
String password = "xxx";    // Password of your WiFi network
String writeApiKey = "xxx"; // Write API Key from ThingSpeak cloud
```

Kód přeložte a nahrajte do Arduino Uno. Vzhledem k tomu, že programování Arduina probíhá na stejných pinech, kde je připojen i WiFi modul, před samotným uploadem kódu odpojte vodiče z pinů 0 a 1 (Rx a Tx). Po úspěšném naprogramování vodiče opět připojte. Spusťte **Serial Monitor** na kterém pozorujte část komunikace s WiFi modulem (pozor, odpovědi modulu se nezobrazují a je nutné nastavit rychlost komunikace na 115200 baud):

![Serial monitor](Images/monitor_ver2_wifi.png)

Je-li komunikace prostřednictvím WiFi sítě korektní, na webové stránce `thingspeak.com` se začnou shromažďovat a zobrazovat změřená data. Pokud tomu tak není, zkontrolujte zadané údaje v kódu, případně si zobrazte asynchronní komunikaci na pinech 0 a 1 pomocí logického analyzátoru.

![Sběr dat na ThingSpeak](Images/ts_data.png)



## Modifikace projektu

Rozšířit tuto základí aplikaci lze celou řadou modulů a senzorů, např. tlak vzduchu, míra osvětlení, senzor srážek, síla větru, vlhkost půdy, aj. Velký výběr naleznete např. na serveru [dratek.cz](https://dratek.cz/).

Sběr meteodat, systém pro závlahu rostlin, LoRa síť: [bakalářská práce](https://www.facebook.com/URELBrno/posts/1459331204276701)



## Kontakt

[Tomáš Frýza](https://www.vutbr.cz/lide/tomas-fryza-11389)

Bakalářský studijní program [Elektronika a komunikační technologie](https://www.urel.fekt.vut.cz/bakalarsky-program) na Ústavu radioelektroniky, FEKT VUT v Brně.

[Facebook](https://www.facebook.com/URELBrno)

![Foto fakulty FEKT](Images/fekt_leto.jpg)
