#ifndef RELAP7INDICES3EQN_H
#define RELAP7INDICES3EQN_H

namespace RELAP73Eqn
{
/// Number of solution variables, plus cross-sectional area
static const unsigned int N_CONS_VAR = 4;
/// Indices for solution variables, plus cross-sectional area
enum VariableIndex
{
  CONS_VAR_RHOA = 0,
  CONS_VAR_RHOUA = 1,
  CONS_VAR_RHOEA = 2,
  CONS_VAR_AREA = 3
};

/// Number of equations
static const unsigned int N_EQ = 3;
/// Indices for equations
enum EquationIndex
{
  EQ_MASS = 0,
  EQ_MOMENTUM = 1,
  EQ_ENERGY = 2
};

/// Number of slopes
static const unsigned int N_SLOPES = 3;
/// Indices for slope array
enum SlopeIndex
{
  SLOPE_PRESSURE = 0,
  SLOPE_VELOCITY = 1,
  SLOPE_TEMPERATURE = 2
};
}

#endif // RELAP7INDICES3EQN_H
