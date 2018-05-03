//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALBASENOSPD_H
#define MATERIALBASENOSPD_H

#include "DerivativeMaterialInterface.h"
#include "MechanicsMaterialBasePD.h"
#include "RankTwoTensor.h"

class MaterialBaseNOSPD;

template <>
InputParameters validParams<MaterialBaseNOSPD>();

/**
 * Base material class for correspondence material model
 */
class MaterialBaseNOSPD : public DerivativeMaterialInterface<MechanicsMaterialBasePD>
{
public:
  MaterialBaseNOSPD(const InputParameters & parameters);
  virtual void initQpStatefulProperties() override;

protected:
  virtual void computeProperties() override;
  virtual void computeBondStretch() override;

  /**
   * Function to compute the bond-associated deformation gradient
   */
  virtual void computeQpDeformationGradient();
  virtual void computeQpStrain() = 0;

  ///@{ Material properties to store
  std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains;

  MaterialProperty<RankTwoTensor> & _shape_tensor;
  MaterialProperty<RankTwoTensor> & _deformation_gradient;

  MaterialProperty<RankTwoTensor> & _ddgraddu;
  MaterialProperty<RankTwoTensor> & _ddgraddv;
  MaterialProperty<RankTwoTensor> & _ddgraddw;

  MaterialProperty<RankTwoTensor> & _total_strain;
  MaterialProperty<RankTwoTensor> & _mechanical_strain;

  MaterialProperty<Real> & _multi;
  ///@}
};

#endif // MATERIALBASENOSPD_H
