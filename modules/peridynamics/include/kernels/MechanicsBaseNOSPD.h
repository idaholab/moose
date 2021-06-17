//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MechanicsBasePD.h"

/**
 * Base kernel class for bond-associated correspondence material models
 */
class MechanicsBaseNOSPD : public MechanicsBasePD
{
public:
  static InputParameters validParams();

  MechanicsBaseNOSPD(const InputParameters & parameters);

protected:
  /**
   * Function to compute derivative of stress with respect to displacements for small strain
   * problems
   * @param component   The index of displacement component
   * @param nd   The local index of element node (either 1 or 2 for Edge2 element)
   * @return The calculated derivative
   */
  virtual RankTwoTensor computeDSDU(unsigned int component, unsigned int nd);

  ///@{ Material point based material properties
  const MaterialProperty<Real> & _multi;
  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankTwoTensor> & _shape2;
  const MaterialProperty<RankTwoTensor> & _dgrad;
  const MaterialProperty<RankTwoTensor> & _ddgraddu;
  const MaterialProperty<RankTwoTensor> & _ddgraddv;
  const MaterialProperty<RankTwoTensor> & _ddgraddw;
  const MaterialProperty<RankFourTensor> & _Jacobian_mult;
  const std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const MaterialProperty<RankTwoTensor> *> _deigenstrain_dT;
  ///@}
};
