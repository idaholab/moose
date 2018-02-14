#ifndef ENUMS_H
#define ENUMS_H

#include <algorithm>
#include "MooseEnum.h"

namespace RELAP7
{

/**
 * Converts a string to an enum
 *
 * This template is designed to be specialized and use the other version of this
 * function in conjunction with the correct map.
 *
 * @tparam    T          enum type
 * @param[in] s          string to convert
 */
template <typename T>
T stringToEnum(const std::string & s);

/**
 * Converts a string to an enum using a map of string to enum
 *
 * @tparam    T          enum type
 * @param[in] s          string to convert
 * @param[in] enum_map   map of string to enum
 */
template <typename T>
T stringToEnum(const std::string & s,
               const std::map<std::string, T> & enum_map,
               const std::string & description);

/**
 * Gets MooseEnum corresponding to an enum, using a map of string to enum
 *
 * @tparam    T             enum type
 * @param[in] default_key   key corresponding to default value
 * @param[in] enum_map      map of string to enum
 */
template <typename T>
MooseEnum getMooseEnum(const std::string & default_key, const std::map<std::string, T> & enum_map);

/// Type of the heat transfer geometry
enum EConvHeatTransGeom
{
  CHTG_PIPE,
  CHTG_ROD_BUNDLE
};

const std::map<std::string, EConvHeatTransGeom> heat_transfer_geom_to_enum{
    {"PIPE", CHTG_PIPE}, {"ROD_BUNDLE", CHTG_ROD_BUNDLE}};

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

const std::map<std::string, EEndType> pipe_end_type_to_enum{{"IN", IN}, {"OUT", OUT}};

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

const std::map<std::string, EValveStatusType> valve_status_type_to_enum{{"OPEN", VALVE_OPEN},
                                                                        {"CLOSE", VALVE_CLOSE}};

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

const std::map<std::string, EValveActionType> valve_action_type_to_enum{
    {"NO_ACTION", VALVE_NO_ACTION}, {"OPEN", VALVE_TURNING_ON}, {"CLOSE", VALVE_TURNING_OFF}};

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

const std::map<std::string, ECheckValveType> check_valve_type_to_enum{
    {"FLOW", CHECK_VALVE_FLOW}, {"STATIC", CHECK_VALVE_STATIC}, {"DYNAMIC", CHECK_VALVE_DYNAMIC}};

template <>
ECheckValveType stringToEnum<ECheckValveType>(const std::string & s);

MooseEnum getCheckValveType(const std::string & str = "FLOW");

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
};

const std::map<std::string, EFlowEquationType> flow_equation_type_to_enum{
    {"CONTINUITY", CONTINUITY},
    {"MOMENTUM", MOMENTUM},
    {"ENERGY", ENERGY},
    {"VOIDFRACTION", VOIDFRACTION}};

template <>
EFlowEquationType stringToEnum<EFlowEquationType>(const std::string & s);

// get MooseEnum with equation type
MooseEnum getFlowEquationType(const std::string & eqn_name = "");

// ----------------------------------------------------------------------------

/// Type of heat structure
enum EHeatStructureType
{
  HS_TYPE_PLATE,
  HS_TYPE_CYLINDER
};

const std::map<std::string, EHeatStructureType> hs_type_to_enum{{"PLATE", HS_TYPE_PLATE},
                                                                {"CYLINDER", HS_TYPE_CYLINDER}};

template <>
EHeatStructureType stringToEnum<EHeatStructureType>(const std::string & s);

/// Enum with the heat structure type
MooseEnum getHeatStructureType(const std::string & name = "PLATE");

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

const std::map<unsigned int, std::string> flow_regime_type_to_string{
    {FR_DISPERSEDBUBBLE, "dispersed_bubble"},
    {FR_CAPSLUG, "capslug"},
    {FR_ANNULARMIST, "annular_mist"},
    {FR_STRATIFIED, "stratified"},
    {FR_INVERTEDANNULAR, "inverted_annular"},
    {FR_INVERTEDSLUG, "inverted_slug"},
    {FR_DISPERSED, "dispersed"}};

// ----------------------------------------------------------------------------

enum EWallDragFlowRegimeNamesType
{
  WDFR_BUBBLYSLUG,      ///< Weight of Bubbly/Slug Correlations PreCHF
  WDFR_ANNULARMIST,     ///< Weight of Annular/Mist Correlations PreCHF
  WDFR_STRATIFIED,      ///< Weight of Horiz Stratified Flow Exp PreCHF
  WDFR_INVERTEDANNULAR, ///< Weight of Inverted Annular Flow Correlations PostCHF
  WDFR_DISPERSED        ///< Weight of Dispersed Flow Correlations PostCHF
};

const std::map<unsigned int, std::string> wall_drag_flow_regime_type_to_string{
    {WDFR_BUBBLYSLUG, "bubbly_slug"},
    {WDFR_ANNULARMIST, "annular_mist"},
    {WDFR_STRATIFIED, "stratified"},
    {WDFR_INVERTEDANNULAR, "inverted_annular"},
    {WDFR_DISPERSED, "dispersed"}};

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

const std::map<unsigned int, std::string> wall_heat_transfer_flow_regime_type_to_string{
    {WHT_SINGLECONVECTION, "single_phase_forced_convection"},
    {WHT_TWOPHASECONVECTION, "two_phase_forced_convection"},
    {WHT_FILMCONDENSATION, "film_condensation"},
    {WHT_SUBCOOLED, "subcooled_nucleate_boiling"},
    {WHT_NUCLEATE, "stable_nucleate_boiling"},
    {WHT_TRANSITION, "transition_boiling"},
    {WHT_INVERTEDANNULAR, "inverted_annular_postCHF"},
    {WHT_DISPERSED, "dispersed_postCHF"}};
} // namespace RELAP7

#endif // ENUMS_H
