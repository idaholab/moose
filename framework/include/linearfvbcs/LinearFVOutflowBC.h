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
 * Class implementing a outflow boundary condition for linear finite
 * volume variables
 */
class LinearFVOutflowBC : public LinearFVBoundaryCondition
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVOutflowBC(const InputParameters & parameters);

  virtual bool needsExtrapolation() const override { return _two_term_expansion; }

  static InputParameters validParams();

  virtual Real computeBoundaryValue() override;

  virtual Real computeBoundaryNormalGradient() override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

protected:
  RealVectorValue computeCellToFaceVector() const;
  const bool _two_term_expansion;
  const RealVectorValue _velocity;
};
