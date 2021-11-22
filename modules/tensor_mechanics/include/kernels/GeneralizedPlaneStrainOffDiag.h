//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"
#include "SubblockIndexProvider.h"
#include "ADRankTwoTensorForward.h"
#include "ADRankFourTensorForward.h"
#include "UserObject.h"

class GeneralizedPlaneStrainOffDiag : public DerivativeMaterialInterface<Kernel>
{
public:
  static InputParameters validParams();

  GeneralizedPlaneStrainOffDiag(const InputParameters & parameters);

protected:
  Real computeQpResidual() override { return 0; }

  /**
   * These methods are used to compute the off-diagonal jacobian for the coupling
   * between scalar variable strain_yy or strain_zz and nonlinear variables displacements and
   * temperature.
   * disp indicates the coupling is between displacements and strain_yy or strain_zz and
   * temp is for temperature and strain_yy or strain_zz
   */
  void computeOffDiagJacobianScalar(unsigned int jvar) override;
  virtual void computeDispOffDiagJacobianScalar(unsigned int component, unsigned int jvar);
  virtual void computeTempOffDiagJacobianScalar(unsigned int jvar);

  /// Base name of the material system that this kernel applies to
  const std::string _base_name;

  const MaterialProperty<RankFourTensor> & _Jacobian_mult;
  const std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const MaterialProperty<RankTwoTensor> *> _deigenstrain_dT;

  /// Variable number of the out-of-plane strain scalar variable
  unsigned int _scalar_out_of_plane_strain_var;

  /// A Userobject that carries the subblock ID for all elements
  const SubblockIndexProvider * const _subblock_id_provider;
  const unsigned int _scalar_var_id;

  MooseVariable * _temp_var;

  const unsigned int _num_disp_var;
  std::vector<MooseVariable *> _disp_var;

  /// The direction of the out-of-plane strain
  unsigned int _scalar_out_of_plane_strain_direction;
};
