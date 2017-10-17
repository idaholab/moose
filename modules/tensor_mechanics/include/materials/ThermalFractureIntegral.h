/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef THERMALFRACTUREINTEGRAL_H
#define THERMALFRACTUREINTEGRAL_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

class RankTwoTensor;

/**
 * ThermalFractureIntegral computes the summation of the derivative of the
 * eigenstrains with respect to temperature.
 */
class ThermalFractureIntegral : public DerivativeMaterialInterface<Material>
{
public:
  ThermalFractureIntegral(const InputParameters & parameters);

  virtual void computeQpProperties() override;

protected:
  std::string _base_name;
  const std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const MaterialProperty<RankTwoTensor> *> _deigenstrain_dT;
  MaterialProperty<RankTwoTensor> & _total_deigenstrain_dT;
};

#endif // THERMALFRACTUREINTEGRAL_H
