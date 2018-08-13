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
T stringToEnum(const std::string & s, const std::map<std::string, T> & enum_map);

/**
 * Gets MooseEnum corresponding to an enum, using a map of string to enum
 *
 * @tparam    T             enum type
 * @param[in] default_key   key corresponding to default value
 * @param[in] enum_map      map of string to enum
 */
template <typename T>
MooseEnum getMooseEnum(const std::string & default_key, const std::map<std::string, T> & enum_map);

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
}

template <typename T>
T
RELAP7::stringToEnum(const std::string & s, const std::map<std::string, T> & enum_map)
{
  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!enum_map.count(upper))
    return static_cast<T>(-100);
  else
    return enum_map.at(upper);
}

template <typename T>
MooseEnum
RELAP7::getMooseEnum(const std::string & default_key, const std::map<std::string, T> & enum_map)
{
  std::string keys_string;
  for (typename std::map<std::string, T>::const_iterator it = enum_map.begin();
       it != enum_map.end();
       it++)
    if (it == enum_map.begin())
      keys_string += it->first;
    else
      keys_string += " " + it->first;

  return MooseEnum(keys_string, default_key, true);
}

#endif // ENUMS_H
