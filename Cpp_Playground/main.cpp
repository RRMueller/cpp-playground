#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <math.h>

typedef struct
{
  uint8_t byte;
  uint8_t bit;
  uint8_t len;
} SPN_Config;

long long epoch();
long long epochMillis();
uint64_t hours();
uint64_t minutes();
uint64_t seconds();
uint64_t millis();
uint64_t micros();
uint64_t nanos();
bool timerMillis(uint64_t* prevTime, uint64_t timeout, bool resetPrevTime, uint64_t current_time, bool useFakeMillis);
double scale(double input, double minIn, double maxIn, double minOut, double maxOut, bool clipOutput);

int64_t random(void);
int64_t random(int64_t max);
int64_t random(int64_t min, int64_t max);
bool random(bool* trigger, int64_t* output, bool noWait);
bool testRandom(bool noWait, uint8_t* count);
int getBoolFromCanTelegram(uint8_t telegram[], uint8_t sizeOfTelegram, bool* output, SPN_Config spnConfig);
int getIntFromCanTelegram(uint8_t telegram[], uint8_t sizeOfTelegram, int* output, SPN_Config spnConfig);

auto startTime = std::chrono::steady_clock::now(); //steady clock is great for timers, not great for epoch

# define PI 3.14159265358979323846
# define PI_2 1.57079632679489661923



float invSqrt(float x)
{
  float halfx = 0.5f * x;
  union
  {
    float f;
    long i;
  } conv = { x };
  conv.i = 0x5f3759df - (conv.i >> 1);
  conv.f *= 1.5f - (halfx * conv.f * conv.f);
  conv.f *= 1.5f - (halfx * conv.f * conv.f);
  return conv.f;
}

float fsc_sqrt(float x)
{
  return (1.0f / invSqrt(x)); // This is probably the fastest way to approximate this...
}


float fsc_asinf(float x)
{
  // https://developer.download.nvidia.com/cg/asin.html
  float negate = (x < 0) ? -1.0f : 1.0f;
  x = abs(x);
  float ret = -0.0187293;
  ret *= x;
  ret += 0.0742610;
  ret *= x;
  ret -= 0.2121144;
  ret *= x;
  ret += 1.5707288;
  ret = PI_2 - sqrt(1.0 - x) * ret;
  return ret /*- 2 */ * negate /** ret*/;
}

float fsc_atan2f(float y, float x)
{
  // http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
  // https://dspguru.com/dsp/tricks/fixed-point-atan2-with-self-normalization/
  const float ONEQTR_PI = PI / 4.0;
  const float THRQTR_PI = 3.0 * PI / 4.0;
  float r, angle;
  float abs_y = fabs(y) + 1e-10f; // kludge to prevent 0/0 condition
  if (x < 0.0f)
  {
    r = (x + abs_y) / (abs_y - x);
    angle = THRQTR_PI;
  }
  else
  {
    r = (x - abs_y) / (x + abs_y);
    angle = ONEQTR_PI;
  }
  angle += (0.1963f * r * r - 0.9817f) * r;
  if (y < 0.0f)
    return (-angle); // negate if in quad III or IV
  else
    return (angle);
}

#define SHIFT_8b 8


#define SHIFT_MSG_NUM 56
#define SHIFT_TOT_MSG 54
#define SHIFT_DTC_LAMPS 50
#define SHIFT_DTC_MSG_1 40
#define SHIFT_DTC_MSG_2 30
#define SHIFT_DTC_MSG_3 20
#define SHIFT_DTC_MSG_4 10
#define SHIFT_DTC_MSG_5 0

// #define MAX_NUM_DTC_MSGS 20
#define MAX_NUM_ENC_DTC_MSGS 4
#define MAX_NUM_DTCS_PER_ENC_MSG 5
#define MAX_NUM_BYTES_PER_DTC_MSG 8
#define MASK_2LSB 0x03
#define MASK_4LSB 0x0F
#define MASK_8LSB 0xFF
#define MASK_10LSB 0x3FF

#define RBR_ISOBUS_DTC_LIST_SIZE_DU16           20u

typedef struct rbr_isobus_dtc_t
{
  uint32_t spn_u32;
  uint8_t  fmi_u8;
  uint8_t  occ_u8;
} rbr_isobus_dtc_ts;

enum DTC_Codes
{
  DFC_EGRVlvDrftClsd,
  DFC_FuelPLoP,
  DFC_FuelPSRCMax,
  DFC_FuelPSRCMin,
  DFC_FlFWLvlWtHi,
  DFC_OilPSwmpPhysRngHi,
  DFC_OilPSwmpPhysRngLo,
  DFC_OilPSwmpSRCMax,
  DFC_OilPSwmpSRCMin,
  DFC_TCACDsPhysRngHi,
  DFC_PAirFltDSRCMax,
  DFC_PAirFltDSRCMin,
  DFC_AirFltClogDet,
  DFC_PEnvRngChkMax,
  DFC_PEnvRngChkMin,
  DFC_PEnvSnsrPlaus,
  DFC_PEnvSigRngMax,
  DFC_PEnvSigRngMin,
  DFC_CEngDsTPhysRngHi,
  DFC_CEngDsTSRCMax,
  DFC_CEngDsTSRCMin,
  DFC_CEngDsTNplHigh,
  DFC_CEngDsTAbsTst,
  DFC_CEngDsTDynTst,
  DFC_RailPSRCMax,
  DFC_RailPSRCMin,
  DFC_AltIOMonPlaus,
  DFC_BattUSRCMax,
  DFC_BattUSRCMin,
  DFC_FuelTPhysRngHi,
  DFC_FuelTSRCMax,
  DFC_FuelTSRCMin,
  DFC_OilTPhysRngHi,
  DFC_OilTSRCMax,
  DFC_OilTSRCMin,
  DFC_OilTNplHigh,
  DFC_EpmCaSI1OfsErr,
  DFC_EpmCaSI1ErrSig,
  DFC_EpmCrSErrSig,
  DFC_EpmCaSI1NoSig,
  DFC_EpmCrSNoSig,
  DFC_StrtCoilHSSCB,
  DFC_GbxAliveChk,
  DFC_BusDiagBusOffNodeA,
  DFC_InjVlv_DI_ScCyl_0,
  DFC_InjVlv_DI_ScHsLs_0,
  DFC_InjVlv_DI_NoLd_0,
  DFC_IVAdjDiaIVAdj_0,
  DFC_InjVlv_DI_ScCyl_3,
  DFC_InjVlv_DI_ScHsLs_3,
  DFC_InjVlv_DI_NoLd_3,
  DFC_IVAdjDiaIVAdj_3,
  DFC_InjVlv_DI_ScCyl_1,
  DFC_InjVlv_DI_ScHsLs_1,
  DFC_InjVlv_DI_NoLd_1,
  DFC_IVAdjDiaIVAdj_1,
  DFC_InjVlv_DI_ScCyl_2,
  DFC_InjVlv_DI_ScHsLs_2,
  DFC_InjVlv_DI_NoLd_2,
  DFC_IVAdjDiaIVAdj_2,
  DFC_GlwPlgDiff,
  DFC_GlwPlgLVSSCB,
  DFC_GlwPlgLVSSCG,
  DFC_GlwPlgLVSOL,
  DFC_GlwPlgDiagErr,
  DFC_GlwPlgLVSOvrTemp,
  DFC_GlwPlg2of3,
  DFC_StrtLSSCB,
  DFC_StrtLSSCG,
  DFC_StrtOL,
  DFC_T50Err,
  DFC_StrtLSOvrTemp,
  DFC_ComCM1TO,
  DFC_MeUnOL,
  DFC_MeUnOT,
  DFC_MeUnShCirLSBatt,
  DFC_MeUnShCirLSGnd,
  DFC_EngICO,
  DFC_TECUSigRngMax,
  DFC_TECUSigRngMin,
  DFC_TECUPhysRngHi,
  DFC_BusDiagBusOffNodeB,
  DFC_PCVOL,
  DFC_PCVOT,
  DFC_PCVShCirLSBatt,
  DFC_PCVShCirLSGnd,
  DFC_EngPrtOvrSpd,
  DFC_MRlyErlyOpng,
  DFC_EGRVlvJamVlvOpn,
  DFC_EGRVlvJamVlvClsd,
  DFC_EGRVlvSRCMax,
  DFC_EGRVlvSRCMin,
  DFC_EGRVlvDrftOpn,
  DFC_EGRVlvGovDvtMin,
  DFC_EGRVlvGovDvtMax,
  DFC_EEPWrErr,
  DFC_EEPRdErr,
  DFC_SSpMon1,
  DFC_SSpMon2,
  DFC_SSpMon3,
  DFC_GlwPlgPLUGSC_0,
  DFC_GlwPlgPLUGErr_0,
  DFC_GlwPlgPLUGSC_1,
  DFC_GlwPlgPLUGErr_1,
  DFC_GlwPlgPLUGSC_2,
  DFC_GlwPlgPLUGErr_2,
  DFC_GlwPlgPLUGSC_3,
  DFC_GlwPlgPLUGErr_3,
  DFC_EGRVlvHBrgShCirBatt1,
  DFC_EGRVlvHBrgShCirGnd1,
  DFC_EGRVlvHBrgOpnLd,
  DFC_EGRVlvHBrgOvrTemp,
  DFC_EGRVlvHBrgShCirBatt2,
  DFC_EGRVlvHBrgShCirGnd2,
  DFC_StrtHSSCB,
  DFC_StrtHSSCG,
  DFC_StrtHSOvrTemp,
  DFC_PSPSCB,
  DFC_PSPSCG,
  DFC_PSPOL,
  DFC_PSPOvrTemp,
  DFC_BusDiagBusOffErrPasNodeA,
  DFC_BusDiagBusOffErrPasNodeB,
  DFC_ComTSC1TETO,
  DFC_RailPCV0,
  DFC_RailPCV2,
  DFC_RailPCV42,
  DFC_RailPCV4,
  DFC_InjVlv_DI_ScBnk_0,
  DFC_InjVlv_DI_ScBnk_1,
  DFC_RailMeUn0,
  DFC_RailMeUn4,
  DFC_GlwPlgUnErr,
  DFC_GlwPlgT30Miss,
  DFC_MoCADCTst,
  DFC_MoCADCVltgRatio,
  DFC_MoCComErrCnt,
  DFC_MoCComSPI,
  DFC_MoCROMErrXPg,
  DFC_MoCSOPErrMMRespByte,
  DFC_MoCSOPErrNoChk,
  DFC_MoCSOPErrRespTime,
  DFC_MoCSOPErrSPI,
  DFC_MoCSOPLoLi,
  DFC_MoCSOPMM,
  DFC_MoCSOPOSTimeOut,
  DFC_MoCSOPPsvTstErr,
  DFC_MoCSOPTimeOut,
  DFC_MoCSOPUpLi,
  DFC_MoFAPP,
  DFC_MoFESpd,
  DFC_MoFInjDatET,
  DFC_MoFInjDatPhi,
  DFC_MoFInjQnt,
  DFC_MoFMode2,
  DFC_MoFMode3,
  DFC_MoFOvR,
  DFC_MoFOvRHtPrt,
  DFC_MoFQntCor,
  DFC_MoFRailP,
  DFC_MoFRmtAPP,
  DFC_MoFTrqCmp,
  DFC_MonLimCurr,
  DFC_MonLimLead,
  DFC_MonLimSet,
  DFC_MoFInjDatBlkShtET,
  DFC_OCWDACom_ERR,
  DFC_OCWDALowVltg,
  DFC_OCWDAOvrVltg,
  DFC_OCWDAReasUnkwn,
  DFC_RailMeUn10,
  DFC_RailMeUn2,
  DFC_SWReset_0,
  DFC_SWReset_1,
  DFC_SWReset_2,
  DFC_MoCADCNTP,
  DFC_MoFStrt,
  DFC_Cy327SpiCom,
  NUM_DTC_CODES // Special value to represent the total number of DTC codes
};






// DM1/DM2 CODES
rbr_isobus_dtc_ts dtc_info_array[NUM_DTC_CODES] = {
    {.spn_u32 = 27, .fmi_u8 = 17, .occ_u8 = 0},              // EGR Valve
    {.spn_u32 = 94, .fmi_u8 = 13, .occ_u8 = 0},                    // Low fuel pressure error monitoring
    {.spn_u32 = 95, .fmi_u8 = 3, .occ_u8 = 0},                  // SRC High for Environment Pressure
    {.spn_u32 = 95, .fmi_u8 = 4, .occ_u8 = 0},                  // SRC low for Environment Pressure
    {.spn_u32 = 97, .fmi_u8 = 15, .occ_u8 = 0},                 // Water in fuel detected
    {.spn_u32 = 100, .fmi_u8 = 0, .occ_u8 = 0},           // Maximum oil pressure error in plausibility check
    {.spn_u32 = 100, .fmi_u8 = 1, .occ_u8 = 0},           // Minimum oil pressure error in plausibility check
    {.spn_u32 = 100, .fmi_u8 = 3, .occ_u8 = 0},              // SRC high for oil pressure sensor
    {.spn_u32 = 100, .fmi_u8 = 0, .occ_u8 = 0},              // SRC low for Oil pressure sensor
    {.spn_u32 = 105, .fmi_u8 = 17, .occ_u8 = 0},            // Physical Range Check high for Charged Air cooler down stream temperature
    {.spn_u32 = 107, .fmi_u8 = 3, .occ_u8 = 0},              // SRC High for Controller Mode Switch
    {.spn_u32 = 107, .fmi_u8 = 4, .occ_u8 = 0},              // SRC low for Controller Mode Switch
    {.spn_u32 = 107, .fmi_u8 = 14, .occ_u8 = 0},              // Error path for Clog Detection in Air filter
    {.spn_u32 = 108, .fmi_u8 = 0, .occ_u8 = 0},               // Ambient air pressure sensor range chack max-error
    {.spn_u32 = 108, .fmi_u8 = 1, .occ_u8 = 0},               // Ambient air pressure sensor range check min-error
    {.spn_u32 = 108, .fmi_u8 = 2, .occ_u8 = 0},               // Ambient air pressure sensor sensor error by component self diagnosis
    {.spn_u32 = 108, .fmi_u8 = 3, .occ_u8 = 0},               // fault check max signal range violated for ambient air pressure sensor
    {.spn_u32 = 108, .fmi_u8 = 4, .occ_u8 = 0},               // fault check min signal range violated for ambient air pressure sensor
    {.spn_u32 = 110, .fmi_u8 = 0, .occ_u8 = 0},            // Physical Range Check high for CEngDsT
    {.spn_u32 = 110, .fmi_u8 = 3, .occ_u8 = 0},               // SRC High for Engine coolant temperature(down stream)
    {.spn_u32 = 110, .fmi_u8 = 4, .occ_u8 = 0},               // SRC low for Engine coolant temperature(down stream)
    {.spn_u32 = 110, .fmi_u8 = 16, .occ_u8 = 0},             // Engine coolant temperature too high plausibility error
    {.spn_u32 = 110, .fmi_u8 = 17, .occ_u8 = 0},              // defect fault check for Absolute plausibility test
    {.spn_u32 = 110, .fmi_u8 = 18, .occ_u8 = 0},              // defect fault check for dynamic plausibility test
    {.spn_u32 = 157, .fmi_u8 = 3, .occ_u8 = 0},                 // Sensor voltage above upper limit
    {.spn_u32 = 157, .fmi_u8 = 4, .occ_u8 = 0},                 // Sensor voltage below lower limit
    {.spn_u32 = 167, .fmi_u8 = 7, .occ_u8 = 0},               // Plausibility check for input signal for monitoring the alternator
    {.spn_u32 = 168, .fmi_u8 = 3, .occ_u8 = 0},                 // Diagnostic Fault Check for Signal Range Max Check of Battery Voltage
    {.spn_u32 = 168, .fmi_u8 = 4, .occ_u8 = 0},                 // Diagnostic Fault Check for Signal Range Min Check of Battery Voltage
    {.spn_u32 = 174, .fmi_u8 = 0, .occ_u8 = 0},              // Physical Range Check high for fuel temperature
    {.spn_u32 = 174, .fmi_u8 = 3, .occ_u8 = 0},                 // SRC high for fuel temperature sensor
    {.spn_u32 = 174, .fmi_u8 = 4, .occ_u8 = 0},                 // SRC low for fuel temperature sensor
    {.spn_u32 = 175, .fmi_u8 = 0, .occ_u8 = 0},               // Physical Range Check high for Oil Temperature
    {.spn_u32 = 175, .fmi_u8 = 3, .occ_u8 = 0},                  // SRC High for Oil Temperature
    {.spn_u32 = 175, .fmi_u8 = 4, .occ_u8 = 0},                  // SRC low for Oil Temperature
    {.spn_u32 = 175, .fmi_u8 = 13, .occ_u8 = 0},                // Oil temperature too high plausibility error
    {.spn_u32 = 190, .fmi_u8 = 2, .occ_u8 = 0},              // DFC for camshaft offset angle exceeded
    {.spn_u32 = 190, .fmi_u8 = 8, .occ_u8 = 0},              // DFC for camshaft signal diagnose - disturbed signal
    {.spn_u32 = 190, .fmi_u8 = 9, .occ_u8 = 0},                // DFC for crankshaft signal diagnose - disturbed signal
    {.spn_u32 = 190, .fmi_u8 = 12, .occ_u8 = 0},              // DFC for camshaft signal diagnose - no signal
    {.spn_u32 = 190, .fmi_u8 = 18, .occ_u8 = 0},                // DFC for crankshaft signal diagnose - no signal
    {.spn_u32 = 430, .fmi_u8 = 3, .occ_u8 = 0},               // Short circuit to battery error at High side of coil in Inhibit starter strategy
    {.spn_u32 = 604, .fmi_u8 = 2, .occ_u8 = 0},                 // Alive Detection for Gbx_stNPos
    {.spn_u32 = 639, .fmi_u8 = 14, .occ_u8 = 0},         // BusOff error CAN A
    {.spn_u32 = 651, .fmi_u8 = 3, .occ_u8 = 0},           // Short circuit of the power stage low-side (cylinder error)
    {.spn_u32 = 651, .fmi_u8 = 4, .occ_u8 = 0},          // Short circuit between high-side and low-side of the power stage (high-side non plausible error)
    {.spn_u32 = 651, .fmi_u8 = 5, .occ_u8 = 0},            // Open load on the power stage
    {.spn_u32 = 651, .fmi_u8 = 13, .occ_u8 = 0},            // check of missing injector adjustment value programming
    {.spn_u32 = 652, .fmi_u8 = 3, .occ_u8 = 0},           // Short circuit of the power stage low-side (cylinder error)
    {.spn_u32 = 652, .fmi_u8 = 4, .occ_u8 = 0},          // Short circuit between high-side and low-side of the power stage (high-side non plausible error)
    {.spn_u32 = 652, .fmi_u8 = 5, .occ_u8 = 0},            // Open load on the power stage
    {.spn_u32 = 652, .fmi_u8 = 13, .occ_u8 = 0},            // check of missing injector adjustment value programming
    {.spn_u32 = 653, .fmi_u8 = 3, .occ_u8 = 0},           // Short circuit of the power stage low-side (cylinder error)
    {.spn_u32 = 653, .fmi_u8 = 4, .occ_u8 = 0},          // Short circuit between high-side and low-side of the power stage (high-side non plausible error)
    {.spn_u32 = 653, .fmi_u8 = 5, .occ_u8 = 0},            // Open load on the power stage
    {.spn_u32 = 653, .fmi_u8 = 13, .occ_u8 = 0},            // check of missing injector adjustment value programming
    {.spn_u32 = 654, .fmi_u8 = 3, .occ_u8 = 0},           // Short circuit of the power stage low-side (cylinder error)
    {.spn_u32 = 654, .fmi_u8 = 4, .occ_u8 = 0},          // Short circuit between high-side and low-side of the power stage (high-side non plausible error)
    {.spn_u32 = 654, .fmi_u8 = 5, .occ_u8 = 0},            // Open load on the power stage
    {.spn_u32 = 654, .fmi_u8 = 13, .occ_u8 = 0},            // check of missing injector adjustment value programming
    {.spn_u32 = 676, .fmi_u8 = 2, .occ_u8 = 0},                  // DFC for coding error when different coding words were received in a coding cycle
    {.spn_u32 = 676, .fmi_u8 = 3, .occ_u8 = 0},                // Short circuit to battery error for Low Voltage System
    {.spn_u32 = 676, .fmi_u8 = 4, .occ_u8 = 0},                // Short circuit to ground error for Low Voltage System
    {.spn_u32 = 676, .fmi_u8 = 5, .occ_u8 = 0},                 // No load error for Low Voltage System
    {.spn_u32 = 676, .fmi_u8 = 11, .occ_u8 = 0},              // DFC for faulty diagnostic data transmission or protocol error
    {.spn_u32 = 676, .fmi_u8 = 12, .occ_u8 = 0},           // Over temperature error on ECU powerstage for Glow plug Low Voltage System
    {.spn_u32 = 676, .fmi_u8 = 21, .occ_u8 = 0},                 // DFC for coding error when selected coding is not working
    {.spn_u32 = 677, .fmi_u8 = 3, .occ_u8 = 0},                   // Short circuit to battery error for Starter low side
    {.spn_u32 = 677, .fmi_u8 = 4, .occ_u8 = 0},                   // Short circuit to ground error for Starter low side
    {.spn_u32 = 677, .fmi_u8 = 5, .occ_u8 = 0},                      // No load error for Starter
    {.spn_u32 = 677, .fmi_u8 = 10, .occ_u8 = 0},                     // Defective T50 switch
    {.spn_u32 = 677, .fmi_u8 = 12, .occ_u8 = 0},              // Over temperature error for Starter low side
    {.spn_u32 = 986, .fmi_u8 = 9, .occ_u8 = 0},                    // Timeout Error of CAN-Receive-Frame Cab Message 1
    {.spn_u32 = 1076, .fmi_u8 = 5, .occ_u8 = 0},                     // open load of metering unit output
    {.spn_u32 = 1076, .fmi_u8 = 12, .occ_u8 = 0},                    // over teperature of device driver of metering unit
    {.spn_u32 = 1076, .fmi_u8 = 16, .occ_u8 = 0},           // short circuit to battery of metering unit output
    {.spn_u32 = 1076, .fmi_u8 = 18, .occ_u8 = 0},            // short circuit to ground of metering unit output
    {.spn_u32 = 1109, .fmi_u8 = 11, .occ_u8 = 0},                    // Injection cut off demand (ICO) for shut off coordinator
    {.spn_u32 = 1136, .fmi_u8 = 0, .occ_u8 = 0},              // ECU Temperature Sensor MAX
    {.spn_u32 = 1136, .fmi_u8 = 1, .occ_u8 = 0},              // ECU Temperature Sensor MIN
    {.spn_u32 = 1136, .fmi_u8 = 16, .occ_u8 = 0},             // Diagnostic Fault Check for Physical Signal above maximum limit
    {.spn_u32 = 1231, .fmi_u8 = 14, .occ_u8 = 0},        // BusOff error CAN B
    {.spn_u32 = 1244, .fmi_u8 = 5, .occ_u8 = 0},                      // open load of pressure control valve output
    {.spn_u32 = 1244, .fmi_u8 = 12, .occ_u8 = 0},                     // over teperature of device driver of pressure control valve
    {.spn_u32 = 1244, .fmi_u8 = 16, .occ_u8 = 0},            // short circuit to battery of pressure control valve output
    {.spn_u32 = 1244, .fmi_u8 = 18, .occ_u8 = 0},             // short circuit to ground of the pressure control valve output
    {.spn_u32 = 1769, .fmi_u8 = 11, .occ_u8 = 0},              // Overspeed detection in component engine protection
    {.spn_u32 = 2634, .fmi_u8 = 11, .occ_u8 = 0},              // Early opening defect of main relay
    {.spn_u32 = 2791, .fmi_u8 = 0, .occ_u8 = 0},            // EGR Valve OPEN
    {.spn_u32 = 2791, .fmi_u8 = 1, .occ_u8 = 0},           // EGR Valve CLOSED
    {.spn_u32 = 2791, .fmi_u8 = 13, .occ_u8 = 0},              // EGR Valve SCR MAX
    {.spn_u32 = 2791, .fmi_u8 = 14, .occ_u8 = 0},              // EGR Valve SCR MIN
    {.spn_u32 = 2791, .fmi_u8 = 15, .occ_u8 = 0},             // EGR Valve DRFT OPEN
    {.spn_u32 = 2791, .fmi_u8 = 16, .occ_u8 = 0},           // EGR Valve GOV DVT MIN
    {.spn_u32 = 2791, .fmi_u8 = 18, .occ_u8 = 0},           // EGR Valve GOV DVT MAX
    {.spn_u32 = 2802, .fmi_u8 = 12, .occ_u8 = 0},                  // EEP Write Error based on the error in storing the blocks in memory media
    {.spn_u32 = 2802, .fmi_u8 = 14, .occ_u8 = 0},                  // EEP Read Error based on the error in reading blocks from memory media
    {.spn_u32 = 3509, .fmi_u8 = 2, .occ_u8 = 0},                    // Voltage fault at Sensor supply 1
    {.spn_u32 = 3510, .fmi_u8 = 2, .occ_u8 = 0},                    // Voltage fault at Sensor supply 2
    {.spn_u32 = 3511, .fmi_u8 = 2, .occ_u8 = 0},                    // Voltage fault at Sensor supply 3
    {.spn_u32 = 5324, .fmi_u8 = 4, .occ_u8 = 0},             // Array of DFCs for short circuit in i+1th Glow Plug
    {.spn_u32 = 5324, .fmi_u8 = 11, .occ_u8 = 0},           // Array of DFCs for failure in i+1th Glow Plug
    {.spn_u32 = 5325, .fmi_u8 = 4, .occ_u8 = 0},             // Array of DFCs for short circuit in i+1th Glow Plug
    {.spn_u32 = 5325, .fmi_u8 = 11, .occ_u8 = 0},           // Array of DFCs for failure in i+1th Glow Plug
    {.spn_u32 = 5326, .fmi_u8 = 4, .occ_u8 = 0},             // Array of DFCs for short circuit in i+1th Glow Plug
    {.spn_u32 = 5326, .fmi_u8 = 11, .occ_u8 = 0},           // Array of DFCs for failure in i+1th Glow Plug
    {.spn_u32 = 5327, .fmi_u8 = 4, .occ_u8 = 0},             // Array of DFCs for short circuit in i+1th Glow Plug
    {.spn_u32 = 5327, .fmi_u8 = 11, .occ_u8 = 0},           // Array of DFCs for failure in i+1th Glow Plug
    {.spn_u32 = 5763, .fmi_u8 = 33, .occ_u8 = 0},      // EGR VALVE
    {.spn_u32 = 5763, .fmi_u8 = 4, .occ_u8 = 0},        // EGR VALVE
    {.spn_u32 = 5763, .fmi_u8 = 5, .occ_u8 = 0},            // EGR VALVE
    {.spn_u32 = 5763, .fmi_u8 = 12, .occ_u8 = 0},         // EGR VALVE
    {.spn_u32 = 5770, .fmi_u8 = 3, .occ_u8 = 0},       // EGR VALVE
    {.spn_u32 = 5770, .fmi_u8 = 4, .occ_u8 = 0},        // EGR VALVE
    {.spn_u32 = 6385, .fmi_u8 = 3, .occ_u8 = 0},                  // Short circuit to battery error for Starter high side
    {.spn_u32 = 6385, .fmi_u8 = 4, .occ_u8 = 0},                  // Short circuit to ground error for Starter high side
    {.spn_u32 = 6385, .fmi_u8 = 12, .occ_u8 = 0},             // Over temperature error for Starter high side
    {.spn_u32 = 6719, .fmi_u8 = 3, .occ_u8 = 0},                     // short circuit to battery of pre-supply pump output
    {.spn_u32 = 6719, .fmi_u8 = 4, .occ_u8 = 0},                     // short circuit to ground of pre-supply pump output
    {.spn_u32 = 6719, .fmi_u8 = 5, .occ_u8 = 0},                      // open load of pre-supply pump output
    {.spn_u32 = 6719, .fmi_u8 = 12, .occ_u8 = 0},                // Over temperature error on ECU powerstage for Pre supply pump
    {.spn_u32 = 22000, .fmi_u8 = 14, .occ_u8 = 0}, // error passive CAN A
    {.spn_u32 = 22001, .fmi_u8 = 15, .occ_u8 = 0}, // error passive CAN B
    {.spn_u32 = 22040, .fmi_u8 = 19, .occ_u8 = 0},              // Timeout Error of CAN-Receive-Frame TSC1TE
    {.spn_u32 = 523037, .fmi_u8 = 0, .occ_u8 = 0},                 // maximum positive deviation of rail pressure exceeded
    {.spn_u32 = 523040, .fmi_u8 = 0, .occ_u8 = 0},                 // maximum negative rail pressure deviation with closed pressure control valve exceeded
    {.spn_u32 = 523042, .fmi_u8 = 0, .occ_u8 = 0},                // maximum rail pressure exceeded (second stage)
    {.spn_u32 = 523043, .fmi_u8 = 0, .occ_u8 = 0},                 // maximum rail pressure exceeded
    {.spn_u32 = 523350, .fmi_u8 = 4, .occ_u8 = 0},        // Short circuit of the power stage high-side (bank error)
    {.spn_u32 = 523352, .fmi_u8 = 4, .occ_u8 = 0},        // Short circuit of the power stage high-side (bank error)
    {.spn_u32 = 523613, .fmi_u8 = 0, .occ_u8 = 0},                // maximum positive deviation of rail pressure exceeded
    {.spn_u32 = 523613, .fmi_u8 = 16, .occ_u8 = 0},               // maximum rail pressure exceeded
    {.spn_u32 = 523676, .fmi_u8 = 12, .occ_u8 = 0},             // DFC for glow module error in GCU-T
    {.spn_u32 = 523676, .fmi_u8 = 16, .occ_u8 = 0},           // DFC for T30 missing error in GCU-T
    {.spn_u32 = 524059, .fmi_u8 = 12, .occ_u8 = 0},               // Diagnostic fault check to report the ADC test error
    {.spn_u32 = 524060, .fmi_u8 = 12, .occ_u8 = 0},         // Diagnostic fault check to report the error in Voltage ratio in ADC monitoring
    {.spn_u32 = 524061, .fmi_u8 = 12, .occ_u8 = 0},            // Diagnostic fault check to report errors in query-/response-communication
    {.spn_u32 = 524062, .fmi_u8 = 12, .occ_u8 = 0},               // Diagnostic fault check to report errors in SPI-communication
    {.spn_u32 = 524063, .fmi_u8 = 12, .occ_u8 = 0},            // Diagnostic fault check to report multiple error while checking the complete ROM-memory
    {.spn_u32 = 524064, .fmi_u8 = 12, .occ_u8 = 0},     // Loss of synchronization sending bytes to the MM from CPU.
    {.spn_u32 = 524065, .fmi_u8 = 12, .occ_u8 = 0},          // DFC to set a torque limitation once an error is detected before MoCSOP's error reaction is set
    {.spn_u32 = 524066, .fmi_u8 = 12, .occ_u8 = 0},       // Wrong set response time
    {.spn_u32 = 524067, .fmi_u8 = 12, .occ_u8 = 0},            // Too many SPI errors during MoCSOP execution.
    {.spn_u32 = 524068, .fmi_u8 = 12, .occ_u8 = 0},              // Diagnostic fault check to report the error in undervoltage monitoring
    {.spn_u32 = 524069, .fmi_u8 = 12, .occ_u8 = 0},                // Diagnostic fault check to report that WDA is not working correct
    {.spn_u32 = 524070, .fmi_u8 = 12, .occ_u8 = 0},         // OS timeout in the shut off path test. Failure setting the alarm task period.
    {.spn_u32 = 524071, .fmi_u8 = 12, .occ_u8 = 0},         // Diagnostic fault check to report that the positive test failed
    {.spn_u32 = 524072, .fmi_u8 = 12, .occ_u8 = 0},           // Diagnostic fault check to report the timeout in the shut off path test
    {.spn_u32 = 524073, .fmi_u8 = 12, .occ_u8 = 0},              // Diagnostic fault check to report the error in overvoltage monitoring
    {.spn_u32 = 524074, .fmi_u8 = 12, .occ_u8 = 0},                  // Diagnostic fault check to report the accelerator pedal position error
    {.spn_u32 = 524075, .fmi_u8 = 12, .occ_u8 = 0},                 // Diagnostic fault check to report the engine speed error
    {.spn_u32 = 524076, .fmi_u8 = 12, .occ_u8 = 0},             // Diagnostic fault check to report the plausibility error between level 1 energizing time and level 2 information
    {.spn_u32 = 524077, .fmi_u8 = 12, .occ_u8 = 0},            // Diagnostic fault check to report the error due to plausibility between the injection begin v/s injection type
    {.spn_u32 = 524078, .fmi_u8 = 12, .occ_u8 = 0},               // Diagnostic fault check to report the error due to non plausibility in ZFC
    {.spn_u32 = 524080, .fmi_u8 = 12, .occ_u8 = 0},                // Diagnosis fault check to report the error to demand for an ICO due to an error in the PoI2 shut-off
    {.spn_u32 = 524081, .fmi_u8 = 12, .occ_u8 = 0},                // Diagnosis fault check to report the error to demand for an ICO due to an error in the PoI3 efficiency factor
    {.spn_u32 = 524082, .fmi_u8 = 12, .occ_u8 = 0},                  // Diagnostic fault check to report the error due to Over Run
    {.spn_u32 = 524083, .fmi_u8 = 12, .occ_u8 = 0},             // Diagnostic fault check to report the error due to cooling injection in Over Run
    {.spn_u32 = 524084, .fmi_u8 = 12, .occ_u8 = 0},               // Diagnostic fault check to report the error due to injection quantity correction
    {.spn_u32 = 524085, .fmi_u8 = 12, .occ_u8 = 0},                // Diagnostic fault check to report the plausibility error in rail pressure monitoring
    {.spn_u32 = 524086, .fmi_u8 = 12, .occ_u8 = 0},               // Diagnostic fault check to report the remote accelerator pedal position error
    {.spn_u32 = 524087, .fmi_u8 = 12, .occ_u8 = 0},               // Diagnostic fault check to report the error due to torque comparison
    {.spn_u32 = 524088, .fmi_u8 = 12, .occ_u8 = 0},              // Diagnosis of curr path limitation forced by ECU monitoring level 2
    {.spn_u32 = 524089, .fmi_u8 = 12, .occ_u8 = 0},              // Diagnosis of lead path limitation forced by ECU monitoring level 2
    {.spn_u32 = 524090, .fmi_u8 = 12, .occ_u8 = 0},               // Diagnosis of set path limitation forced by ECU monitoring level 2
    {.spn_u32 = 524093, .fmi_u8 = 12, .occ_u8 = 0},       // Diagnostic fault check to report the plausibility error for Blankshot injection
    {.spn_u32 = 524098, .fmi_u8 = 12, .occ_u8 = 0},            // Not the same name cuz it exists elsewhere.. Diagnostic fault check to report "WDA active" due to errors in query-/response communication
    {.spn_u32 = 524099, .fmi_u8 = 12, .occ_u8 = 0},            // Diagnostic fault check to report "ABE active" due to undervoltage detection
    {.spn_u32 = 524100, .fmi_u8 = 12, .occ_u8 = 0},            // Diagnostic fault check to report "ABE active" due to overvoltage detection
    {.spn_u32 = 524101, .fmi_u8 = 12, .occ_u8 = 0},          // Diagnostic fault check to report "WDA/ABE active" due to unknown reason
    {.spn_u32 = 524104, .fmi_u8 = 0, .occ_u8 = 0},               // leakage is detected based on fuel quantity balance
    {.spn_u32 = 524105, .fmi_u8 = 0, .occ_u8 = 0},                // maximum negative rail pressure deviation with metering unit on lower limit is exceeded
    {.spn_u32 = 524120, .fmi_u8 = 14, .occ_u8 = 0},               // Visibility of SoftwareResets in DSM
    {.spn_u32 = 524121, .fmi_u8 = 14, .occ_u8 = 0},               // Visibility of SoftwareResets in DSM
    {.spn_u32 = 524122, .fmi_u8 = 14, .occ_u8 = 0},               // Visibility of SoftwareResets in DSM
    {.spn_u32 = 524124, .fmi_u8 = 12, .occ_u8 = 0},               // Diagnostic fault check to report the NTP error in ADC monitoring
    {.spn_u32 = 524128, .fmi_u8 = 12, .occ_u8 = 0},                 // function monitoring: fault in the monitoring of the start control
    {.spn_u32 = 524131, .fmi_u8 = 12, .occ_u8 = 0}              // CY327 SPI Communication Error
};


// just a linear search function - takes at most 12us to complete at 180 searchable indexes
int16_t GetIndexOfDM1(uint32_t spn, uint8_t fmi, rbr_isobus_dtc_ts dtcs[], uint16_t len)
{
  int i;
  for (i = 0; i < len; i++)
  {
    if (spn == dtcs[i].spn_u32 && fmi == dtcs[i].fmi_u8)
    {
      return i;
    }
  }
  return -1;
}

void EncodeDTCMessages(rbr_isobus_dtc_ts listDTCs[RBR_ISOBUS_DTC_LIST_SIZE_DU16], uint16_t encDTCs[RBR_ISOBUS_DTC_LIST_SIZE_DU16])
{
  int i;
  for (i = 0; i < RBR_ISOBUS_DTC_LIST_SIZE_DU16; i++)
  {
    int16_t output = GetIndexOfDM1(listDTCs[i].spn_u32, listDTCs[i].fmi_u8, dtc_info_array, NUM_DTC_CODES);
    //if (output != -1)
    encDTCs[i] = output;
  }
}

void SerializeDTCMessages(uint8_t lamps, rbr_isobus_dtc_ts listDTCs[RBR_ISOBUS_DTC_LIST_SIZE_DU16], uint8_t numDTCs, uint8_t encodedMessages[MAX_NUM_ENC_DTC_MSGS][MAX_NUM_BYTES_PER_DTC_MSG], uint8_t* numEncodedMessages)
{
  uint16_t encDTCs[RBR_ISOBUS_DTC_LIST_SIZE_DU16] = { 0 };
  EncodeDTCMessages(listDTCs, encDTCs);

  uint64_t encMsg;
  *numEncodedMessages = 1 + (numDTCs / MAX_NUM_DTCS_PER_ENC_MSG); // up to 20 DTCs can be stored from bds, so send up to 4 messages of 5 DTCs each.
  int i;
  for (i = 0; i < MAX_NUM_DTCS_PER_ENC_MSG; i++)
  {
    encMsg = 0; // start off clean
    encMsg |= (uint64_t)(i & MASK_2LSB) << SHIFT_MSG_NUM;
    encMsg |= (uint64_t)(*numEncodedMessages & MASK_2LSB) << SHIFT_TOT_MSG;
    encMsg |= (uint64_t)(lamps & MASK_4LSB) << SHIFT_DTC_LAMPS; // lamps is only 4 last bits                 
    encMsg |= (uint64_t)(encDTCs[i * MAX_NUM_DTCS_PER_ENC_MSG + 0] & MASK_10LSB) << SHIFT_DTC_MSG_1; // listDTCs are 10bit numbers
    encMsg |= (uint64_t)(encDTCs[i * MAX_NUM_DTCS_PER_ENC_MSG + 1] & MASK_10LSB) << SHIFT_DTC_MSG_2;
    encMsg |= (uint64_t)(encDTCs[i * MAX_NUM_DTCS_PER_ENC_MSG + 2] & MASK_10LSB) << SHIFT_DTC_MSG_3;
    encMsg |= (uint64_t)(encDTCs[i * MAX_NUM_DTCS_PER_ENC_MSG + 3] & MASK_10LSB) << SHIFT_DTC_MSG_4;
    encMsg |= (uint64_t)(encDTCs[i * MAX_NUM_DTCS_PER_ENC_MSG + 4] & MASK_10LSB) << SHIFT_DTC_MSG_5;
    int j;
    for (j = 0; j < MAX_NUM_BYTES_PER_DTC_MSG; j++)
    {
      encodedMessages[i][j] = (encMsg >> (SHIFT_8b * j)) & MASK_8LSB; // Converting to a 64-bit num makes parsing easier
    }
  }
}

void DecodeDTCMessages(uint16_t encDTCs[RBR_ISOBUS_DTC_LIST_SIZE_DU16], rbr_isobus_dtc_ts decDTCs[RBR_ISOBUS_DTC_LIST_SIZE_DU16])
{
  int i;
  for (i = 0; i < RBR_ISOBUS_DTC_LIST_SIZE_DU16; i++)
  {
    if (encDTCs[i] != (uint16_t)(-1))
      decDTCs[i] = dtc_info_array[encDTCs[i]];
  }
}

void ParseDTCMessages(uint8_t* lamps, uint8_t encodedMessages[MAX_NUM_ENC_DTC_MSGS][MAX_NUM_BYTES_PER_DTC_MSG], uint8_t numEncodedMessages, rbr_isobus_dtc_ts listDTCs[RBR_ISOBUS_DTC_LIST_SIZE_DU16], uint8_t* numDTCs)
{
  uint16_t encDTCs[RBR_ISOBUS_DTC_LIST_SIZE_DU16];
  uint64_t encMsg = 0;
  int i;
  for (i = 0; i < MAX_NUM_ENC_DTC_MSGS; i++)
  {
    encMsg = 0;
    int j;
    for (j = 0; j < MAX_NUM_BYTES_PER_DTC_MSG; j++)
    {
      encMsg |= (uint64_t)encodedMessages[i][j] << (SHIFT_8b * j); // Converting to a 64-bit num makes parsing easier
    }
    *numDTCs |= (uint64_t)(i & MASK_2LSB) << SHIFT_MSG_NUM;
    *lamps = (encMsg >> SHIFT_DTC_LAMPS) & MASK_4LSB;
    encDTCs[i * MAX_NUM_DTCS_PER_ENC_MSG + 0] = (encMsg >> SHIFT_DTC_MSG_1) & MASK_10LSB;
    encDTCs[i * MAX_NUM_DTCS_PER_ENC_MSG + 1] = (encMsg >> SHIFT_DTC_MSG_2) & MASK_10LSB;
    encDTCs[i * MAX_NUM_DTCS_PER_ENC_MSG + 2] = (encMsg >> SHIFT_DTC_MSG_3) & MASK_10LSB;
    encDTCs[i * MAX_NUM_DTCS_PER_ENC_MSG + 3] = (encMsg >> SHIFT_DTC_MSG_4) & MASK_10LSB;
    encDTCs[i * MAX_NUM_DTCS_PER_ENC_MSG + 4] = (encMsg >> SHIFT_DTC_MSG_5) & MASK_10LSB;
  }
  DecodeDTCMessages(encDTCs, listDTCs);
}


//void SetMRFRMRelay(int byte, int bit, int spnInfoIndex, int state)
//{
//#define BITS_PER_BYTE 8
//  uint8_t data[8] = { 0xFF,0xAA,0,0,0,0,0,0 };
//#define MASK_2LSB 0x03
//  uint8_t byteIndex = byte - 1; // These values start from 1, not 0
//  uint8_t bitIndex = bit - 1;
//  uint8_t mask = MASK_2LSB;
//
//  printf("mask1: %02X\n", mask);
//  uint8_t shift = (BITS_PER_BYTE - bit) - 1;
//  mask = mask << shift;
//
//  printf("mask2: %02X\n", mask);
//  mask = ~mask;
//
//  printf("mask3: %02X\n", mask);
//
//  uint8_t originalVal = data[byteIndex];
//
//  uint8_t oldValue = data[byteIndex] & mask; // remove what we are changing from the current state
//  uint8_t newValue = state << shift; // shift the 2-bit state into position
//  data[byteIndex] = oldValue | newValue; // OR the new state with the masked oldValue
//
//  printf("byteIndex: %02X\n", byteIndex);
//  printf("bitIndex: %02X\n", bitIndex);
//  printf("mask: %02X\n", mask);
//  printf("shift: %d\n", shift);
//  printf("original val: %02X\n", originalVal);
//  printf("masked old val: %02X\n", oldValue);
//  printf("masked new val: %02X\n", newValue);
//  printf("new val: %02X\n", data[byteIndex]);
//  printf("~~~~~~~~~~~~\n");
//}

typedef enum
{
  TYPE_INT,
  TYPE_FLOAT,
  NUM_VAR_TYPES // Special value to represent the total number of DTC codes
} var_type;

typedef struct
{
  uint32_t spnNum;
  uint8_t byte; // we start counting this at 1 instead of 0, since non-programmers made the J1939 standard...
  uint8_t bit;  // we start counting this at 1 instead of 0, since non-programmers made the J1939 standard...
  uint8_t len;
  float scaling;
  int32_t offset;
  var_type varType;
} spn_info;

#define MAX_NUM_SPNS 30
#define BITS_PER_BYTE 8

typedef struct
{
  uint8_t instanceNum;
  uint16_t boxNum;
  uint32_t pgn;
  uint8_t format; // BDS_CAN_STD_DU8 or BDS_CAN_EXD_DU8
  uint8_t prio;
  uint8_t src;
  uint8_t dest;
  uint16_t cycle;
  uint16_t offset;
  uint16_t timeout;
  uint16_t startTimeout;
  uint32_t lenMax;
  uint8_t data[8];
  spn_info spns[MAX_NUM_SPNS];
} can_isobus_info;

int ExtractValueFromCanTelegram(can_isobus_info messageData, int spnInfoIndex, uint64_t* output)
{
  uint8_t byteIndex = messageData.spns[spnInfoIndex].byte - 1;                                                              // These values start from 1, not 0
  uint8_t bitIndex = messageData.spns[spnInfoIndex].bit - 1;                                                                // These values start from 1, not 0
  if ((BITS_PER_BYTE * byteIndex + bitIndex + messageData.spns[spnInfoIndex].len) > (messageData.lenMax * BITS_PER_BYTE)) // Check if we are asking for something outside of telegram's allocation
    return -1;                                                                                                            // Return FSC_ERR if we are going to overrun the array

  uint64_t mask = 0;
  int i = 0;
  for (i; i < messageData.spns[spnInfoIndex].len; i++)
  {
    mask |= (1 << i);
  }
  uint64_t val = 0;
  uint8_t numBytes = 1 + (bitIndex + messageData.spns[spnInfoIndex].len - 1) / BITS_PER_BYTE; // How many bytes does this information span?
  i = 0;
  for (i; i < numBytes; i++)
  {
    val |= messageData.data[byteIndex + i] << (BITS_PER_BYTE * i);
  }
  *output = (val >> bitIndex) & mask;
  return 0;
}

typedef enum
{
  MM7_TX2_ROLL_RATE,
  MM7_TX2_CLU_STAT,
  MM7_TX2_ROLL_RATE_STAT,
  MM7_TX2_CLU_DIAG,
  MM7_TX2_AX,
  MM7_TX2_MSG_CNT,
  MM7_TX2_AX_STAT,
  MM7_TX2_CRC,
  MM7_TX2_NUM
} PGN_MM7_TX2;

can_isobus_info INFO_MM7_A_TX2 = {
    .instanceNum = 1,
    .boxNum = 11,
    .format = 0,
    .cycle = 0,
    .offset = 0,
    .timeout = 2500,
    .startTimeout = 5000,
    .lenMax = 8,
    .spns = {
        {.spnNum = 0, .byte = 1, .bit = 1, .len = 16, .scaling = 0.005, .offset = -0x8000, .varType = TYPE_FLOAT},
        {.spnNum = 0, .byte = 3, .bit = 1, .len = 4, .scaling = 1, .offset = 0, .varType = TYPE_INT},
        {.spnNum = 0, .byte = 3, .bit = 5, .len = 4, .scaling = 1, .offset = 0, .varType = TYPE_INT},
        {.spnNum = 0, .byte = 4, .bit = 1, .len = 8, .scaling = 1, .offset = 0, .varType = TYPE_INT},
        {.spnNum = 0, .byte = 5, .bit = 1, .len = 16, .scaling = 0.00125, .offset = -0x8000, .varType = TYPE_FLOAT},
        {.spnNum = 0, .byte = 7, .bit = 1, .len = 4, .scaling = 1, .offset = 0, .varType = TYPE_INT},
        {.spnNum = 0, .byte = 7, .bit = 5, .len = 4, .scaling = 1, .offset = 0, .varType = TYPE_INT},
        {.spnNum = 0, .byte = 8, .bit = 1, .len = 8, .scaling = 1, .offset = 0, .varType = TYPE_INT}}};

int main()
{
  for (;;)
  {
    static uint64_t prevPrintTime = 0;
    uint64_t printTimeout = 2000;
    uint8_t count = 0;

    INFO_MM7_A_TX2.data[0] = 0xFC;
    INFO_MM7_A_TX2.data[1] = 0x7F;
    INFO_MM7_A_TX2.data[2] = 0x0B;
    INFO_MM7_A_TX2.data[3] = 0x37;
    INFO_MM7_A_TX2.data[4] = 0xD4;
    INFO_MM7_A_TX2.data[5] = 0x81;
    INFO_MM7_A_TX2.data[6] = 0x06;
    INFO_MM7_A_TX2.data[7] = 0x96;

    INFO_MM7_A_TX2.data[0] = 0x64;
    INFO_MM7_A_TX2.data[1] = 0x5B;
    INFO_MM7_A_TX2.data[2] = 0x07;
    INFO_MM7_A_TX2.data[3] = 0x37;
    INFO_MM7_A_TX2.data[4] = 0x59;
    INFO_MM7_A_TX2.data[5] = 0x82;
    INFO_MM7_A_TX2.data[6] = 0x0E;
    INFO_MM7_A_TX2.data[7] = 0x8D;
    // {0xfc, 0x7f, 0x0b, 0x37, 0xd4, 0x81, 0x06, 0x96};
    uint64_t extractedValue = 0;
    ExtractValueFromCanTelegram(INFO_MM7_A_TX2, MM7_TX2_ROLL_RATE, &extractedValue);

    printf("rollrate Extracted: %d\n", extractedValue);
    printf("rollrate actual:    %d\n", INFO_MM7_A_TX2.data[0] | (INFO_MM7_A_TX2.data[1] << 8));

    while (true)
    {
    }
    if (timerMillis(&prevPrintTime, printTimeout, true, 0, false))
    {
    }
  }
  return 0;
}


int getBoolFromCanTelegram(uint8_t telegram[], uint8_t sizeOfTelegram, bool* output, SPN_Config spnConfig)
{
  if ((8 * spnConfig.byte + spnConfig.bit + spnConfig.len) > (sizeOfTelegram * 8)) // Check if we are asking for something outside of telegram's allocation
    return -1;	// Return -1 if we overrun the array?

  int mask = 0;
  int i = 0;
  for (i; i < spnConfig.len; i++)
  {
    mask |= (1 << i);
  }
  int val = (telegram[spnConfig.byte] >> spnConfig.bit) & mask;
  if (val)
    *output = true;
  else
    *output = false;
  return 0;
}

int getIntFromCanTelegram(uint8_t telegram[], uint8_t sizeOfTelegram, int* output, SPN_Config spnConfig)
{
  if ((8 * spnConfig.byte + spnConfig.bit + spnConfig.len) > (sizeOfTelegram * 8)) // Check if we are asking for something outside of telegram's allocation
    return -1;	// Return -1 if we overrun the array?

  int mask = 0;
  int i = 0;
  for (i; i < spnConfig.len; i++)
  {
    mask |= (1 << i);
  }
  int val = 0;
  uint8_t numBytes = 1 + (spnConfig.bit + spnConfig.len) / 8;
  i = 0;
  for (i; i < numBytes; i++)
  {
    val |= telegram[spnConfig.byte + i] << (8 * i);
  }
  *output = (val >> spnConfig.bit) & mask;
  return 0;
}


bool testRandom(bool noWait, uint8_t* count)
{
  bool success = false;
  do
  {
    *count += 1;
    if (random(5) == 5)
    {
      success = true;
    }
  } while (noWait && !success);
  return success;
}


/**
 * @brief Generates a 64-bit random number between `0` and `0x7FFFFFFFFFFFFFFF`. This is
 *			also slower compared to `random(*trigger, *output)` since it waits for the RNG to
 *			successfully generate.
 *
 * @return Returns random number between `0` and `0x7FFFFFFFFFFFFFFF`.
 */
int64_t random(void)
{
  return random(0x7FFFFFFFFFFFFFFF);
}

/**
 * @brief Generates a 64-bit random number between `0` and `max`. This is also slower
 *			compared to `random(*trigger, *output)` since it waits for the RNG to
 *			successfully generate.
 *
 * @param max Maximum bound for output
 * @return Returns random number between `0` and `max`.
 */
int64_t random(int64_t max)
{
  int64_t output;
  bool trigger = true;
  if (max == 0)
  {
    return 0;
  }
  if (random(&trigger, &output, true))
  {
    if (max == -1)	//	Edge conditions...
    {// Do nothing
    }
    else if (max != 0x7FFFFFFFFFFFFFFF)	//	Be inclusive on max value
    {
      max++;
    }
    return output % (max);
  }
  return 0;
}

/**
 * @brief Generates a 64-bit random number between `min` and `max`. NOTE: difference between
      `max` and `min` must be smaller than `0x7FFFFFFFFFFFFFFF`!! This is also slower
      compared to `random(*trigger, *output)` since it waits for the RNG to
      successfully generate.
 *
 * @param min Minimum bound for output
 * @param max Maximum bound for output
 * @return Returns random number between `min` and `max`.
 */
int64_t random(int64_t min, int64_t max)
{
  int64_t minVal = (min < max) ? min : max;	//	Find min/max values incase someone gave us parameters in the wrong order
  int64_t maxVal = (min > max) ? min : max;
  printf("minVal = %lld, maxVal = %lld\n", minVal, maxVal);
  int64_t diff = maxVal - minVal;
  return random(diff) + minVal;
}

/**
 * @brief Generates a 64-bit random number
 *
 * @param trigger Start RNG generation
 * @param *output if RNG generation was a success, the generated random number
 * @param noWait if TRUE, wait until generation is complete before returning, otherwise return even if generation is not complete
 * @return If RNG is sucessful, returns true, otherwise false
 */
bool random(bool* trigger, int64_t* output, bool noWait)
{
#define RND_SIZE_DU16 8 // global data definitions
  bool success = false;
  uint8_t rnd_au8[RND_SIZE_DU16]; // array for storage of random
  // static bool getRandom_l = FALSE;
  uint16_t result_u16;
  if (*trigger != false) // check for request to generate a new random
  {
    *output = ((int64_t)rand() << 32) | rand();
    *trigger = false;
    success = true;
  }
  return success;
}


double scale(double input, double minIn, double maxIn, double minOut, double maxOut, bool clipOutput)
{
  double slope = ((maxOut - minOut) / (maxIn - minIn));
  double intercept = (minOut - (minIn * slope));
  double output = ((slope * input) + intercept);
  if (clipOutput) //  DON'T ALLOW OUTPUT OUTSIDE RANGE
  {
    double minVal = (minOut < maxOut) ? minOut : maxOut;	//	FIND MIN/MAX VALUES - INCASE WE ARE INVERSELY SCALING
    double maxVal = (minOut > maxOut) ? minOut : maxOut;
    if (output > maxVal)
      output = maxVal;
    if (output < minVal)
      output = minVal;
  }
  return output;
}


//---------------------------------------------------------------------------------------------------------
// timerMillis() - RETURNS WHETHER THE DURATION OF TIME SINCE prevTime IS GREATER THAN OR EQUAL TO timeout
// parameters:  prevTime - TIME START
//              timeout - DURATION OF TIME SINCE prevTime TO TRIGGER
//              resetPrevTime - RESET prevTime ONCE DURATION IS REACHED
//              current_time - USE WHEN useFakeMillis = true (1)
//              useFakeMillis - USE current_time INSTEAD OF GETTING millis().  USEFUL WHEN CALLING
//                              FUNCTION OFTEN AND RUNTIME SPEED PRIORITIZES TIMER ACCURACY 
//
// returns:     true (1) = IF current_time (OR millis()) IS GREATER THAN OR EQUAL TO prevTime + timeout
//              false (0) = IF NOT true
//---------------------------------------------------------------------------------------------------------
bool timerMillis(uint64_t* prevTime, uint64_t timeout, bool resetPrevTime, uint64_t current_time, bool useFakeMillis)
{
  if (!useFakeMillis)	   //  IF WE DON'T SAY TO USE OUR GIVEN MILLIS, LET'S CHECK MILLIS OURSELVES!
  {
    current_time = millis();
  }
  if ((uint64_t)(current_time - *prevTime) >= timeout)	  // typecast to create a massive positive number when current_time rolls over to 0
  {
    if (resetPrevTime)
    {
      *prevTime = millis();
    }
    return 1;
  }
  return 0;
}

long long epoch()
{
  long long milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  return milliseconds_since_epoch;
}

long long epochMillis()
{
  //returning epoch from 1970
  long long milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  return milliseconds_since_epoch;
}

//return a 64 bit millis() since the program started. This makes the output very similar to (not) Arduino's hours() function
uint64_t hours()
{
  auto now = std::chrono::steady_clock::now();
  auto now_ms = std::chrono::duration_cast<std::chrono::hours>(now - startTime).count();
  uint64_t ms = (uint64_t)now_ms;
  return ms;
}

//return a 64 bit millis() since the program started. This makes the output very similar to (not) Arduino's minutes() function
uint64_t minutes()
{
  auto now = std::chrono::steady_clock::now();
  auto now_ms = std::chrono::duration_cast<std::chrono::minutes>(now - startTime).count();
  uint64_t ms = (uint64_t)now_ms;
  return ms;
}

//return a 64 bit millis() since the program started. This makes the output very similar to (not) Arduino's seconds() function
uint64_t seconds()
{
  auto now = std::chrono::steady_clock::now();
  auto now_ms = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
  uint64_t ms = (uint64_t)now_ms;
  return ms;
}

//return a 64 bit millis() since the program started. This makes the output very similar to Arduino's millis() function
uint64_t millis()
{
  auto now = std::chrono::steady_clock::now();
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
  uint64_t ms = (uint64_t)now_ms;
  return ms;
}

//return a 64 bit micros() since the program started. This makes the output very similar to Arduino's micros() function
uint64_t micros()
{
  auto now = std::chrono::steady_clock::now();
  auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
  uint64_t us = (uint64_t)now_us;
  return us;
}

//return a 64 bit nanos() since the program started. This makes the output very similar to (not) Arduino's nanos() function
uint64_t nanos()
{
  auto now = std::chrono::steady_clock::now();
  auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - startTime).count();
  uint64_t ns = (uint64_t)now_ns;
  return  ns;
}