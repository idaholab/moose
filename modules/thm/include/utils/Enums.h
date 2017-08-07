#ifndef ENUMS_H
#define ENUMS_H

#include <string>
#include "MooseEnum.h"

namespace RELAP7
{

template <typename T>
T stringToEnum(const std::string & s);

/// Type of the heat transfer geometry
enum EConvHeatTransGeom
{
  CHTG_PIPE,
  CHTG_ROD_BUNDLE
};

template <>
EConvHeatTransGeom stringToEnum<EConvHeatTransGeom>(const std::string & s);

/// Enum with convective heat transfer geometry
MooseEnum getConvHeatTransGeometry(const std::string & name);

// ----------------------------------------------------------------------------

/// Type of the end
enum EEndType
{
  IN, ///< inlet
  OUT ///< outlet
};

template <>
EEndType stringToEnum<EEndType>(const std::string & s);

// ----------------------------------------------------------------------------

enum EValveStatusType
{
  VALVE_CLOSE = 0,    ///< valve is closed
  VALVE_OPEN = 1,     ///< valve is opened
  VALVE_CLOSING = -1, ///< valve is being closed
  VALVE_OPENING = 2   ///< valve is being opened
};

template <>
EValveStatusType stringToEnum<EValveStatusType>(const std::string & s);

/// Enum with valve status
MooseEnum getValveStatusType(const std::string & name = "");

// ----------------------------------------------------------------------------

enum EValveActionType
{
  VALVE_NO_ACTION = 0,   ///< maintaining current status
  VALVE_TURNING_ON = 1,  ///< turning on the valve
  VALVE_TURNING_OFF = -1 ///< turning off the valve
};

template <>
EValveActionType stringToEnum<EValveActionType>(const std::string & s);

/// Enum with valve action
MooseEnum getValveActionType(const std::string & name = "");

// ----------------------------------------------------------------------------

/// Check valve type
enum ECheckValveType
{
  CHECK_VALVE_FLOW = 0,   ///< the type of check valve which closes by flow reversal
  CHECK_VALVE_STATIC = 1, ///< the type of check valve which closes by static differential pressure
  CHECK_VALVE_DYNAMIC = 2 ///< the type of check valve which closes by dynamic differential pressure
};

MooseEnum getCheckValveType(const std::string & str = "FLOW");

template <>
ECheckValveType stringToEnum<ECheckValveType>(const std::string & s);

// ----------------------------------------------------------------------------

enum ETHCouplingType
{
  MOD_DENSITY, ///< moderator density
  MOD_TEMP,    ///< moderator temperature
  FUEL_TEMP    ///< fuel temperature
};

// ----------------------------------------------------------------------------

/// The type of an equation
enum EFlowEquationType
{
  CONTINUITY = 0,
  MOMENTUM = 1,
  ENERGY = 2,
  VOIDFRACTION = 3,
  INVALID = 4
};

template <>
EFlowEquationType stringToEnum<EFlowEquationType>(const std::string & s);

// get MooseEnum with equation type
MooseEnum getFlowEquationType(const std::string & eqn_name = "INVALID");

// ----------------------------------------------------------------------------

/// Type of heat structure
enum EHeatStructureType
{
  HS_TYPE_INVALID,
  HS_TYPE_PLATE,
  HS_TYPE_CYLINDER
};

template <>
EHeatStructureType stringToEnum<EHeatStructureType>(const std::string & s);

/// Enum with the heat structure type
MooseEnum getHeatStructureType(const std::string & name = "PLATE");

// ----------------------------------------------------------------------------

/// Closures type
enum EClosuresType
{
  CLOSURES_SIMPLE = 0,
  CLOSURES_TRACE = 1
};

template <>
EClosuresType stringToEnum<EClosuresType>(const std::string & s);

// ----------------------------------------------------------------------------

enum EFlowRegimeNamesType
{
  FR_DISPERSEDBUBBLE, ///< Weight of DispersedBubble Correlations  PreCHF
  FR_CAPSLUG,         ///< Weight of TaylorCap / Slug Flow
  FR_ANNULARMIST,     ///< Weight of Annular Mist Correlations PreCHF
  FR_STRATIFIED,      ///< Weight of Horiz Stratified Flow Exp PreCHF
  FR_INVERTEDANNULAR, ///< Weight of Inverted Annular Flow Correlations PostCHF
  FR_INVERTEDSLUG,    ///< Weight of InvertedSlug Flow Correlations PostCHF
  FR_DISPERSED        ///< Weight of Dispersed Flow Correlations PostCHF
};

// ----------------------------------------------------------------------------

enum EWallDragFlowRegimeNamesType
{
  WDFR_BUBBLYSLUG,      ///< Weight of Bubbly/Slug Correlations PreCHF
  WDFR_ANNULARMIST,     ///< Weight of Annular/Mist Correlations PreCHF
  WDFR_STRATIFIED,      ///< Weight of Horiz Stratified Flow Exp PreCHF
  WDFR_INVERTEDANNULAR, ///< Weight of Inverted Annular Flow Correlations PostCHF
  WDFR_DISPERSED        ///< Weight of Dispersed Flow Correlations PostCHF
};

// ----------------------------------------------------------------------------

enum EWallHeatTransferRegimeNamesType
{
  WHT_SINGLECONVECTION,   ///< Weight of Single Phase Forced Convection PreCHF
  WHT_TWOPHASECONVECTION, ///< Weight of Two Phase Forced Convection PreCHF
  WHT_FILMCONDENSATION,   ///< Weight of Film Condensation PreCHF
  WHT_SUBCOOLED,          ///< Weight of Subcooled Nucleate boiling PreCHF
  WHT_NUCLEATE,           ///< Weight of Stable Nucleate boiling PreCHF
  WHT_TRANSITION,         ///< Weight of Transition Boiling PreCHF
  WHT_INVERTEDANNULAR,    ///< Weight of Inverted Annular Flow Correlations PostCHF
  WHT_DISPERSED           ///< Weight of Dispersed Flow Correlations PostCHF
};
}

#endif // ENUMS_H
