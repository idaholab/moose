//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVBoundaryCondition.h"

/**
 * Class implementing a Dirichlet boundary condition for linear finite
 * volume variables
 */
class LinearFVFunctorDirichletBC : public LinearFVBoundaryCondition
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVFunctorDirichletBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

protected:
  /// Compute the distance for the gradient approximation. We need this because
  /// the sideset associated within this boundary condition might be within the mesh.
  Real computeCellToFaceDistance() const;

  /// The functor for this BC (can be value, function, etc)
  const Moose::Functor<Real> & _functor;
};
