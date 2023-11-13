//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE
#include "LinearFVBoundaryCondition.h"

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

  virtual Real computeBoundaryValueMatrixContribution(const FaceInfo * const face_info) override;

  virtual Real computeBoundaryValueRHSContribution(const FaceInfo * const face_info) override;

  virtual Real computeBoundaryGradientMatrixContribution(const FaceInfo * const face_info) override;

  virtual Real computeBoundaryGradientRHSContribution(const FaceInfo * const face_info) override;

protected:
  const Real _value;
};
