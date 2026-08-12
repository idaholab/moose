//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionFunctorNeumannBC.h"

/**
 * A Neumann boundary condition for linear finite volume anisotropic diffusion problems.
 *
 * The prescribed functor is the complete tensor-weighted boundary flux. The diagonal diffusion
 * tensor is used to reconstruct the boundary-normal gradient and boundary value.
 */
class LinearFVAnisotropicDiffusionFunctorNeumannBC
  : public LinearFVAdvectionDiffusionFunctorNeumannBC
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVAnisotropicDiffusionFunctorNeumannBC(const InputParameters & parameters);

  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

protected:
  /// Whether to reconstruct the boundary value with the prescribed flux and cell gradient
  const bool _two_term_expansion;

  /// The functor for the diagonal diffusion tensor
  const Moose::Functor<RealVectorValue> & _diffusion_tensor;
};
