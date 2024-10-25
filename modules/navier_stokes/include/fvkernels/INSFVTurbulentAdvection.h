//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVAdvectionKernel.h"

/**
 * Computes the advection term with the assumption that the advected quantity will have special wall
 * treatments associated with turbulence.
 */
class INSFVTurbulentAdvection : public INSFVAdvectionKernel
{
public:
  static InputParameters validParams();
  virtual void initialSetup() override;
  INSFVTurbulentAdvection(const InputParameters & params);

  bool hasMaterialTimeDerivative() const override { return false; }

protected:
  ADReal computeQpResidual() override;
  using INSFVAdvectionKernel::computeResidual;
  void computeResidual(const FaceInfo & fi) override;
  using INSFVAdvectionKernel::computeJacobian;
  void computeJacobian(const FaceInfo & fi) override;

  // Density
  const Moose::Functor<ADReal> & _rho;

  /// Wall boundaries
  const std::vector<BoundaryName> & _wall_boundary_names;

  /// Maps for wall treatment
  std::map<const Elem *, bool> _wall_bounded;

  /// Whether to remove the derivative of this term wrt to velocity
  const bool _neglect_advection_derivatives;
};
