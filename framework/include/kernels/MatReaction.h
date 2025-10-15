//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * This kernel adds to the residual a contribution of \f$ -L*v \f$ where \f$ L \f$ is a material
 * property and \f$ v \f$ is a variable (nonlinear or coupled).
 */
template <bool is_ad>
class MatReactionTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  MatReactionTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual();

  /// Reaction rate
  const GenericMaterialProperty<Real, is_ad> & _rate;

  /**
   * Kernel variable (can be nonlinear or coupled variable)
   * (For constrained Allen-Cahn problems, v = lambda
   * where lambda is the Lagrange multiplier)
   */
  const GenericVariableValue<is_ad> & _v;

  usingGenericKernelMembers;
};

class MatReaction
  : public DerivativeMaterialInterface<JvarMapKernelInterface<MatReactionTempl<false>>>
{
public:
  static InputParameters validParams();

  MatReaction(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// is the kernel used in a coupled form?
  const bool _is_coupled;

  /**
   * Kernel variable (can be nonlinear or coupled variable)
   * (For constrained Allen-Cahn problems, v = lambda
   * where lambda is the Lagrange multiplier)
   */
  std::string _v_name;
  unsigned int _v_var;

  ///  Reaction rate derivative w.r.t. primal variable
  const MaterialProperty<Real> & _drate_du;

  ///  Reaction rate derivative w.r.t. the variable being added by this kernel
  const MaterialProperty<Real> & _drate_dv;

  ///  Reaction rate derivatives w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _drate_darg;
};

typedef MatReactionTempl<true> ADMatReaction;
