/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EIGENSTRAINBASEMATERIAL_H
#define EIGENSTRAINBASEMATERIAL_H

#include "LinearElasticMaterial.h"
#include "DerivativeMaterialInterface.h"

/**
 * EigenStrainBaseMaterial is a base to construct material kernels that represent
 * different eigenstrain - concentration relationships.
 * It provides place holders for calculating eigenstrain and its 1st and 2nd
 * order derivatives with respect to c, elasticity_tensor and its 1st and 2nd
 * order derivatives wrt c if it is a function of c instead of a constant.
 */
class EigenStrainBaseMaterial : public DerivativeMaterialInterface<LinearElasticMaterial>
{
public:
  EigenStrainBaseMaterial(const InputParameters & parameters);

protected:
  virtual void computeEigenStrain() = 0;
  virtual RankTwoTensor computeStressFreeStrain();

  const VariableValue & _c;
  VariableName _c_name;

  std::string _eigenstrain_name;
  MaterialProperty<RankTwoTensor> & _eigenstrain;

  MaterialProperty<RankTwoTensor> & _delastic_strain_dc;
  MaterialProperty<RankTwoTensor> & _d2elastic_strain_dc2;

  MaterialProperty<RankFourTensor> & _delasticity_tensor_dc;
  MaterialProperty<RankFourTensor> & _d2elasticity_tensor_dc2;
};

#endif // EIGENSTRAINBASEMATERIAL_H
