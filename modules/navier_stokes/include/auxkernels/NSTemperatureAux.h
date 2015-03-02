/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSTEMPERATUREAUX_H
#define NSTEMPERATUREAUX_H

#include "AuxKernel.h"

// Forward Declarations
class NSTemperatureAux;

template<>
InputParameters validParams<NSTemperatureAux>();

/**
 * Temperature is an auxiliary value computed from the total energy
 * and the velocity magnitude (e_i = internal energy, e_t = total energy):
 * T = e_i / c_v
 *   = (e_t - |u|^2/2) / c_v
 */
class NSTemperatureAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NSTemperatureAux(const std::string & name, InputParameters parameters);

  virtual ~NSTemperatureAux() {}

protected:
  virtual Real computeValue();

  // The temperature depends on velocities and total energy
  VariableValue & _rho;
  VariableValue & _u_vel;
  VariableValue & _v_vel;
  VariableValue & _w_vel;
  VariableValue & _rhoe;

  // Specific heat at constant volume, treated as a single
  // constant value.
  Real _R;
  Real _gamma;
};

#endif // NSTEMPERATUREAUX_H
