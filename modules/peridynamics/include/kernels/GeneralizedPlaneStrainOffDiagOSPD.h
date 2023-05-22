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
 * Kernel class for coupled off diagonal Jacobian entries of ordinary state-based peridynamic
 * generalized plane strain model
 */
class GeneralizedPlaneStrainOffDiagOSPD : public MechanicsBasePD
{
public:
  static InputParameters validParams();

  GeneralizedPlaneStrainOffDiagOSPD(const InputParameters & parameters);

protected:
  virtual void computeLocalResidual() override{};
  virtual void computeOffDiagJacobianScalar(unsigned int jvar_num) override;

  /**
   * Function to compute the full off diagonal Jacobian for coupling between displacements and
   * scalar variable
   * @param component   The index of displacement component
   * @param jvar_num   The coupled scalar variable number
   */
  void computeDispFullOffDiagJacobianScalar(unsigned int component, unsigned int jvar_num);

  /**
   * Function to compute partial off diagonal Jacobian for coupling between displacements and scalar
   * variable
   * @param component   The index of displacement component
   * @param jvar_num   The coupled scalar variable number
   */
  void computeDispPartialOffDiagJacobianScalar(unsigned int component, unsigned int jvar_num);

  /**
   * Function to compute off disgonal Jacobian for coupling between temperature and scalar variable
   * @param jvar_num   The coupled scalar variable number
   */
  void computeTempOffDiagJacobianScalar(unsigned int jvar_num);

  ///@{ Bond based material properties
  const MaterialProperty<Real> & _bond_local_dfdE;
  const MaterialProperty<Real> & _bond_nonlocal_dfdE;
  const MaterialProperty<Real> & _alpha;
  ///@}

  /// Material point based material property
  const MaterialProperty<RankFourTensor> & _Cijkl;

  /// The variable number of the scalar out-of-plane strain variable
  const unsigned int _scalar_out_of_plane_strain_var_num;

  DenseMatrix<Number> _ken;
  DenseMatrix<Number> _kne;
};
