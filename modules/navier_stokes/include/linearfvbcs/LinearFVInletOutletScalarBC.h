//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionOutflowBC.h"

/**
 * Linear-FV scalar inlet/outlet boundary condition.
 *
 * On outflow this behaves like a zero-gradient / extrapolated outlet.
 * On backflow it switches to a prescribed boundary value.
 */
class LinearFVInletOutletScalarBC : public LinearFVAdvectionDiffusionOutflowBC
{
public:
  static InputParameters validParams();

  LinearFVInletOutletScalarBC(const InputParameters & parameters);

  Real computeBoundaryValue() const override;
  Real computeBoundaryValue(bool backflow) const;
  Real computeBoundaryNormalGradient() const override;
  Real computeBoundaryValueMatrixContribution() const override;
  Real computeBoundaryValueRHSContribution() const override;
  Real computeBoundaryGradientMatrixContribution() const override;
  Real computeBoundaryGradientRHSContribution() const override;

  bool includesMaterialPropertyMultiplier() const override { return !isBackflow(); }
  bool useBoundaryGradientExtrapolation() const override { return isBackflow(); }

protected:
  bool isBackflow() const;
  Real outwardFaceFlux() const;
  const ElemInfo & fluidElemInfo() const;
  virtual Real computeBackflowBoundaryValue() const;
  virtual Real computeBackflowBoundaryValueMatrixContribution() const { return 0.0; }

  const Moose::Functor<Real> & _backflow_value;
  const Moose::Functor<Real> & _face_flux;
};
