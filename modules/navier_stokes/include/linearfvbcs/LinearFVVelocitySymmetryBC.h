//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionBC.h"
#include "NSFVUtils.h"

/**
 * Class implementing a symmetry boundary condition for the velocity variable.
 * The implementation is based on the method discussed in:
 * @book{greenshieldsweller2022,
 *  title     = "Notes on Computational Fluid Dynamics: General Principles",
 *  author    = "Greenshields, Christopher and Weller, Henry",
 *  year      = 2022,
 *  publisher = "CFD Direct Ltd",
 *  address   = "Reading, UK"
 * }
 */
class LinearFVVelocitySymmetryBC : public LinearFVAdvectionDiffusionBC
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVVelocitySymmetryBC(const InputParameters & parameters);

  /// Overriding all of these
  virtual Real computeBoundaryValue() const override;
  virtual Real computeBoundaryNormalGradient() const override;
  virtual Real computeBoundaryValueMatrixContribution() const override;
  virtual Real computeBoundaryValueRHSContribution() const override;
  virtual Real computeBoundaryGradientMatrixContribution() const override;
  virtual Real computeBoundaryGradientRHSContribution() const override;

protected:
  /// The dimension of the mesh
  const unsigned int _dim;

  /// Velocity variables by component
  const NS::LinearFVVelocityVariableArray _velocity_vars;

  /// Index x|y|z, this is mainly to handle the deviatoric parts correctly in
  /// in the stress term
  const unsigned int _index;
};
