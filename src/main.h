#include "pebble.h"
#include "pebble_process_info.h"  // Used to display app info

// =========================================================================================================== //
//  App Specific
// =========================================================================================================== //
// ------------------------------------------------------------------------ //
//  Global Settings
// ------------------------------------------------------------------------ //
#define logging true  // Enable/Disable logging for debugging

// ------------------------------------------------------------------------ //
//  Global Structures and Enumerations
// ------------------------------------------------------------------------ //

// =========================================================================================================== //
//  Logging
// =========================================================================================================== //
//#ifndef printf
//  #define printf APP_LOG
//#endif

//Note: printf uses APP_LOG_LEVEL_DEBUG
#if logging
  #define LOG(...) (printf(__VA_ARGS__))
#else
  #define LOG(...)
#endif

// =========================================================================================================== //
//  Inline Multi-Pebble Support
// =========================================================================================================== //
#if defined(PBL_PLATFORM_APLITE)
  #define IF_APLITE(statement) (statement)
  #define IF_BASALT(statement)
  #define IF_CHALK(statement)
#elif defined(PBL_PLATFORM_BASALT)
  #define IF_APLITE(statement)
  #define IF_BASALT(statement) (statement)
  #define IF_CHALK(statement)
#elif defined(PBL_PLATFORM_CHALK)
  #define IF_APLITE(statement)
  #define IF_BASALT(statement)
  #define IF_CHALK(statement) (statement)
#else
  #define IF_APLITE(statement)
  #define IF_BASALT(statement)
  #define IF_CHALK(statement)
#endif


#if defined(PBL_COLOR)
  #define IF_COLOR(color) (color)
  #define IF_BW(bw)
  #define IF_COLOR_ELSE(color, other) (color)
  #define IF_BW_ELSE(bw, other) (other)
  #define IF_COLOR_BW(color, bw) (color)
  #define IF_BW_COLOR(bw, color) (color)
  #define IF_COLOR_BW_ELSE(color, bw, other) (color)
  #define IF_BW_COLOR_ELSE(bw, color, other) (color)
#elif defined(PBL_BW)
  #define IF_COLOR(color)
  #define IF_BW(bw) (bw)
  #define IF_COLOR_ELSE(color, other) (other)
  #define IF_BW_ELSE(bw, other) (bw)
  #define IF_COLOR_BW(color, bw) (bw)
  #define IF_BW_COLOR(bw, color) (bw)
  #define IF_COLOR_BW_ELSE(color, bw, other) (bw)
  #define IF_BW_COLOR_ELSE(bw, color, other) (bw)
#else
  #define IF_COLOR(color)
  #define IF_BW(bw)
  #define IF_COLOR_ELSE(color, other) (other)
  #define IF_BW_ELSE(bw, other) (other)
  #define IF_COLOR_BW(color, bw)
  #define IF_BW_COLOR(bw, color)
  #define IF_COLOR_BW_ELSE(color, bw, other) (other)
  #define IF_BW_COLOR_ELSE(bw, color, other) (other)
#endif

  
#if defined(PBL_SDK_2)
  #define IF_SDK2(SDK2) (SDK2)
  #define IF_SDK3(SDK3)
  #define IF_SDK2_ELSE(SDK2, other) (SDK2)
  #define IF_SDK3_ELSE(SDK3, other) (other)
  #define IF_SDK2_SDK3(SDK2, SDK3) (SDK2)
  #define IF_SDK2_SDK3_ELSE(SDK2, SDK3, other) (SDK2)
#elif defined(PBL_SDK_3)
  #define IF_SDK2(SDK2)
  #define IF_SDK3(SDK3) (SDK3)
  #define IF_SDK2_ELSE(SDK2, other) (other)
  #define IF_SDK3_ELSE(SDK3, other) (SDK3)
  #define IF_SDK2_SDK3(SDK2, SDK3) (SDK3)
  #define IF_SDK2_SDK3_ELSE(SDK2, SDK3, other) (SDK3)
#else
  #define IF_SDK2(SDK2)
  #define IF_SDK3(SDK3)
  #define IF_SDK2_ELSE(SDK2, other) (other)
  #define IF_SDK3_ELSE(SDK3, other) (other)
  #define IF_SDK2_SDK3(SDK2, SDK3)
  #define IF_SDK2_SDK3_ELSE(SDK2, SDK3, other) (other)
#endif

#if defined(PBL_RECT)
  #define IF_RECT(rect) (rect)
  #define IF_ROUND(round)
  #define IF_RECT_ELSE(rect, other) (rect)
  #define IF_ROUND_ELSE(round, other) (other)
  #define IF_RECT_ROUND(rect, round) (rect)
  #define IF_ROUND_RECT(round, rect) (rect)
  #define IF_RECT_ROUND_ELSE(rect, round, other) (rect)
  #define IF_ROUND_RECT_ELSE(round, rect, other) (rect)
#elif defined(PBL_ROUND)
  #define IF_RECT(rect)
  #define IF_ROUND(round) (round)
  #define IF_RECT_ELSE(rect, other) (other)
  #define IF_ROUND_ELSE(round, other) (round)
  #define IF_RECT_ROUND(rect, round) (round)
  #define IF_ROUND_RECT(round, rect) (round)
  #define IF_RECT_ROUND_ELSE(rect, round, other) (round)
  #define IF_ROUND_RECT_ELSE(round, rect, other) (round)
#else
  #define IF_RECT(rect)
  #define IF_ROUND(round)
  #define IF_RECT_ELSE(rect, other) (other)
  #define IF_ROUND_ELSE(round, other) (other)
  #define IF_RECT_ROUND(rect, round)
  #define IF_ROUND_RECT(round, rect)
  #define IF_RECT_ROUND_ELSE(rect, round, other) (other)
  #define IF_ROUND_RECT_ELSE(round, rect, other) (other)
#endif

// =========================================================================================================== //
//  Color Remapping
//  Copied from: https://github.com/cpfair/sand/blob/master/modules/colour_bw_remap.h
// =========================================================================================================== //
// Rather than compute the fallback during code generation
// This will allow us to continue to use GColorWhatever on Aplite without worrying about the B&W fallback
// The logic is: lightness > 0.5 ? GColorWhite : GColorBlack
#ifdef PBL_BW
  #define GColorOxfordBlue GColorBlack
  #define GColorDukeBlue GColorBlack
  #define GColorBlue GColorBlack
  #define GColorDarkGreen GColorBlack
  #define GColorMidnightGreen GColorBlack
  #define GColorCobaltBlue GColorBlack
  #define GColorBlueMoon GColorBlack
  #define GColorIslamicGreen GColorBlack
  #define GColorJaegerGreen GColorBlack
  #define GColorTiffanyBlue GColorBlack
  #define GColorVividCerulean GColorBlack
  #define GColorGreen GColorBlack
  #define GColorMalachite GColorBlack
  #define GColorMediumSpringGreen GColorBlack
  #define GColorCyan GColorBlack
  #define GColorBulgarianRose GColorBlack
  #define GColorImperialPurple GColorBlack
  #define GColorIndigo GColorBlack
  #define GColorElectricUltramarine GColorBlack
  #define GColorArmyGreen GColorBlack
  #define GColorLiberty GColorBlack
  #define GColorVeryLightBlue GColorWhite
  #define GColorKellyGreen GColorBlack
  #define GColorMayGreen GColorBlack
  #define GColorCadetBlue GColorBlack
  #define GColorPictonBlue GColorWhite
  #define GColorBrightGreen GColorBlack
  #define GColorScreaminGreen GColorWhite
  #define GColorMediumAquamarine GColorWhite
  #define GColorElectricBlue GColorWhite
  #define GColorDarkCandyAppleRed GColorBlack
  #define GColorJazzberryJam GColorBlack
  #define GColorPurple GColorBlack
  #define GColorVividViolet GColorBlack
  #define GColorWindsorTan GColorBlack
  #define GColorRoseVale GColorBlack
  #define GColorPurpureus GColorBlack
  #define GColorLavenderIndigo GColorWhite
  #define GColorLimerick GColorBlack
  #define GColorBrass GColorBlack
  #define GColorBabyBlueEyes GColorWhite
  #define GColorSpringBud GColorBlack
  #define GColorInchworm GColorWhite
  #define GColorMintGreen GColorWhite
  #define GColorCeleste GColorWhite
  #define GColorRed GColorBlack
  #define GColorFolly GColorBlack
  #define GColorFashionMagenta GColorBlack
  #define GColorMagenta GColorBlack
  #define GColorOrange GColorBlack
  #define GColorSunsetOrange GColorWhite
  #define GColorBrilliantRose GColorWhite
  #define GColorShockingPink GColorWhite
  #define GColorChromeYellow GColorBlack
  #define GColorRajah GColorWhite
  #define GColorMelon GColorWhite
  #define GColorRichBrilliantLavender GColorWhite
  #define GColorYellow GColorBlack
  #define GColorIcterine GColorWhite
  #define GColorPastelYellow GColorWhite
  #define GColorLightGray GColorWhite
  #define GColorDarkGray GColorBlack
#endif

// =========================================================================================================== //
