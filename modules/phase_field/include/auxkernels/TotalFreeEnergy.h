/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TOTALFREEENERGY_H
#define TOTALFREEENERGY_H

#include "TotalFreeEnergyBase.h"

// Forward Declarations
class TotalFreeEnergy;

template <>
InputParameters validParams<TotalFreeEnergy>();

/**
 * Total free energy (both the bulk and gradient parts), where the bulk free energy has been defined
 * in a material and called f_name
 */
class TotalFreeEnergy : public TotalFreeEnergyBase
{
public:
  TotalFreeEnergy(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Bulk free energy material property
  const MaterialProperty<Real> & _F;

  /// Gradient interface free energy coefficients
  std::vector<const MaterialProperty<Real> *> _kappas;
};

#endif // TOTALFREEENERGY_H
