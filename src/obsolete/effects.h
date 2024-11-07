// those effects were removed as unused / unsupported unless someone find a time to invest in supporting it

// ----------- Эффект "Пятнашки"
// https://editor.soulmatelights.com/gallery/1471-puzzles-subpixel
// (c) Stepko 10.12.21
class EffectPuzzles : public EffectCalc {
private:
private:
    byte psizeX = 4;
    byte psizeY = 4;
    uint8_t pcols, prows;
    std::vector< std::vector<uint8_t> > puzzle;
    byte color;
    Vector2<int8_t> z_dot;
    byte step;
    Vector2<int8_t> move{0, 0};
    Vector2<float> shift{ 0, 0 };

    void draw_square(byte x1, byte y1, byte x2, byte y2, byte col);
    void draw_squareF(float x1, float y1, float x2, float y2, byte col);
    void regen();

    // effect instance mutex, 'cause controls are not thread-safe here
    std::mutex _mtx;
    String setDynCtrl(UIControl*_val) override;

public:
    EffectPuzzles(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){ speedFactor = 0.1; regen(); }
    void load() override;
    bool run() override;
};

// ---------- Эффект "Тикси Ленд"
// (c)  Martin Kleppe @aemkei, https://github.com/owenmcateer/tixy.land-display
class EffectTLand: public EffectCalc {
  private:
    bool isSeq = false;
    byte animation = 0;
    bool ishue;
    bool ishue2;
    byte hue = 0;
    byte hue2 = 128;
    byte shift = 0;
    byte fine = 1;
    float t;
    void processFrame(LedFB<CRGB> *fb, float t, float x, float y);
    float code(float i, float x, float y);
    String setDynCtrl(UIControl*_val);
  public:
    EffectTLand(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};

//------------ Эффект "Шторм" 
// (с) kostyamat 1.12.2020
class EffectWrain: public EffectCalc {
  private:

    struct Drop {
        float posX{0};
        float posY{0};
        uint8_t color{0};    // цвет капли
        float accell{0};     // персональное ускорение каждой капли
        uint8_t bri{0};      // яркость капли
    };

    const uint8_t cloudHeight = fb->h() / 5 + 1;
    float dotChaos;         // сила ветра
    int8_t dotDirect;       // направление ветра 
    bool clouds = false;
    bool storm = false;
    bool white = false;
    uint8_t _scale=1;
    byte type = 1;
    bool _flash;
    bool randColor = false;
    float windProgress = 0;
    uint32_t timer = 0;
    Vector2D<uint8_t> _noise {Vector2D<uint8_t>(fb->w(), cloudHeight)};
    std::vector<Drop> drops {std::vector<Drop>(fb->w() * 3)};

    void reload();
    String setDynCtrl(UIControl*_val) override;
    bool Lightning(uint16_t chanse);
    void Clouds(bool flash);

  public:
    EffectWrain(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
    void load() override;
};


// ============= ЭФФЕКТ ПРИЗМАТА ===============
// подбор размерности под ширину экрана нереализован и эффект на большей части превращается в кашу
// Prismata Loading Animation
class EffectPrismata : public EffectCalc {
private:
    byte spirohueoffset = 0;
    uint8_t fadelvl=1;
	    
    String setDynCtrl(UIControl*_val) override;
public:
    EffectPrismata(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ============= SWIRL /  ВОДОВОРОТ ===============
// требует алгоритма подбора масштаба
// https://gist.github.com/kriegsman/5adca44e14ad025e6d3b
// Copyright (c) 2014 Mark Kriegsman
class EffectSwirl : public EffectCalc {
private:
    bool swirlRoutine();

public:
    EffectSwirl(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ============= DRIFT / ДРИФТ ===============
// требует алгоритма подбора масштаба/скорости
// v1.0 - Updating for GuverLamp v1.7 by SottNick 12.04.2020
// v1.1 - +dither, +phase shifting by PalPalych 12.04.2020
// https://github.com/pixelmatix/aurora/blob/master/PatternIncrementalDrift.h
class EffectDrift : public EffectCalc {
private:
	byte maxDim_steps(){ return 256 / fb->maxDim(); }
	uint8_t dri_phase;
	float _dri_speed;
	uint8_t _dri_delta;
	byte driftType = 0;

	String setDynCtrl(UIControl*_val) override;
	bool incrementalDriftRoutine();
	bool incrementalDriftRoutine2();
    // some adjustments
    int width_adj(){ return (fb->w() < fb->h() ? (fb->h() - fb->w()) /2 : 0); };
    int height_adj(){ return (fb->h() < fb->w() ? (fb->w() - fb->h()) /2: 0); };

public:
    EffectDrift(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ----------- Эфеект "Попкорн"
// требует алгоритма подбора масштаба/скорости
// (C) Aaron Gotwalt (Soulmate)
// адаптация и доработки kostyamat
#define POPCORN_ROCKETS 10
class EffectPopcorn : public EffectCalc {
private:
    uint8_t numRockets = POPCORN_ROCKETS;
    bool blurred = false;
    bool revCol = false;
    //bool tiltDirec;
        float center = (float)fb->w() / 2.;

    struct Rocket {
        float x, y, xd, yd;
        byte hue;
    };

    std::vector<Rocket> rockets{std::vector<Rocket>(POPCORN_ROCKETS, Rocket())};

    void restart_rocket(uint8_t r);
    void reload();

    String setDynCtrl(UIControl*_val) override;
    //void setscl(const byte _scl) override; // перегрузка для масштаба

public:
    EffectPopcorn(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ============= RADAR / РАДАР ===============
// сломан на больших панелях
// Aurora : https://github.com/pixelmatix/aurora/blob/master/PatternRadar.h
// Copyright(c) 2014 Jason Coon
class EffectRadar : public EffectCalc {
private:
    float eff_offset; 
    float eff_theta;  // глобальная переменная угла для работы эффектов
    bool subPix = false;
    byte hue;
    const float width_adj_f = (float)(fb->w() < fb->h() ? (fb->h() - fb->w()) / 2. : 0);
    const float height_adj_f= (float)(fb->h() < fb->w() ? (fb->w() - fb->h()) / 2. : 0);
    int width_adj(){ return (fb->w() < fb->h() ? (fb->h() - fb->w()) /2 : 0); };
    int height_adj(){ return (fb->h() < fb->w() ? (fb->w() - fb->h()) /2: 0); };

    bool radarRoutine();
    String setDynCtrl(UIControl *_val) override;

public:
    EffectRadar(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ----------- Эффект "Бенгальские Огни"
// требует алгоритма подбора масштаба/скорости
// (c) stepko https://wokwi.com/arduino/projects/289797125785520649
// 06.02.2021
class EffectBengalL : public EffectCalc {
    struct Spark{
        float posx, posy;
        float speedx, speedy;
        float sat;
        float fade;
        byte color;
    };

    const uint8_t minSparks = 4;
    const uint8_t maxSparks = fb->w() * 4;
    uint8_t gPosx, gPosy;

    bool centerRun = true;
    byte period = 10;
    byte _x = fb->w()/2;
    byte _y = fb->h()/2;
    std::vector<Spark> sparks{ std::vector<Spark>(minSparks) };

    void regen(Spark &s);
    void physics(Spark &s);
    String setDynCtrl(UIControl*_val) override;


public:
    EffectBengalL(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};



// требует алгоритма подбора масштаба/скорости, переворачивания оси
class EffectFire2018 : public EffectCalc {
#define FIRE_NUM_LAYERS     2
private:
  bool isLinSpeed = true;
  uint8_t fade_amount = 15;      // this could be a new control for flame size/height
  // use vector of vectors to take benefit of swap operations
  std::vector<std::vector<uint8_t>> fire18heat;
  Noise3dMap noise;

  String setDynCtrl(UIControl*_val) override;

public:
    EffectFire2018(LedFB<CRGB> *framebuffer) : 
        EffectCalc(framebuffer),
        fire18heat(std::vector<std::vector<uint8_t>>(framebuffer->h(), std::vector<uint8_t>(framebuffer->w()))),
        noise(FIRE_NUM_LAYERS, framebuffer->w(), framebuffer->h()) { fb->clear(); }
    bool run() override;
};


// -------------- Эффект "Кодовый замок"
// требует алгоритма подбора масштаба/скорости, выглядит бестолково на плоской панели
// (c) SottNick
//uint8_t ringColor[fb->h()]; // начальный оттенок каждого кольца (оттенка из палитры) 0-255
//uint8_t huePos[fb->h()]; // местоположение начального оттенка кольца 0-fb->maxWidthIndex()
//uint8_t shiftHueDir[fb->h()]; // 4 бита на ringHueShift, 4 на ringHueShift2
////ringHueShift[ringsCount]; // шаг градиета оттенка внутри кольца -8 - +8 случайное число
////ringHueShift2[ringsCount]; // обычная скорость переливания оттенка всего кольца -8 - +8 случайное число
//uint8_t currentRing; // кольцо, которое в настоящий момент нужно провернуть
//uint8_t stepCount; // оставшееся количество шагов, на которое нужно провернуть активное кольцо - случайное от fb->w()/5 до fb->w()-3
class EffectRingsLock : public EffectCalc {
private:
    struct LockRing {
        uint8_t color;      // начальный оттенок каждого кольца (оттенка из палитры) 0-255
        uint8_t huePos;     // местоположение начального оттенка кольца 0-w-1
        uint8_t shiftHueDir;    // 4 бита на ringHueShift, 4 на ringHueShift2
    };

    uint8_t ringWidth;      // максимальне количество пикселей в кольце (толщина кольца) от 1 до height / 2 + 1
    //uint8_t ringNb; // количество колец от 2 до height
    uint8_t lowerRingWidth, upperRingWidth; // количество пикселей в нижнем (lowerRingWidth) и верхнем (upperRingWidth) кольцах
    uint8_t currentRing; // кольцо, которое в настоящий момент нужно провернуть
    uint8_t stepCount; // оставшееся количество шагов, на которое нужно провернуть активное кольцо - случайное от w/5 до w-3

    // набор колец
    std::vector<LockRing> rings{std::vector<LockRing>(2, LockRing())};
  //std::vector<uint8_t> ringColor{std::vector<uint8_t>(fb->h())};    // начальный оттенок каждого кольца (оттенка из палитры) 0-255
  //std::vector<uint8_t> huePos{std::vector<uint8_t>(fb->h())};       // местоположение начального оттенка кольца 0-w-1
  //std::vector<uint8_t> shiftHueDir{std::vector<uint8_t>(fb->h())};  

  ////ringHueShift[ringsCount]; // шаг градиета оттенка внутри кольца -8 - +8 случайное число
  ////ringHueShift2[ringsCount]; // обычная скорость переливания оттенка всего кольца -8 - +8 случайное число



  void ringsSet();
  bool ringsRoutine();
  String setDynCtrl(UIControl*_val) override;

public:
    EffectRingsLock(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ------ Эффект "Прыгуны" (c) obliterator
// Хороший эффект, требует алгоритма подбора масштаба/скорости, переворачивания оси
#define LEAPERS_MIN     5
#define LEAPERS_MAX     20
class EffectLeapers : public EffectCalc {
    struct Leaper {
        float x{0}, y{0};
        float xd{0}, yd{0};
        byte color;
    };
private:
    unsigned numParticles = 0;
    uint8_t _rv{0};
    std::vector<Leaper> leapers{std::vector<Leaper>(LEAPERS_MIN, Leaper())};

    void generate();
    void restart_leaper(Leaper &l);
    void move_leaper(Leaper &l);
	String setDynCtrl(UIControl*_val) override;
public:
    EffectLeapers(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
	void load() override;
    bool run() override;

};


// ------------- Эффект "Блики на воде Цвета"
// на панели выглядит бестолково, работает медленно, палитры и цвета не сочетаются
// Идея SottNick
// переписал на программные блики + паттерны - (c) kostyamat
// Генератор бликов (c) stepko
class EffectAquarium : public EffectCalc {
private:

struct Drop{
    uint8_t posX, posY;
    float radius;
};

    CRGBPalette16 currentPalette;
    const uint8_t _scale = 25;
    const uint8_t _speed = 3;

    float hue = 0.;
    uint16_t x{0}, y{0}, z{0};
    Vector2D<uint8_t> noise;

    inline uint8_t maxRadius(){return fb->w() + fb->h();};
    std::vector<Drop> drops;
    uint8_t satur;
    uint8_t glare = 0;
    uint8_t iconIdx = 0;

    void nGlare(uint8_t bri);
    void nDrops(uint8_t bri);
    void fillNoiseLED();

public:
    EffectAquarium(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer),
        noise(framebuffer->w(), framebuffer->h()),
        drops(std::vector<Drop>((fb->h() + fb->w()) / 6)) {}

    void load() override;
    String setDynCtrl(UIControl*_val) override;
    bool run() override;
};



//---------- Эффект "Фейерверк"
// требует алгоритма подбора масштаба/скорости, переворачивания оси
// адаптация и переписал - kostyamat
// https://gist.github.com/jasoncoon/0cccc5ba7ab108c0a373
// vortigont переписал за kostyamat'ом

#define MIN_RCKTS   2U
#define MAX_RCKTS   8U     // максимальное количество снарядов
#define NUM_SPARKS  16U    // количество разлитающихся петард (частей снаряда)

struct DotsStore {
    accum88 gBurstx;
    accum88 gBursty;
    saccum78 gBurstxv;
    saccum78 gBurstyv;
    CRGB gBurstcolor;
    bool gSkyburst = false;
};

struct Dot {    // класс для создания снарядов и петард

  static constexpr saccum78 gGravity = 10;
  static constexpr fract8  gBounce = 127;
  static constexpr fract8  gDrag = 255;

  byte    show{0};
  byte    theType{0};
  accum88 x{0}, y{0};
  saccum78 xv{0}, yv{0};
  accum88 r{0};
  CRGB color{CRGB::Black};
  uint16_t cntdown{0};

  //Dot()
  void Move(DotsStore &store, bool flashing);
  void GroundLaunch(DotsStore &store, uint16_t h);
  void Skyburst( accum88 basex, accum88 basey, saccum78 basedv, CRGB& basecolor, uint8_t dim);
  int16_t scale15by8_local( int16_t i, fract8 _scale ){ return (i * _scale / 256); };
};

class EffectFireworks : public EffectCalc {

    DotsStore store;
    byte dim;
    uint8_t valDim;
    bool flashing = false;
    bool fireworksRoutine();
    void sparkGen();
    std::vector<Dot> gDot;
    std::vector<Dot> gSparks;
    String setDynCtrl(UIControl*_val) override;
    void draw(Dot &d);
    int16_t _model_w(){ return 2*(fb->w() - 4) + fb->w(); };  // как далеко за экран может вылетить снаряд, если снаряд вылетает за экран, то всышка белого света (не особо логично)
    int16_t _model_h(){ return 2*(fb->h() - 4) + fb->h(); };
    int16_t _x_offset(){ return (_model_w()-fb->w())/2; };
    int16_t _y_offset(){ return (_model_h()-fb->h())/2; };
    void _screenscale(accum88 a, uint16_t N, uint16_t &screen, uint16_t &screenerr);

public:
    EffectFireworks(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer), gDot(std::vector<Dot>(MIN_RCKTS)), gSparks(std::vector<Dot>(NUM_SPARKS)) {}
    //void load() override;
    bool run() override;
};




// ------------ Эффект "Тихий Океан"
// на панели выглядит бестолково, работает медленно, палитры и цвета не сочетаются
//  Gentle, blue-green ocean waves.
//  December 2019, Mark Kriegsman and Mary Corey March.
//  For Dan.
// https://raw.githubusercontent.com/FastLED/FastLED/master/examples/Pacifica/Pacifica.ino
class EffectPacific : public EffectCalc {
private:

    uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;    // "color index start" counters
    uint32_t sLastms = 0;

	void pacifica_one_layer(const TProgmemRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff);
	void pacifica_deepen_colors();
	void pacifica_add_whitecaps();
	String setDynCtrl(UIControl*_val) override;

public:
    EffectPacific(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){ speedFactor = 100; }
    //void load() override;
    bool run() override;
};



// ------ Эффект "Вышиванка" 
// сломан на больших панелях
// (с) проект Aurora "Munch"
class EffectMunch : public EffectCalc {
private:
    byte count = 0;
    int8_t dir = 1;
    byte flip = 0;
    byte generation = 0;
    byte mic[2];
    byte rand;
    bool flag = false;
    uint8_t minDimLocal = fb->maxDim() > 32 ? 32 : 16;

    String setDynCtrl(UIControl*_val) override;
    bool munchRoutine();

public:
    EffectMunch(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ------ Эффект "Цветной шум" 
// полностью рабочий эффект, но своим адским мельтешением вызывает приступы эпилепсии
// (с) https://gist.github.com/StefanPetrick/c856b6d681ec3122e5551403aabfcc68
class EffectNoise : public EffectCalc {
private:

    const uint8_t centreX = (fb->w() / 2) - 1;
    const uint8_t centreY = (fb->h() / 2) - 1;
    Noise3dMap  noise{Noise3dMap(1, fb->w(), fb->h())};
    bool type = false;

	String setDynCtrl(UIControl*_val) override;

public:
    EffectNoise(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ---- Эффект "Мотыльки"
// черные точки на сплошной заливке - ерунда какая-то
// (с) SottNick, https://community.alexgyver.ru/threads/wifi-lampa-budilnik-obsuzhdenie-proekta.1411/post-49262
class EffectButterfly : public EffectCalc {
private:
    float butterflysPosX[BUTTERFLY_MAX_COUNT];
    float butterflysPosY[BUTTERFLY_MAX_COUNT];
    float butterflysSpeedX[BUTTERFLY_MAX_COUNT];
    float butterflysSpeedY[BUTTERFLY_MAX_COUNT];
    float butterflysTurn[BUTTERFLY_MAX_COUNT];
    uint8_t butterflysColor[BUTTERFLY_MAX_COUNT];
    uint8_t butterflysBrightness[BUTTERFLY_MAX_COUNT];
    uint8_t deltaValue;
    uint8_t deltaHue;
    uint8_t hue;
    uint8_t hue2;
    byte step = 0;
    byte csum = 0;
    uint8_t cnt;
    bool wings = false;
    bool isColored = true;
		String setDynCtrl(UIControl*_val) override;

public:
    EffectButterfly(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ---- Эффект "Тени"
// радуга, которая очень плохо масштабируется
// требует алгоритма подбора масштаба/скорости, переворачивания оси
// https://github.com/vvip-68/GyverPanelWiFi/blob/master/firmware/GyverPanelWiFi_v1.02/effects.ino
class EffectShadows : public EffectCalc {
private:
    uint16_t sPseudotime = 0;
    uint16_t sLastMillis = 0;
    uint16_t sHue16 = 0;
    bool linear = true;
    String setDynCtrl(UIControl*_val) override;

public:
    EffectShadows(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};


// ---- Эффект "Узоры"
// заливка патернами, большинство бестолковые, цвета/фон сочетаются плохо, смотреть не на что
// (c) kostyamat (Kostyantyn Matviyevskyy) 2020
// переделано kDn
// идея отсюда https://github.com/vvip-68/GyverPanelWiFi/
#define PATTERNS_BUFFSIZE   10
class EffectPatterns : public EffectCalc {
private:
    int8_t patternIdx = -1;
    int8_t lineIdx = 0;
    bool _subpixel = false;
    bool _sinMove = false;
    int8_t _speed = 1, _scale = 1;
    bool dir = false;
    byte csum = 0;
    byte _bri = 255U;
    std::vector< std::vector<uint8_t> > buff{ std::vector< std::vector<uint8_t> >(PATTERNS_BUFFSIZE, std::vector<uint8_t>(PATTERNS_BUFFSIZE)) };
    //LedFB<CRGB> buff(PATTERNS_BUFFSIZE, PATTERNS_BUFFSIZE);
    float xsin, ysin;
    unsigned long lastrun2;
    byte _sc = 0;
    float _speedX, _speedY;

    CHSV colorMR[12] = {
        CHSV(0, 0, 0),              // 0 - Black
        CHSV(HUE_RED, 255, 255),    // 1 - Red
        CHSV(HUE_GREEN , 255, 255),  // 2 - Green
        CHSV(HUE_BLUE, 255, 255),   // 3 - Blue
        CHSV(HUE_YELLOW, 255, 255), // 4 - Yellow
        CHSV(0, 0, 220),            // 5 - White
        CHSV(0, 255, 255),              // 6 - плавно меняеться в цикле (фон)
        CHSV(0, 255, 255),              // 7 - цвет равен 6 но +64
        CHSV(HUE_ORANGE, 255, 255),
        CHSV(HUE_PINK, 255, 255),
        CHSV(HUE_PURPLE, 255, 255),
        CHSV(HUE_AQUA, 255, 255),
    };

    String setDynCtrl(UIControl*_val) override;
    void drawPicture_XY();
    bool patternsRoutine();

public:
    EffectPatterns(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ***************************** "Стрелки" *****************************
// полностью рабочий но скучный эффект с бегающими стрелками
// взято отсюда https://github.com/vvip-68/GyverPanelWiFi/
class EffectArrows : public EffectCalc {
private:
    float arrow_x[4], arrow_y[4], stop_x[4], stop_y[4];
    byte arrow_direction;             // 0x01 - слева направо; 0x02 - снизу вверх; 0х04 - справа налево; 0х08 - сверху вниз
    byte arrow_mode, arrow_mode_orig; // 0 - по очереди все варианты
                                      // 1 - по очереди от края до края экрана;
                                      // 2 - одновременно по горизонтали навстречу к ентру, затем одновременно по вертикали навстречу к центру
                                      // 3 - одновременно все к центру
                                      // 4 - по два (горизонталь / вертикаль) все от своего края к противоположному, стрелки смещены от центра на 1/3
                                      // 5 - одновременно все от своего края к противоположному, стрелки смещены от центра на 1/3
    bool arrow_complete, arrow_change_mode;
    byte arrow_hue[4];
    byte arrow_play_mode_count[6];      // Сколько раз проигрывать полностью каждый режим если вариант 0 - текущий счетчик
    byte arrow_play_mode_count_orig[6]; // Сколько раз проигрывать полностью каждый режим если вариант 0 - исходные настройки
    uint8_t _scale;
        void arrowSetupForMode(byte mode, bool change);
    void arrowSetup_mode1();
    void arrowSetup_mode2();
    //void arrowSetup_mode3(;)
    void arrowSetup_mode4();

    String setDynCtrl(UIControl*_val) override;
public:
    EffectArrows(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ------ Эффект "Дикие шарики"
// требует алгоритма подбора масштаба/скорости, выглядит бестолково на плоской панели
// (с) https://gist.github.com/bonjurroughs/9c107fa5f428fb01d484#file-noise-balls
class EffectNBals : public EffectCalc {
private:
    uint8_t lastSecond = 99;
    uint16_t speedy;// speed is set dynamically once we've started up
    uint16_t _scale;
    byte beat1, beat2 = 0;
    byte balls = 4;
    void balls_timer();
    void blur();
    bool nballsRoutine();

    String setDynCtrl(UIControl*_val) override;

public:
    EffectNBals(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};



// --------- Эффект "Северное Сияние"
// требует подбора палитр и пр, выглядит бестолково
// (c) kostyamat 05.02.2021
// идеи подсмотрены тут https://www.reddit.com/r/FastLED/comments/jyly1e/challenge_fastled_sketch_that_fits_entirely_in_a/

class EffectPolarL : public EffectCalc {
private:
    const byte numpalettes = 14;
    unsigned long timer = 0;
    float adjastHeight = 1;
    uint16_t adjScale = 1;
    byte pal = 0;
    uint16_t _scale = 30;
    byte flag = 0;
	byte _speed = 16;

    //void setscl(const byte _scl) override;
    String setDynCtrl(UIControl*_val) override;
    void palettemap(std::vector<PGMPalette*> &_pals, const uint8_t _val, const uint8_t _min, const uint8_t _max) override;
    void palettesload() override;

public:
    EffectPolarL(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};



/* Эффект "Цветение" */
// полностью рабочий но скучный эффект, выводит набор кругов
class EffectFlower : public EffectCalc {
	private:
        uint8_t effTimer;
        float ZVoffset = 0;
        const float COLS_HALF = fb->w() * .5;
        const float ROWS_HALF = fb->h() * .5;
        int16_t ZVcalcDist(uint8_t x, uint8_t y, float center_x, float center_y);
	public:
    EffectFlower(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
        bool run() override;
};


// ----------- Эфеект "Змеиный Остров"
// (c) Сотнег
// радужные змейки, сломан
// адаптация и доработки kostyamat
#define MAX_SNAKES    (16U)
class EffectSnakeIsland : public EffectCalc {
    const uint8_t snake_len = fb->h()/2;
    struct Snake {
        long  last;            // тут будет траектория тела червяка
        float posX, posY;      // тут будет позиция головы
        float speedX, speedY;  // тут будет скорость червяка
        uint8_t color;         // тут будет начальный цвет червяка
        uint8_t direct;        //тут будет направление червяка
    };

    std::vector<Snake> snakes{std::vector<Snake>(1)};
	
    String setDynCtrl(UIControl*_val) override;
    void regen();

public:
    EffectSnakeIsland(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};



// --------- Эффект "Космо-Гонщик"
// требует алгоритма подбора масштаба/скорости
// (c) Stepko + kostyamat https://editor.soulmatelights.com/my-patterns/655
class EffectRacer: public EffectCalc {
private:
    float posX = random(0, fb->w()-1);
    float posY = random(0, fb->h()-1);
    uint8_t aimX = random(0, fb->w())-1;
    uint8_t aimY = random(0, fb->h()-1);
    float radius = 0;
    byte hue = millis()>>1; //random(0, 255);
    CRGB color;
        float addRadius;
    float angle;
    byte starPoints = random(3, 7);

    const float _speed = (float)fb->size() / 256; // Нормализация скорости для разных размеров матриц
    const float _addRadius = (float)fb->size() / 4000;   // Нормализация скорости увеличения радиуса круга для разных матриц


    void aimChange();
    void drawStarF(float x, float y, float biggy, float little, int16_t points, float dangle, CRGB color);
    //void setspd(const byte _spd) override;
    String setDynCtrl(UIControl*_val) override;

public:
    EffectRacer(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ----------- Эффект "Мираж"
// требует подбора палитр и цветов фона, рабочий
// based on cod by @Stepko (c) 23/12/2021
class EffectMirage : public EffectCalc {
private:
    const float div = 10.;
    const uint16_t width = (fb->w() - 1) * div, height = fb->h() * div;
    uint16_t _speed;
    byte color;
    bool colorShift = false;
    Noise3dMap buff{Noise3dMap(1, fb->w()+2, fb->h()+2)};
    bool a = true;
	
    String setDynCtrl(UIControl*_val) override;
    void drawDot(float x, float y, byte a);
    void blur();

public:
    EffectMirage(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};



/*
    Effect "Tetris clock"
    требует либу адафрутгфх
    based on https://github.com/witnessmenow/WiFi-Tetris-Clock
*/
class TetrisClock : public EffectCalc {
    LedFB_GFX   screen;
    TetrisMatrixDraw t_clk;     // Main clock
    TetrisMatrixDraw t_m;       // The "M" of AM/PM
    TetrisMatrixDraw t_ap;      // The "P" or "A" of AM/PM

    Task *seconds;               // seconds pulse
    Task *animatic;              // animation task

    bool animation_idle;        // animation in progress
    bool hour24{1};             // 12/24 hour mode
    bool showColon{0};          // hh:mm sepparator
    bool forceRefresh{true};    // redraw all numbers
    bool redraw{0};             // flag that triggers screen refresh

    uint8_t lastDisplayedAmPm;
    int lastmin{-1};


    void _clock_animation();
    void _gettime();
    void _handleColonAfterAnimation();
    String setDynCtrl(UIControl*_val) override;

	public:
    TetrisClock(std::shared_ptr< LedFB<CRGB> > framebuffer);
    ~TetrisClock();
    void load() override; 
    bool run() override;
};


/* -------------- эффект "VU-meter, частотный анализатор"
    (c) G6EJD, https://www.youtube.com/watch?v=OStljy_sUVg&t=0s
    reworked by s-marley https://github.com/s-marley/ESP32_FFT_VU
    adopted for FireLamp_jeeUI by kostyamat, kDn
    reworked and updated (c) kostyamat 24.04.2021
*/
class EffectVU: public EffectCalc {
private:
    CRGBPalette16 gradPal[5] = {
        purple_gp,    rainbowsherbet_gp, 
        redyellow_gp, Colorfull_gp, es_ocean_breeze_068_gp
    };
    size_t bands = fb->w();
    size_t bar_width{1};
    uint8_t calcArray = 1;          // уменьшение частоты пересчета массива
    uint8_t colorTimer = 0;
    const uint8_t colorDev = 256/fb->maxHeightIndex();
    // Sampling and FFT stuff
    std::vector<float> peak{std::vector<float>(fb->w())};              // The length of these arrays must be >= bands
    std::vector<float> oldBarHeights{std::vector<float>(fb->w())};
    std::vector<float> bandValues{std::vector<float>(fb->w())};
    MicWorker *mw = nullptr;

    int _mic_gpio;
    float samp_freq;
    float last_freq = 0;
    uint8_t last_min_peak, last_max_peak;
    float maxVal;
    float threshold;
    byte tickCounter;
    byte colorType;

    float amplitude = 1.0;
    int effId = 0;
    bool type = false;
    bool colorShifting = false;
    const float speedFactorVertical = (float)fb->h() / 16;
    bool averaging = true;

    String setDynCtrl(UIControl*_val) override;
    void horizontalColoredBars(uint8_t band, float barHeight, uint8_t type = 0, uint8_t colorShift = 0);
    void paletteBars(uint8_t band, float barHeight, CRGBPalette16& palette, uint8_t colorShift = 0);
    void verticalColoredBars(uint8_t band, float barHeight, uint8_t type = 0, uint8_t colorShift = 0);
    void centerBars(uint8_t band, float barHeight, CRGBPalette16& palette, uint8_t colorShift = 0);
    void whitePeak(uint8_t band);
    void outrunPeak(uint8_t band, CRGBPalette16& palette, uint8_t colorShift = 0);
    void waterfall(uint8_t band, uint8_t barHeight);

public:
    EffectVU(LedFB<CRGB> *framebuffer, int gpio) : EffectCalc(framebuffer), _mic_gpio(gpio) {}
    ~EffectVU(){ delete mw; }
    bool run() override;
    void load() override;
};





//----- Эффект "Осциллограф" 
// (c) kostyamat
class EffectOsc : public EffectCalc {
private:
    int _mic_gpio;
    int pointer = 2048;
    uint32_t oscHV{16}, oscilLimit{16};

    CRGB color;
    int div{1}, gain{16};
    int y[2] = {0, 0};
    String setDynCtrl(UIControl*_val) override;

public:
    EffectOsc(LedFB<CRGB> *framebuffer, int gpio = GPIO_NUM_NC) : EffectCalc(framebuffer), _mic_gpio(gpio) {}
    bool run() override;
    void setMicGPIO(int gpio){ _mic_gpio = gpio; };
};



// ========== Эффект "Эффектопад"
// совместное творчество юзеров форума https://community.alexgyver.ru/
class EffectEverythingFall : public EffectCalc {
private:
    Vector2D<uint8_t> heat{ Vector2D<uint8_t>(fb->w(),fb->h()) };

public:
    EffectEverythingFall(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ----------- Эффект "Огненная лампа"
// https://editor.soulmatelights.com/gallery/546-fire
// (c) Stepko 17.06.21
// sparks (c) kostyamat 10.01.2022 https://editor.soulmatelights.com/gallery/1619-fire-with-sparks
// cpp refactoring (c) Vortigont
// непонятная бурда, на панели выглядит бестолково
class EffectFire2021 : public EffectCalc {
private:
    byte _pal{8};
    uint8_t _fill{10};
    uint16_t t{0};
    size_t _sparks_cnt{0};

    const uint8_t spacer = fb->h()/4;

    class Spark {
    private:
        CRGB color;
        uint8_t Bri;
        uint8_t Hue;
        float x, y, speedy = 1;
    
    public:
        void addXY(float nx, float ny, LedFB<CRGB> *fb);

        float getY() { return y; }

        void reset(LedFB<CRGB> *fb);

        void draw(LedFB<CRGB> *fb);
    }; 

    std::vector<Spark> sparks;  //{std::vector<Spark>(fb->w() / 4, Spark())};

    //void palettesload() override;

public:
    EffectFire2021(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){ scale = 32; }
    void load() override;
    void setControl(size_t idx, int32_t value) override;
    bool run() override;
};

