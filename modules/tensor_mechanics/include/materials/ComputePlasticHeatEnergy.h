/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEPLASTICHEATENERGY_H
#define COMPUTEPLASTICHEATENERGY_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

/**
 * ComputePlasticHeatEnergy computes stress * (plastic_strain - plastic_strain_old)
 * and, if currentlyComputingJacobian, then the derivative of this quantity wrt total strain
 */
class ComputePlasticHeatEnergy : public DerivativeMaterialInterface<Material>
{
public:
  ComputePlasticHeatEnergy(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// optional parameter that allows multiple mechanics materials to be defined
  std::string _base_name;

  /// plastic strain
  const MaterialProperty<RankTwoTensor> & _plastic_strain;

  /// old value of plastic strain
  const MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  /// stress
  const MaterialProperty<RankTwoTensor> & _stress;

  /// d(stress)/d(total strain)
  const MaterialProperty<RankFourTensor> & _Jacobian_mult;

  /// elasticity tensor
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  /// computed property: stress * (plastic_strain - plastic_strain_old) / dt
  MaterialProperty<Real> & _plastic_heat;

  /// d(plastic_heat)/d(total strain)
  MaterialProperty<RankTwoTensor> & _dplastic_heat_dstrain;
};

#endif // COMPUTEPLASTICHEATENERGY_H
