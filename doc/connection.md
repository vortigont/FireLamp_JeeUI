| __[README](/README.md)__ | __[CHANGELOG](/CHANGELOG.md)__ |

![Schematics](/doc/schematic.jpg)

### Распиновка
Референсная таблица назначения выводов по-умолчанию

ниже указаны номера gpio, НЕ нумерация выводов на плате! Соответствие нумерации пинов платы и gpio чипа ищите в описании конкретной платы произодителя. (на wemos нумерация не совпадает с gpio)


<details>
 <summary>ESP32</summary>

![esp32 pinout](https://cdn.shopify.com/s/files/1/0609/6011/2892/files/doc-full_pinout_Wroom_rev2.png?width=1500)

Общие рекоммендации:
- пины 34-36,39 работают **только на вход!** и не имеют внутренней подтяжки.
- пины 0,2,12 должны быть подтянуты к "земле" при рестарте/прошивке контроллера иначе он не загрузится/не будет шиться.
- универсальные безопасные пины 4-5,13-14,16-33
 
|gpio | назначение | запрещенные пины |
|-|-|-|
|0     | подключение матрицы|  |
|34    | вход микрофона|
|4     | TM1637 CLK|
|5     | TM1637 DIO|
|17    | tx (DFPlayer rx)|
|16    | rx (DFPlayer tx)|
|2     | TM1637 Clk|
|13    | TM1637 Data|
|15    | N канальный МОП (N-MOSFET)|
|14    | ttp223 или обычная кнопка|

[Подробнее](https://www.upesy.com/blogs/tutorials/esp32-pinout-reference-gpio-pins-ultimate-guide#) о выводах esp32

</details>

<details>
 <summary>ESP8266</summary> 

![8266 pinout](https://i0.wp.com/randomnerdtutorials.com/wp-content/uploads/2019/05/ESP8266-WeMos-D1-Mini-pinout-gpio-pin.png)

 Общие рекоммендации:
- универсальные безопасные пины 4-5,12-14
- с остальными так или иначе есть ограничения, см ссылку ниже для более подробной информации
* - обозначает рекоммендованый пин для конкретного устройства. Пин может иметь ограничения для использования с другими устройствами.
 
|gpio | назначение |
|-|-|
|0*    | подключение матрицы|
|acd0* | вход микрофона|
|4     | TM1637 CLK  (i2c SDA)|
|5     | TM1637 DIO  (i2c SCL)|
|12    | tx (DFPlayer rx)|
|14    | rx (DFPlayer tx)|
|2     | TM1637 Clk|
|13    | TM1637 Data|
|15*   | N канальный МОП (N-MOSFET)  /P-канальный транзистор подключать нельзя! вывод имеет внутреннюю подтяжку к земле|
|16    | ttp223 или обычная кнопка (не поддерживает прерывания)|

[Подробнее](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/) о выводах 8266
</details>

Дополнительно: [справочники](https://community.alexgyver.ru/threads/wifi-lampa-budilnik-obsuzhdenie-proshivki-firelamp_embui.7257/post-145868) по выводам esp8266/esp32

