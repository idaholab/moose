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

class ForceStabilizedSmallStrainMechanicsNOSPD;

template <>
InputParameters validParams<ForceStabilizedSmallStrainMechanicsNOSPD>();

/**
 * Kernel class for fictitious force stabilized conventional correspondence material model for small
 * strain
 */
class ForceStabilizedSmallStrainMechanicsNOSPD : public MechanicsBasePD
{
public:
  ForceStabilizedSmallStrainMechanicsNOSPD(const InputParameters & parameters);

protected:
  virtual void computeLocalResidual() override;
  virtual void computeLocalJacobian() override;
  virtual void computeNonlocalJacobian() override;

  void computeLocalOffDiagJacobian(unsigned int coupled_component) override;
  void computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                        unsigned int coupled_component) override;

  /**
   * Function to compute derivative of stress with respect to displacements
   * @param component   The index of displacement component
   * @param nd   The local index of element node (either 1 or 2 for Edge2 element)
   * @return The calculated derivative
   */
  RankTwoTensor computeDSDU(unsigned int component, unsigned int nd);

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

  /// Bond based material property for fictitious stabilization force
  const MaterialProperty<Real> & _sf_coeff;

  /// The index of displacement component
  const unsigned int _component;
};
