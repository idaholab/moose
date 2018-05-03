//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MECHANICSBASENOSPD_H
#define MECHANICSBASENOSPD_H

#include "MechanicsBasePD.h"

class MechanicsBaseNOSPD;
class RankTwoTensor;
class RankFourTensor;

template <>
InputParameters validParams<MechanicsBaseNOSPD>();

/**
 * Base kernel class for bond-associated correspondence material models
 */
class MechanicsBaseNOSPD : public MechanicsBasePD
{
public:
  MechanicsBaseNOSPD(const InputParameters & parameters);

protected:
  /**
   * Function to compute derivative of stress with respect to displacements
   */
  virtual RankTwoTensor computeDSDU(unsigned int component, unsigned int nd);

  ///@{ Material point based material properties
  const MaterialProperty<Real> & _multi;
  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankTwoTensor> & _shape;
  const MaterialProperty<RankTwoTensor> & _dgrad;
  const MaterialProperty<RankTwoTensor> & _ddgraddu;
  const MaterialProperty<RankTwoTensor> & _ddgraddv;
  const MaterialProperty<RankTwoTensor> & _ddgraddw;
  const MaterialProperty<RankFourTensor> & _Cijkl;
  ///@}
};

#endif // MECHANICSBASENOSPD_H
