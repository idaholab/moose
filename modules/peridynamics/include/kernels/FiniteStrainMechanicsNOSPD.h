//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MechanicsBaseNOSPD.h"

class FiniteStrainMechanicsNOSPD;

template <>
InputParameters validParams<FiniteStrainMechanicsNOSPD>();

/**
 * Kernel class for bond-associated correspondence material model for finite strain
 */
class FiniteStrainMechanicsNOSPD : public MechanicsBaseNOSPD
{
public:
  FiniteStrainMechanicsNOSPD(const InputParameters & parameters);

protected:
  virtual void computeLocalResidual() override;

  virtual void computeLocalJacobian() override;
  virtual void computeNonlocalJacobian() override;

  virtual void computeLocalOffDiagJacobian(unsigned int coupled_component) override;
  virtual void computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                                unsigned int coupled_component) override;
  virtual RankTwoTensor computeDSDU(unsigned int component, unsigned int nd) override;

  /**
   * Function to compute derivative of stress with respect to derived deformation gradient
   * @param nd   The local index of element node (either 1 or 2 for Edge2 element)
   * @return The calculated derivative
   */
  RankFourTensor computeDSDFhat(unsigned int nd);

  /**
   * Function to compute derivative of determinant of deformation gradient with respect to
   * displacements
   * @param component   The index of displacement component
   * @param nd   The local index of element node (either 1 or 2 for Edge2 element)
   * @return The calculated derivative
   */
  Real computeDJDU(unsigned int component, unsigned int nd);

  /**
   * Function to compute derivative of deformation gradient inverse with respect to displacements
   * @param component   The index of displacement component
   * @param nd   The local index of element node (either 1 or 2 for Edge2 element)
   * @return The calculated derivative
   */
  RankTwoTensor computeDinvFTDU(unsigned int component, unsigned int nd);

  ///@{ Material point based material property
  const MaterialProperty<RankTwoTensor> & _dgrad_old;
  const MaterialProperty<RankTwoTensor> & _E_inc;
  const MaterialProperty<RankTwoTensor> & _R_inc;
  ///@}

  /// The index of displacement component
  const unsigned int _component;
};
