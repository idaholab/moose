#ifndef ENUMS_H
#define ENUMS_H

namespace RELAP7
{

/// Type of the stabilization
enum EConvHeatTransGeom
{
  CHTG_PIPE,
  CHTG_VERT_BUNDLE_W_XFLOW
};

///< Type of the end
enum EEndType
{
  IN,                 ///< inlet
  OUT,                ///< outlet
  PRIMARY_IN,         ///< heat exchanger primary loop inlet
  PRIMARY_OUT,        ///< heat exchanger primary loop outlet
  SECONDARY_IN,       ///< heat exchanger secondary loop inlet
  SECONDARY_OUT       ///< heat exchanger secondary loop outlet
};

enum EValveStatusType
{
  VALVE_CLOSE = 0,    ///< valve is closed
  VALVE_OPEN = 1,     ///< valve is opened
  VALVE_CLOSING = -1, ///< valve is being closed
  VALVE_OPENING = 2   ///< valve is being opened
};

enum EValveActionType
{
  VALVE_NO_ACTION = 0,    ///< maintaining current status
  VALVE_TURNING_ON = 1,   ///< turning on the valve
  VALVE_TURNING_OFF = -1  ///< turning off the valve
};

enum ETHCouplingType
{
  MOD_DENSITY, ///< moderator density
  MOD_TEMP,    ///< moderator temperature
  FUEL_TEMP    ///< fuel temperature
};

/// The type of an equation
enum EFlowEquationType
{
  CONTINUITY = 0,
  MOMENTUM = 1,
  ENERGY = 2,
  VOIDFRACTION = 3,
  INVALID = 4
};

/// Type of heat structure
enum EHeatStructureType
{
  HS_TYPE_INVALID,
  HS_TYPE_PLATE,
  HS_TYPE_CYLINDER
};

/**
 * Heat flux partitioning model
 */
enum EHeatFluxPartitioningModelType
{
  HFPM_LINEAR = 0,
  HFPM_TRACE  = 1
};

/**
 * Wall boiling model
 */
enum EWallBoilingModelType
{
  WBM_SIMPLE = 0,
  WBM_ORIGINAL = 1
};

enum EFlowRegimeNamesType
{
  FR_DISPERSEDBUBBLE,   ///< Weight of DispersedBubble Correlations  PreCHF
  FR_CAPSLUG,           ///< Weight of TaylorCap / Slug Flow
  FR_ANNULARMIST,       ///< Weight of Annular Mist Correlations PreCHF
  FR_STRATIFIED,        ///< Weight of Horiz Stratified Flow Exp PreCHF
  FR_INVERTEDANNULAR,   ///< Weight of Inverted Annular Flow Correlations PostCHF
  FR_INVERTEDSLUG,      ///< Weight of InvertedSlug Flow Correlations PostCHF
  FR_DISPERSED,         ///< Weight of Dispersed Flow Correlations PostCHF
  FR_LEFTOVER           ///< Weight of original Pre Map correlations to be applied
};

enum EWallDragFlowRegimeNamesType
{
  WDFR_BUBBLYSLUG,      ///< Weight of Bubbly/Slug Correlations PreCHF
  WDFR_ANNULARMIST,     ///< Weight of Annular/Mist Correlations PreCHF
  WDFR_STRATIFIED,      ///< Weight of Horiz Stratified Flow Exp PreCHF
  WDFR_INVERTEDANNULAR, ///< Weight of Inverted Annular Flow Correlations PostCHF
  WDFR_INVERTEDSLUG,    ///< Weight of InvertedSlug Flow Correlations PostCHF
  WDFR_DISPERSED,       ///< Weight of Dispersed Flow Correlations PostCHF
  WDFR_LEFTOVER         ///< Weight of original Pre Map correlations to be applied
};

/**
 * Interfacial heat transfer model used in 7eqn model
 */
enum EInterfacialHeatTransferModelType
{
  IHTM_SIMPLE = 0,
  IHTM_TRACE = 1
};

/**
 * Interfacial drag model used in 7eqn model
 */
enum EInterfacialDragModelType
{
  IDM_CONST = 0,
  IDM_TRACE = 1
};

enum EWallHeatTransferRegimeNamesType
{
  WHTR_CONVECTION,      ///< Weight of Forced Convection PreCHF
  WHTR_SUBCOOLED,       ///< Weight of Subcooled Nucleate boiling PreCHF
  WHTR_NUCLEATE,        ///< Weight of Stable Nucleate boiling PreCHF
  WHTR_TRANSITION,      ///< Weight of Transition Boiling PreCHF
  WHTR_INVERTEDANNULAR, ///< Weight of Inverted Annular Flow Correlations PostCHF
  WHTR_DISPERSED,       ///< Weight of Dispersed Flow Correlations PostCHF
  WHTR_LEFTOVER         ///< Weight of original Pre Map correlations to be applied
};

/**
 * Check valve type
 */
enum ECheckValveType
{
  CHECK_VALVE_FLOW = 0,   ///< the type of check valve which closes by flow reversal
  CHECK_VALVE_STATIC = 1,    ///< the type of check valve which closes by static differential pressure
  CHECK_VALVE_DYNAMIC = 2  ///< the type of check valve which closes by dynamic differential pressure
};
}

#endif // ENUMS_H
