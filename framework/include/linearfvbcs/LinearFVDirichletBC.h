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
class LinearFVDirichletBC : public LinearFVBoundaryCondition
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   * @param nodal Whether this BC is applied to nodes or not
   */
  LinearFVDirichletBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryValue(const FaceInfo * const face_info) override;

  virtual Real computeBoundaryNormalGradient(const FaceInfo * const face_info) override;

  virtual Real
  computeBoundaryValueMatrixContribution(const FaceInfo * const face_info) const override;

  virtual Real computeBoundaryValueRHSContribution(const FaceInfo * const face_info) const override;

  virtual Real
  computeBoundaryGradientMatrixContribution(const FaceInfo * const face_info) const override;

  virtual Real
  computeBoundaryGradientRHSContribution(const FaceInfo * const face_info) const override;

protected:
  /// The functor for this BC (can be value, function, etc)
  const Moose::Functor<Real> & _functor;
};
