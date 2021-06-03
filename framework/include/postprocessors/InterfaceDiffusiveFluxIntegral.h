//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "InterfaceIntegralPostprocessor.h"
#include "MooseVariableInterface.h"

#include <set>

// Forward Declarations
template <bool>
class InterfaceDiffusiveFluxIntegralTempl;
typedef InterfaceDiffusiveFluxIntegralTempl<false> InterfaceDiffusiveFluxIntegral;
typedef InterfaceDiffusiveFluxIntegralTempl<true> ADInterfaceDiffusiveFluxIntegral;
class FaceInfo;

/**
 * This postprocessor computes an integral of the diffusive flux over an interface.
 */
template <bool is_ad>
class InterfaceDiffusiveFluxIntegralTempl : public InterfaceIntegralPostprocessor
{
public:
  static InputParameters validParams();

  InterfaceDiffusiveFluxIntegralTempl(const InputParameters & parameters);
  void initialize() override;
  void execute() override;

protected:
  virtual Real computeQpIntegral() override;

  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;

  const VariableValue & _u;
  const VariableValue & _u_neighbor;

  /// Whether this postprocessor is being applied to finite volume variables
  const bool _fv;

  /// Material properties for the diffusion coefficient
  const GenericMaterialProperty<Real, is_ad> & _diffusion_coef;
  const GenericMaterialProperty<Real, is_ad> & _diffusion_coef_neighbor;

  /// A pointer to a face info, useful when working with FV
  const FaceInfo * _fi;

  std::unordered_set<const FaceInfo *> _face_infos_processed;
};
