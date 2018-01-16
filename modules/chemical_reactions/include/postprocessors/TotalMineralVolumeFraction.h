/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef TOTALMINERALVOLUMEFRACTION_H
#define TOTALMINERALVOLUMEFRACTION_H

#include "ElementAverageValue.h"

class TotalMineralVolumeFraction;

template <>
InputParameters validParams<TotalMineralVolumeFraction>();

/**
 * Calculates the total volume fraction of the coupled solid mineral
 * species (volume of mineral species / volume of model)
 */
class TotalMineralVolumeFraction : public ElementAverageValue
{
public:
  TotalMineralVolumeFraction(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Molar volume of coupled mineral species (molar mass / density)
  const Real _molar_volume;
};

#endif // TOTALMINERALVOLUMEFRACTION_H
