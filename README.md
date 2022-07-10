# TinyLamp
Проект "TinyLamp" - настольная мини-лампа с питанием от USB.

## Необходимые компоненты
- микроконтроллер ATtiny85
- модуль сенсорной кнопки на микросхеме TTP223
- светодиоды WS2812B (6 штук)
- распаяный microUSB/USB-C вход
- 1 резистор номиналом 100-500 Ом
- электролитический конденсатор (напряжением от 10В, ёмкость чем больше, тем лучше)
- программатор ISP (в качестве оного может выступать плата Arduino (UNO, Mega, Nano), либо популярный USBasp)

## Прошивка
Если у вас нет пакета ATtinyCore, добавьте его в менеджер плат: `http://drazzy.com/package_drazzy.com_index.json`.
Параметры прошивки в среде ArduinoIDE:
- выбираем плату ATtiny25/45/85(No bootloader)
- chip: ATtiny85
- Clock Source: 16MHz(PLL(
- Timer 1 Clock: CPU
- LTO: Enabled
- millis()/micros(): Enabled
- Save EEPROM: EEPROM retained
- B.O.D. Level: Disabled

Все библиотеки можно найти по их названию в менеджере библиотек.

## Сборка
Необходимые компоненты:
![scheme](/doc/components.jpg)

Перед сборкой прошить микроконтроллер. Перемычки на модуле TTP223 оставить разъединенными.
Светодиоды подключить последовательно, распиновка светодиодов такая:
![scheme](/doc/WS2812B_pinout.jpg)

Я собрал их в виде столбика:

![scheme](/doc/WS2812B_soldered.jpg)

Все компоненты подключить между собой как на картинке:
![scheme](/doc/scheme.jpg)

Если всё собрано правильно, светодиоды сразу будут вкючены на первый режим.

### Управление
Управление осуществляется одной кнопкой.
- 1 клик: включение/отключение лампы
- 2 клика: переключение режима на следующий
- 3 клика: переключение подрежимов в некоторых режимах
- длинное удержание: уменьшение/увеличение яркости. При отпускании кнопки меняется направление изменения яркости. При достижении предела вся лампа моргает белым цветом.

### Режимы
- 1: меняется цвет всей лампы по цветовому колесу (от красного к зеленому через желтый, затем к синему через голубой и обратно к красному через фиолетовый)
- 2: бегущая радуга — цвет меняется отдельно на каждом светодиоде
- 3: случайный "огонь" — каждый светодиод вспыхивает оранжевым цветом, плавно меняющемся к красному (имитация огня)
- 4: "свеча" — имитация горения свечи
- 5: падающая капля — сверху вниз падает "капля", и вся лампа "наполняется" светом. При наполнении плавно тухнет и всё начинается сначала
- 6: белый свет с разной световой температурой. Цвет меняется тройным кликом
- 7: различные статичные цвета и градиенты. Цвета меняются тройным кликом

### Сохранение настроек при отключении питания
В прошивке реализовано сохранение настроек в энергонезависимую память. Для увеличения ресурса памяти сохранение происходит спустя 10 секунд после последнего нажатия на кнопку.
Сохраняется последнее состояние лампы, яркость, последний режим и подрежимы.

### Как добавить свой цвет/градиент
В прошивке ищем функцию `SolidWhite()` (для изменения/добавления световой температуры), или `SolidColor()` для добавления/изменения цвета или градиента.

Если хотим добавить световую температуру (`SolidWhite()`):
1) Нужные параметры световой температуры в RGB формате можно узнать [тут](https://academo.org/demos/colour-temperature-relationship/). Просто выбираем нужну температуру в Кельвинах и копируем значения в скобках после rgb.
2) добавляем после последнего `break;` конструкцию:
```cpp
case x: strip.fill(mRGB(200, 240, 255));
        break;
```
Вместо `x` подставить следующий номер подфункции.

2.1) Если же хотим поменять, то в любой строке в скобки подставляете полученное значение цвета (там, где `mRGB(x, x, x)`)

3) В строке `if (subMode_mode5 > 3) subMode_mode5 = 0;` изменить цифру 3 на количество получившихся режимов.

Чтобы добавить цвет в функцию `SolidColor()` можно воспользоваться любой цветовой палитрой, например, [такой](https://g.co/kgs/9ZMMEp).
1) Находим понравившийся цвет. Нам нужны параметры RGB.
2) Аналогично предыдущему режиму, добавляем/меняем цвет. Если добавить цвет в промежуток между цветами, а не в конец, не забудьте сдвинуть последующую нумерацию подрежимов на единицу!
3) Не забываем поменять в строке `if (subMode_mode6 > 8) subMode_mode6 = 0;` количество подрежимов на итоговый.

Чтобы добавить градиент:
1) Градиент возможен из двух цветов. Подбираем необходимые параметры RGB в палитре.
2) Находим строку `strip.fillGradient(0, NUMLEDS, mRGB(255, 0, 0), mRGB(0, 0, 255));`. Третий и четвертый параметры отвечают за начальный и конечный цвет градиентв.
3) Меняем существующий/добавляем новый согласно предыдущим инструкциям, не забываем про количество режимов!

### Возможный вариант исполнения
Я распечатал корпус на 3D принтере. Итоговый вариант получился таким:
![scheme](/doc/mylamp.jpg)

Максимальное потребление в самом ярком режиме (белый цвет, полная яркость) не более 230мА.

### Обратная связь
Предложения/пожелания можно писать на [freewanhub@gmail.com](mailto:freewanhub@gmail.com)
