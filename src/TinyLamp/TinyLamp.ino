/*
  Скетч проекта "TinyLamp"
  Автор: FreeWan

  Версия 1.0
*/

// *******************НАСТРОЙКИ*******************

// ---- номер пина, к которому подключена лента
#define STRIP_PIN 0

// ---- количество светодиодов в ленте
#define NUMLEDS 6
// ВНИМАНИЕ! От количества светодиодов меняется количество занимаемой программой памяти! Менять в большую сторону только если есть понимание, как уменьшить занимаемый объем памяти

// ---- номер пина, к которому подключена кнопка
#define BTN_PIN 2
// Для стабильной работы рекомендую использовать кнопку TTP223

// ---- глубина цвета
#define COLOR_DEBTH 3
// Значение от (1) до (3). При малом количестве светодиодов слабо влияет на количество занимаемой памяти

// ---- коррекция по CRT гамме
#define CRT_OFF
// Отключена по умолчанию на весь скетч. В скетче в некоторых эффектах применяется коррекция конкретно к эффектам, иначе эффекты, в которых она не нужна, выглядят некорректно

// ---- время "дебаунса" кнопки, мс
#define EB_DEB 0
// При использовании сенсорной кнопки дебаунс не нужен, так как она не "шумит"

// ---- таймаут "кликов", мс
#define EB_CLICK 300


// *******************БИБЛИОТЕКИ*******************

#include <microLED.h>
#include <TimerMs.h>
#include <EncButton.h>
#include <EEPROM.h>
// ВНИМАНИЕ! Используется модифицированная библиотека EncButton.h! Для корректной работы использовать версию, приложенную к проекту!


// *******************ИНИЦИАЛИЗАЦИЯ*******************

TimerMs timer1(15, true), btnTimer(1000, 0, 1), timerRandom(120, true), timerTick(16, true), timer_1(15, true), timer_2(18, true), timerSave(10000, 0, 1);
microLED<NUMLEDS, STRIP_PIN, MLED_NO_CLOCK, LED_WS2812, ORDER_GRB, CLI_HIGH, SAVE_MILLIS> strip;
EncButton<EB_TICK, BTN_PIN> button;
byte BRIGHTNESS = 255;
byte mode = 0, subMode_mode5 = 0, subMode_mode6 = 0;
boolean isOn = true;
uint16_t* universal_pointer;

// ---- структура для хранения настроек
struct Settings {
  byte brightness = 0;
  byte mode = 0;
  byte subMode_mode5 = 0;
  byte subMode_mode6 = 0;
  boolean isOn = true;
};

Settings settings;


// *******************ДЛЯ РАЗРАБОТЧИКОВ*******************

void setup() {
  // ---- проверка на первое включение
  if (EEPROM.read(0) != 100) {      // если включение первое, тогда записываем ключ в ячейку 0, и сохраняем текущие настройки
    EEPROM.update(0, 100);
    settings.brightness = BRIGHTNESS;
    settings.mode = mode;
    settings.subMode_mode5 = subMode_mode5;
    settings.subMode_mode6 = subMode_mode6;
    settings.isOn = isOn;
    EEPROM.put(1, settings);
  } else {                          // иначе считываем настройки и применяем их
    EEPROM.get(1, settings);
    BRIGHTNESS = settings.brightness;
    mode = settings.mode;
    isOn = settings.isOn;
    subMode_mode5 = settings.subMode_mode5;
    subMode_mode6 = settings.subMode_mode6;
  }
  
  button.setHoldTimeout(1000);      // таймаут удержания кнопки, мс
  button.setStepTimeout(15);        // таймаут шага, мс
  button.setButtonLevel(HIGH);      // по какому уровню считывать нажатие

  // ---- восстановление последнего состояния лампы
  if (isOn) {
    strip.setBrightness(BRIGHTNESS);
  } else {
    strip.setBrightness(0);
  }

  // !не менять для корректной работы!
  strip.oneLedIdle = 1200;
  strip.oneLedMax = 40;
  strip.setMaxCurrent(10000);
  
  strip.clear();
  strip.show();
  delay(1);
}


void loop() {
  button.tick();                        // опрос кнопки
  button_process();                     // обработчик нажатий на кнопку
  if (mode > 6) mode = 0;               // количество режимов, при добавлении не забывать увеличивать число!
  select_mode(mode);
  if (timerSave.tick()) SaveSettings(); // сохранение настроек по таймеру
}

// *******************ОБРАБОТЧИК НАЖАТИЙ НА КНОПКУ*******************

void button_process() {
  static boolean br_up = false;                 // направление смены яркости
  
  if (button.hasClicks(1)) SwitchLampState();   // один клик - меняем эффект
  
  if (button.step() && isOn) {                  // настройка яркости. Меняется только если лампа светится. При достижении предела вся лампа моргает белым цветом
    if (br_up && BRIGHTNESS < 255) {
      BRIGHTNESS++;
      strip.setBrightness(BRIGHTNESS);
    } else if (br_up && BRIGHTNESS > 254) {
      strip.fill(mRGB(100, 100, 100));
      strip.show();
    }
    if (!br_up && BRIGHTNESS > 30) {            // минимальное значение яркости - 30
      BRIGHTNESS--;
      strip.setBrightness(BRIGHTNESS);
    } else if (!br_up && BRIGHTNESS < 31) {
      strip.fill(mRGB(100, 100, 100));
      strip.show();
    }
  }
  
  if (button.releaseStep() && isOn) { 
    br_up = !br_up;                             // смена направления изменения яркости после отпускания кнопки после удержания
  }
  
  if (button.hasClicks(2) && isOn) {            // 2 клика - поменять эффект на следующий
    *universal_pointer = 0;
    strip.clear();
    mode++;
  }
  
  if (button.release()) {                       // запустить таймер на сохранение настроек после отпускания кнопки
    timerSave.start();
  }
  
  if (button.hasClicks(3)) {                    // в некоторых режимах есть подрежимы - меняюся тройным кликом
    if (mode == 5) {
      subMode_mode5++;
    } else if (mode == 6) {
      subMode_mode6++;
    }
  }
}

void select_mode(byte mode) {
  switch (mode) {
    case 0: SolidRainbow();
      break;
    case 1: CircleRainbow();
      break;
    case 2: RandomFire();
      break;
    case 3: CandleFire();
      break;
    case 4: Raindrop();
      break;
    case 5: SolidWhite();
      break;
    case 6: SolidColor();
      break;
  }
}

// один цвет на всю ленту, плавно меняется цвет по цветовому колесу.
void SolidRainbow() {
  timer1.setTime(15);
  static int h = 0;
  if (timer1.tick()) {
    strip.fill(mWheel(h++));
    if (h > 1530) h = 0;
    strip.show();
  }
}

// бегущая радуга
void CircleRainbow() {
  static int h = 0;
  timer1.setTime(30);
  if (timer1.tick()) {
    if (h > 1530) h = 0;
    for (byte i = 0; i < NUMLEDS; i++) {
      if (i * 1530 / NUMLEDS + h > 1530) {
        strip.set(i, mWheel(i * 1530 / NUMLEDS + h++ - 1530));
      } else {
        strip.set(i, mWheel(i * 1530 / NUMLEDS + h++));
      }
    }
    strip.show();
  }
}

// случайные вспышки "огня" по всей ленте
void RandomFire() {
  static byte zoneValue[NUMLEDS], zoneValueF[NUMLEDS];

  if (timerRandom.tick()) {
    for (byte i = 0; i < NUMLEDS; i++) zoneValueF[i] = random(0, 10);
  }

  if (timerTick.tick()) {
    for (byte i = 0; i < NUMLEDS; i++) {
      zoneValue[i] = (float)zoneValue[i] * 0.8 + (float)zoneValueF[i] * 2;
      byte H = map(zoneValue[i], 0, 100, 15, 35);
      byte S = constrain(map(zoneValue[i], 0, 100, 255, 170), 0, 255);
      byte V = constrain(map(zoneValue[i], 0, 100, 50, 255), 0, 255);
      strip.set(i, mergeRGB(getCRT_CUBIC(getR(mHSVfast(H, S, V))), getCRT_CUBIC(getG(mHSVfast(H, S, V))), getCRT_CUBIC(getB(mHSVfast(H, S, V)))));

    }
    strip.show();
  }
}

// режим "Свеча"
void CandleFire() {
  timer1.setTime(40);
  static byte heat[NUMLEDS];
  static byte cooling = 40;           // скорость "охдаждения" ленты. Больше значение - короче вспышки, быстрое затухание
  static byte sparking = 200;         // количество "вспышек". Чем больше значение, тем стабильнее "пламя"; в некотором роде имитация дуновений ветра
  
  if (timer1.tick()) {
    for (byte i = 0; i < NUMLEDS; i++) {
      heat[i] = max((heat[i] - random(0, ((cooling * 10) / NUMLEDS) + 2)), 0);
    }
    for (byte i = NUMLEDS - 1; i >= 1; i--) {
      heat[i] = (heat[i] + (heat[i - 1] * 2)) / 3;
    }
    if (random(0, 255) < sparking) {
      heat[0] = min((heat[0] + random(200, 240)), 255);
    }
    for (byte i = 0; i < NUMLEDS; i++) {
      byte colorIndex = (heat[i] * 240) / 256;
      byte H = map(colorIndex, 0, 239, 15, 35);
      byte S = constrain(map(colorIndex, 60, 239, 255, 180), 0, 255);
      byte V = constrain(map(colorIndex, 20, 160, 0, 255), 10, 255);
      strip.set(i, mergeRGB(getCRT_CUBIC(getR(mHSVfast(H, S, V))), getCRT_CUBIC(getG(mHSVfast(H, S, V))), getCRT_CUBIC(getB(mHSVfast(H, S, V)))));
    }
    strip.show();
  }
}

// режим "Падающая капля"
void Raindrop() {
  static byte value = 0, crit_mass = random(50, 150);
  static uint16_t pillar_value = 0;
  universal_pointer = &pillar_value;
  if (timer_1.tick()) {
    strip.set(NUMLEDS - 1, mHSVfast(0, 0, value));
    if (value < 256) {
      value++;
      strip.show();
    }
  }
  if (timer_2.tick()) {
    if (pillar_value > 0) {
      setPillarValue(--pillar_value);
      strip.show();
    }
  }
  if (value >= crit_mass) {
    value = 0;
    crit_mass = random(50, 150);
    for (byte i = 5; i > pillar_value / 256; i--) {
      if (getValue(i - 1) != 0) {
        pillar_value += getValue(i);
        setValue(i, 0);
        setPillarValue(pillar_value);
        break;
      } else if (getValue(i - 1) == 0) {
        setValue(i - 1, getValue(i));
        setValue(i, 0);
        strip.show();
        delay(40);
      }
    }
  } else if (pillar_value >= (NUMLEDS - 1) * 255) {
    while (getValue(NUMLEDS - 1) < 255) {
      setValue(NUMLEDS - 1, getValue(NUMLEDS - 1) + 1);
      strip.show();
      delay(15);
    }
    for (int i = 255; i > -1; i--) {
      strip.fill(mHSVfast(0, 0, i));
      strip.show();
      delay(30);
    }
    pillar_value = 0;
  }
}

// белый цвет с заранее вычесленной цветовой температурой 
void SolidWhite() {
  if (subMode_mode5 > 3) subMode_mode5 = 0;
  if (timer1.tick()) {
    switch (subMode_mode5) {
      case 0: strip.fill(mRGB(255, 101, 0));    //warm (K = 1546)
        break;
      case 1: strip.fill(mRGB(255, 215, 183));  //warm (K = 4400)
        break;
      case 2: strip.fill(mRGB(255, 255, 255));  //normal (K = 6600)
        break;
      case 3: strip.fill(mRGB(178, 203, 255));  //cold (K = 16000)
        break;
    }
    strip.show();
  }
}

// разные цвета на всю ленту. Основные цвета (RGB и CMYK) + несколько градиентов
void SolidColor() {
  if (subMode_mode6 > 8) subMode_mode6 = 0;
  if (timer1.tick()) {
    switch (subMode_mode6) {
      case 0: strip.fill(mRGB(255, 0, 0));      // красный
        break;
      case 1: strip.fill(mRGB(0, 255, 0));      // зеленый
        break;
      case 2: strip.fill(mRGB(0, 0, 255));      // синий
        break;
      case 3: strip.fill(mRGB(255, 255, 0));    // желтый
        break;
      case 4: strip.fill(mRGB(0, 255, 255));    // бирюзовый
        break;
      case 5: strip.fill(mRGB(255, 0, 255));    // розовый
        break;
      case 6: strip.fillGradient(0, NUMLEDS, mRGB(255, 0, 0), mRGB(0, 0, 255));       // градиент из красного в синий
        break;
      case 7: strip.fillGradient(0, NUMLEDS, mRGB(41, 10, 89), mRGB(255, 124, 0));    // градиент из фиолетового в оранжевый
        break;
      case 8: strip.fillGradient(0, NUMLEDS, mRGB(120, 0, 255), mRGB(0, 255, 150));   // градиент из фиолетового в аквамариновый
        break;
    }
    strip.show();
  }
}

byte getValue(byte index) {
  byte _maximum;
  _maximum = getR(strip.get(index)) > getG(strip.get(index)) ? getR(strip.get(index)) : getG(strip.get(index));
  _maximum = _maximum > getB(strip.get(index)) ? _maximum : getB(strip.get(index));
  return _maximum;
}

void setValue(int index, byte value) {
  strip.set(index, mHSVfast(0, 0, value));
}

void setPillarValue(uint16_t value) {
  int _value = value;
  for (byte i = 0; i < NUMLEDS - 1; i++) {
    if (_value > 255) {
      strip.leds[i] = mHSVfast(0, 0, 255);
      _value -= 255;
    } else {
      strip.leds[i] = mHSVfast(0, 0, _value);
      _value = 0;
    }
  }
}

void SwitchLampState() {
  if (isOn) {
    for (int i = BRIGHTNESS; i >= 0; i--) {
      strip.setBrightness(i);
      strip.show();
      delay(5);
    }
    isOn = false;
  } else {
    for (byte i = 0; i < BRIGHTNESS; i++) {
      strip.setBrightness(i);
      strip.show();
      delay(5);
    }
    isOn = true;
  }
}

void SaveSettings() {
  settings.brightness = BRIGHTNESS;
  settings.mode = mode;
  settings.subMode_mode5 = subMode_mode5;
  settings.subMode_mode6 = subMode_mode6;
  settings.isOn = isOn;
  EEPROM.put(1, settings);
}
