/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSTHERMALBC_H
#define NSTHERMALBC_H

#include "NodalBC.h"


// Forward Declarations
class NSThermalBC;

template<>
InputParameters validParams<NSThermalBC>();

class NSThermalBC : public NodalBC
{
public:

  NSThermalBC(const InputParameters & parameters);

  virtual ~NSThermalBC(){}

protected:
  // Computes the temperature based on ideal gas equation of state,
  // the total energy, and the velocity: T = e_i/c_v
  virtual Real computeQpResidual();

  unsigned int _rho_var;
  const VariableValue & _rho;

  Real _initial;
  Real _final;
  Real _duration;

  // Specific heat at constant volume, treated as a single
  // constant value.
  Real _R;
  Real _gamma;
};

#endif //THERMALBC_H
