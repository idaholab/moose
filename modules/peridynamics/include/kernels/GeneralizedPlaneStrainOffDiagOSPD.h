//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GENERALIZEDPLANESTRAINOFFDIAGOSPD_H
#define GENERALIZEDPLANESTRAINOFFDIAGOSPD_H

#include "MechanicsBasePD.h"

class GeneralizedPlaneStrainOffDiagOSPD;
class RankFourTensor;

template <>
InputParameters validParams<GeneralizedPlaneStrainOffDiagOSPD>();

/**
 * Kernel class for coupled off diagonal jacobian entries of ordinary state-based peridynamic
 * generalized plane strain model
 */
class GeneralizedPlaneStrainOffDiagOSPD : public MechanicsBasePD
{
public:
  GeneralizedPlaneStrainOffDiagOSPD(const InputParameters & parameters);

protected:
  virtual void computeLocalResidual() override{};
  virtual void computeOffDiagJacobianScalar(unsigned int jvar_num) override;

  /**
   * Function to compute the full off diagonal jacobian for coupling between displacements and
   * scalar variable
   */
  void computeDispFullOffDiagJacobianScalar(unsigned int component, unsigned int jvar_num);

  /**
   * Function to compute partial off diagonal jacobian for coupling between displacements and scalar
   * variable
   */
  void computeDispPartialOffDiagJacobianScalar(unsigned int component, unsigned int jvar_num);

  /**
   * Function to compute off disgonal jacobian for coupling between temperature and scalar variable
   */
  void computeTempOffDiagJacobianScalar(unsigned int jvar_num);

  ///@{ Bond based material properties
  const MaterialProperty<Real> & _bond_dfdE_ij;
  const MaterialProperty<Real> & _bond_dfdE_i_j;
  const MaterialProperty<Real> & _alpha;
  ///@}

  /// Material point based material property
  const MaterialProperty<RankFourTensor> & _Cijkl;

  const unsigned int _scalar_out_of_plane_strain_var_num;
};

#endif // GENERALIZEDPLANESTRAINOFFDIAGOSPD_H
