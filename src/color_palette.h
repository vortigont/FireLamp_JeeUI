#ifndef __COLOR_PALETTE_H
#define __COLOR_PALETTE_H
// переместил в platformio.ini
// #ifdef ESP8266
// #define FASTLED_USE_PROGMEM             (1)
// #endif

#include "FastLED.h"
#include "colorutils.h"
#include "LList.h"

/**
 * Набор палитр в дополнение к тем что идут с FastLED
 * новые палитры добавляем в алфавитном порядке
 */
#define FASTLED_PALETTS_COUNT 22
static const TProgmemRGBPalette16 HeatColors2_p FL_PROGMEM = {    0x000000, 0x330000, 0x660000, 0x990000, 0xCC0000, 0xFF0000, 0xFF3300, 0xFF6600, 0xFF9900, 0xFFCC00, 0xFFFF00, 0xFFFF33, 0xFFFF66, 0xFFFF99, 0xFFFFCC, 0xFFFFFF};
static const TProgmemRGBPalette16 WoodFireColors_p FL_PROGMEM = {CRGB::Black, 0x330e00, 0x661c00, 0x992900, 0xcc3700, CRGB::OrangeRed, 0xff5800, 0xff6b00, 0xff7f00, 0xff9200, CRGB::Orange, 0xffaf00, 0xffb900, 0xffc300, 0xffcd00, CRGB::Gold};             //* Orange
static const TProgmemRGBPalette16 NormalFire3_p FL_PROGMEM = {CRGB::Black, 0x330000, 0x660000, 0x990000, 0xcc0000, CRGB::Red, 0xff0c00, 0xff1800, 0xff2400, 0xff3010, 0xff3c20, 0xff4835, 0xff5440, 0xff6055, 0xff6c60, 0xff7866};                             // пытаюсь сделать что-то более приличное
static const TProgmemRGBPalette16 NormalFire_p FL_PROGMEM = {CRGB::Black, 0x330000, 0x660000, 0x990000, 0xcc0000, CRGB::Red, 0xff0c00, 0xff1800, 0xff2400, 0xff3000, 0xff3c00, 0xff4800, 0xff5400, 0xff6000, 0xff6c00, 0xff7800};                             // пытаюсь сделать что-то более приличное
static const TProgmemRGBPalette16 MagmaColor_p FL_PROGMEM = {CRGB::Black, 0x240000, 0x480000, 0x660000, 0x9a1100, 0xc32500, 0xd12a00, 0xe12f17, 0xf0350f, 0xff3c00, 0xff6400, 0xff8300, 0xffa000, 0xffba00, 0xffd400, 0xffffff};
static const TProgmemRGBPalette16 NormalFire2_p FL_PROGMEM = {CRGB::Black, 0x560000, 0x6b0000, 0x820000, 0x9a0011, CRGB::FireBrick, 0xc22520, 0xd12a1c, 0xe12f17, 0xf0350f, 0xff3c00, 0xff6400, 0xff8300, 0xffa000, 0xffba00, 0xffd400};                      // пытаюсь сделать что-то более приличное
static const TProgmemRGBPalette16 LithiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x240707, 0x470e0e, 0x6b1414, 0x8e1b1b, CRGB::FireBrick, 0xc14244, 0xd16166, 0xe08187, 0xf0a0a9, CRGB::Pink, 0xff9ec0, 0xff7bb5, 0xff59a9, 0xff369e, CRGB::DeepPink};        //* Red
static const TProgmemRGBPalette16 SodiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x332100, 0x664200, 0x996300, 0xcc8400, CRGB::Orange, 0xffaf00, 0xffb900, 0xffc300, 0xffcd00, CRGB::Gold, 0xf8cd06, 0xf0c30d, 0xe9b913, 0xe1af1a, CRGB::Goldenrod};           //* Yellow
static const TProgmemRGBPalette16 CopperFireColors_p FL_PROGMEM = {CRGB::Black, 0x001a00, 0x003300, 0x004d00, 0x006600, CRGB::Green, 0x239909, 0x45b313, 0x68cc1c, 0x8ae626, CRGB::GreenYellow, 0x94f530, 0x7ceb30, 0x63e131, 0x4bd731, CRGB::LimeGreen};     //* Green
static const TProgmemRGBPalette16 AlcoholFireColors_p FL_PROGMEM = {CRGB::Black, 0x000033, 0x000066, 0x000099, 0x0000cc, CRGB::Blue, 0x0026ff, 0x004cff, 0x0073ff, 0x0099ff, CRGB::DeepSkyBlue, 0x1bc2fe, 0x36c5fd, 0x51c8fc, 0x6ccbfb, CRGB::LightSkyBlue};  //* Blue
static const TProgmemRGBPalette16 RubidiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x0f001a, 0x1e0034, 0x2d004e, 0x3c0068, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, 0x3c0084, 0x2d0086, 0x1e0087, 0x0f0089, CRGB::DarkBlue};        //* Indigo
static const TProgmemRGBPalette16 PotassiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x0f001a, 0x1e0034, 0x2d004e, 0x3c0068, CRGB::Indigo, 0x591694, 0x682da6, 0x7643b7, 0x855ac9, CRGB::MediumPurple, 0xa95ecd, 0xbe4bbe, 0xd439b0, 0xe926a1, CRGB::DeepPink}; //* Violet
static const TProgmemRGBPalette16 WaterfallColors_p FL_PROGMEM = {0x000000, 0x060707, 0x101110, 0x151717, 0x1C1D22, 0x242A28, 0x363B3A, 0x313634, 0x505552, 0x6B6C70, 0x98A4A1, 0xC1C2C1, 0xCACECF, 0xCDDEDD, 0xDEDFE0, 0xB2BAB9};        //* Orange
static const TProgmemRGBPalette16 ZeebraColors_p FL_PROGMEM = {CRGB::White, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::White, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::White, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::White, CRGB::Black, CRGB::Black, CRGB::Black};
// Добавил "белую" палитру для "Огонь 2012", в самом конце 4-го ползунка, огонь горит белым цветом, - красиво для белой динамической подсветки
static const TProgmemRGBPalette16 WhiteBlackColors_p FL_PROGMEM = {CRGB::Black, CRGB::Gray, CRGB::White, CRGB::WhiteSmoke};
// stepko нашел и перерисовал палитры в https://colorswall.com/
static const TProgmemRGBPalette16 AcidColors_p FL_PROGMEM = {0xffff00, 0xd2b328, 0xd25228, 0x711323, 0x6c000c, 0x5a0c00, 0x6d373a, 0xaa5a62, 0x604564, 0x313164, 0x332765, 0x3a2465, 0x4b1665, 0x4b0069, 0x31004c, 0x200046};
static const TProgmemRGBPalette16 StepkosColors_p FL_PROGMEM = {0x0000ff, 0x0f00f0, 0x1e00e1, 0x2d00d2, 0x3c00c3, 0x4b00b4, 0x5a00a5, 0x690096, 0x780087, 0x870078, 0x9600cd, 0xa50050, 0xb40041, 0xc30032, 0xd20023, 0xe10014};
static const TProgmemRGBPalette16 AutumnColors_p FL_PROGMEM = {0xbc2300, 0xc84416, 0xdc642c, 0xe69664, 0xfbb979, 0xca503d, 0x882c1c, 0x9a3631, 0xa9624e, 0xcc9762, 0xdcc0b5, 0xc1a29f, 0x826468, 0x4a3334, 0x231a1a, 0x161113};
static const TProgmemRGBPalette16 NeonColors_p FL_PROGMEM = {0x00b1d0, 0x0f93ec, 0x3572ff, 0x4157ff, 0x6162ff, 0x686cff, 0x7473ff, 0x8689e5, 0x9e9dc6, 0x9694ac, 0x979b9b, 0x888b8c, 0x767680, 0x596160, 0x6c736f, 0x7b7359};
static const TProgmemRGBPalette16 EveningColors_p FL_PROGMEM = {0x1e0443, 0x6d0081, 0x8200ac, 0x8200ac, 0x8200ac, 0x8200ac, 0x8200ac, 0x8200ac, 0x7900a1, 0x820055, 0xc80000, 0xe57b00, 0xff9d5a, 0xc58b32, 0xd8d400, 0xffff00};
static const TProgmemRGBPalette16 OrangeColors_p FL_PROGMEM = {0xffff00, 0xfff100, 0xffe100, 0xffd100, 0xffc100, 0xffb100, 0xffa100, 0xff9100, 0xff8100, 0xff7100, 0xff6100, 0xff5100, 0xff4100, 0xff3100, 0xff2100, 0xff1100};
// Удачная палитра от Stepko заменю ка я ею RainbowStripeColors_p, которая "рябит" на большинстве эффектов
static const TProgmemRGBPalette16 AuroraColors_p FL_PROGMEM = {0x00ff00, 0x00c040, 0x008080, 0x0040c0, 0x0000ff, 0x4000c0, 0x800080, 0xc00040, 0xff0000, 0xff4000, 0xff8000, 0xd6c000, 0xffff00, 0xc0ff00, 0x80ff00, 0x40ff00};
static const TProgmemRGBPalette16 HolyLightsColors_p FL_PROGMEM = {0xff0000, 0xff4000, 0xff8000, 0xd6c000, 0xffff00, 0xc0ff00, 0x80ff00, 0x40ff00, 0x00ff00, 0x00c040, 0x008080, 0x0040c0, 0x0000ff, 0x4000c0, 0x800080, 0xc00040};

// ------------ Эффект "Тихий Океан"
//  "Pacifica" перенос кода kostyamat
//  Gentle, blue-green ocean waves.
//  December 2019, Mark Kriegsman and Mary Corey March.
//  For Dan.
// https://raw.githubusercontent.com/FastLED/FastLED/master/examples/Pacifica/Pacifica.ino
static const TProgmemRGBPalette16 pacifica_palette_1 FL_PROGMEM =
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
static const TProgmemRGBPalette16 pacifica_palette_2 FL_PROGMEM =
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
static const TProgmemRGBPalette16 pacifica_palette_3 FL_PROGMEM =
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33,
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };


DECLARE_GRADIENT_PALETTE(MBVioletColors_gp);

//////////////////////////////

DECLARE_GRADIENT_PALETTE(ib_jul01_gp);
DECLARE_GRADIENT_PALETTE(es_vintage_57_gp);
DECLARE_GRADIENT_PALETTE(es_vintage_01_gp);
DECLARE_GRADIENT_PALETTE(es_rivendell_15_gp);
DECLARE_GRADIENT_PALETTE(rgi_15_gp);
DECLARE_GRADIENT_PALETTE(retro2_16_gp);
DECLARE_GRADIENT_PALETTE(Analogous_1_gp);
DECLARE_GRADIENT_PALETTE(es_pinksplash_08_gp);
DECLARE_GRADIENT_PALETTE(es_pinksplash_07_gp);
DECLARE_GRADIENT_PALETTE(Coral_reef_gp);
DECLARE_GRADIENT_PALETTE(es_ocean_breeze_068_gp);
DECLARE_GRADIENT_PALETTE(es_ocean_breeze_036_gp);
DECLARE_GRADIENT_PALETTE(departure_gp);
DECLARE_GRADIENT_PALETTE(es_landscape_64_gp);
DECLARE_GRADIENT_PALETTE(es_landscape_33_gp);
DECLARE_GRADIENT_PALETTE(rainbowsherbet_gp);
DECLARE_GRADIENT_PALETTE(gr65_hult_gp);
DECLARE_GRADIENT_PALETTE(gr64_hult_gp);
DECLARE_GRADIENT_PALETTE(GMT_drywet_gp);
DECLARE_GRADIENT_PALETTE(ib15_gp);
DECLARE_GRADIENT_PALETTE(Fuschia_7_gp);
DECLARE_GRADIENT_PALETTE(es_emerald_dragon_08_gp);
DECLARE_GRADIENT_PALETTE(lava_gp);
DECLARE_GRADIENT_PALETTE(fire_gp);
DECLARE_GRADIENT_PALETTE(Colorfull_gp);
DECLARE_GRADIENT_PALETTE(Magenta_Evening_gp);
DECLARE_GRADIENT_PALETTE(Pink_Purple_gp);
DECLARE_GRADIENT_PALETTE(Sunset_Real_gp);
DECLARE_GRADIENT_PALETTE(es_autumn_19_gp);
DECLARE_GRADIENT_PALETTE(BlacK_Blue_Magenta_White_gp);
DECLARE_GRADIENT_PALETTE(BlacK_Magenta_Red_gp);
DECLARE_GRADIENT_PALETTE(BlacK_Red_Magenta_Yellow_gp);
DECLARE_GRADIENT_PALETTE(Blue_Cyan_Yellow_gp);

DECLARE_GRADIENT_PALETTE(purple_gp);
DECLARE_GRADIENT_PALETTE(outrun_gp);
//DECLARE_GRADIENT_PALETTE(greenblue_gp);
DECLARE_GRADIENT_PALETTE(redyellow_gp);
// Эффект "Цветной шум"
DECLARE_GRADIENT_PALETTE( pit );


class GradientPalette{
    CRGBPalette32 pl;
    int sh;
    int mn;
    int mx;
    public:
    GradientPalette(CRGBPalette32 pallete = CRGBPalette32(CRGB::Black), uint8_t shift = 0, uint8_t min = 0, uint8_t max = 0) :
        pl(pallete), sh(shift), mn(min), mx(max) {};
    CRGB GetColor(uint8_t idx, uint8_t br) {
        if (mn && idx < mn) idx = mn;
        if (mx && idx > mx) idx = mx;
        return ColorFromPalette(pl, (uint8_t)idx + sh, br);
    }
};

class GradientPaletteList{
    LList<GradientPalette> palletes;
    public:
    GradientPaletteList(){}
    // ~GradientPaletteList(){} d-tor is trivial

    // return element at [index], if index is out of bounds returns last element
    GradientPalette operator[](unsigned i){ return (i < size()) ? palletes[i] : palletes.back(); }

    int size(){ return palletes.size(); }
    void del(int idx){ palletes.unlink(idx); }
    int add(CRGBPalette32 pallete, int shift, uint8_t min = 0, uint8_t max = 0) {
        GradientPalette p(pallete, shift, min, max);
        palletes.add(p);
        return size();
    }
    int add(int idx, CRGBPalette32 pallete, int shift, uint8_t min = 0, uint8_t max = 0) {
        if (palletes.exist(idx)){
            GradientPalette p(pallete, shift, min, max);
            palletes.set(idx, p);
        } else {
            GradientPalette p(pallete, shift, min, max);
            palletes.add(p);
        }
        return size();
    }
};

#endif
