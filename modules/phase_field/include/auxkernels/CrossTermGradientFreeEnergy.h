/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CROSSTERMGRADIENTFREEENERGY_H
#define CROSSTERMGRADIENTFREEENERGY_H

#include "TotalFreeEnergyBase.h"

// Forward Declarations
class CrossTermGradientFreeEnergy;

template <>
InputParameters validParams<CrossTermGradientFreeEnergy>();

/**
 * Cross term gradient free energy contribution used by ACMultiInterface
 */
class CrossTermGradientFreeEnergy : public TotalFreeEnergyBase
{
public:
  CrossTermGradientFreeEnergy(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  std::vector<std::vector<const MaterialProperty<Real> *>> _kappas;
};

#endif // CROSSTERMGRADIENTFREEENERGY_H
