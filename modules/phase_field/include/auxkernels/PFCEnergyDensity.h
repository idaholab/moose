/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef PFC_ENERGY_DENSITY_H
#define PFC_ENERGY_DENSITY_H

#include "AuxKernel.h"
#include <sstream>

class PFCEnergyDensity;

template <>
InputParameters validParams<PFCEnergyDensity>();

class PFCEnergyDensity : public AuxKernel
{
public:
  PFCEnergyDensity(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  std::vector<const VariableValue *> _vals;
  std::vector<const MaterialProperty<Real> *> _coeff;

  unsigned int _order;
  const MaterialProperty<Real> & _a;
  const MaterialProperty<Real> & _b;
};

#endif // PFC_ENERGY_DENSITY_H
