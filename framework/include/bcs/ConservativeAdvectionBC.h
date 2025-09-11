//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericIntegratedBC.h"

class Function;

/**
 * A boundary condition for when the advection term is integrated by parts. This can be used at both
 * inlet and outlet boundaries for imposing both "explicit" (e.g. Dirichlet) and "implicit" (use
 * interior information) boundary conditions
 */
template <bool is_ad>
class ConservativeAdvectionBCTempl : public GenericIntegratedBC<is_ad>
{
public:
  static InputParameters validParams();

  ConservativeAdvectionBCTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  /// The velocity as a material property
  const GenericMaterialProperty<RealVectorValue, is_ad> * const _velocity_mat_prop;

  /// The velocity as a function
  const Function * const _velocity_function;

  /// The advected quantity
  const MooseArray<GenericReal<is_ad>> & _adv_quant;

  /// Dirichlet value for the primal variable
  const Function * const _primal_dirichlet;

  /// Coefficient for multiplying the primal Dirichlet value
  const GenericMaterialProperty<Real, is_ad> & _primal_coeff;

  usingGenericIntegratedBCMembers;
};

typedef ConservativeAdvectionBCTempl<false> ConservativeAdvectionBC;
typedef ConservativeAdvectionBCTempl<true> ADConservativeAdvectionBC;
