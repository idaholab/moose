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
 *
 * The switch reads a face flux functor rather than the velocity, so that a model whose transport
 * flux is not the Rhie-Chow one, such as the volume fraction transported with the frozen
 * lagged_corrected_face_phi, switches on the flux it is actually transported with. The default
 * is the corrected face flux of the pressure equation.
 */
class LinearFVInletOutletScalarBC : public LinearFVAdvectionDiffusionOutflowBC
{
public:
  static InputParameters validParams();

  LinearFVInletOutletScalarBC(const InputParameters & parameters);

  Real computeBoundaryValue() const override;
  /// The boundary value for a given flow direction, for a caller that has decided the direction
  /// itself, such as the volume-fraction corrector classifying its faces
  Real computeBoundaryValue(bool backflow) const;
  Real computeBoundaryNormalGradient() const override;
  Real computeBoundaryValueMatrixContribution() const override;
  Real computeBoundaryValueRHSContribution() const override;
  Real computeBoundaryGradientMatrixContribution() const override;
  Real computeBoundaryGradientRHSContribution() const override;

  bool includesMaterialPropertyMultiplier() const override { return !isBackflow(); }
  bool needsBoundaryNonorthogonalCorrection() const override { return isBackflow(); }

protected:
  /// Whether the face flux enters the domain through this face
  bool isBackflow() const;
  /// The value imposed on backflow; the prescribed functor here, a derived condition may form it
  /// from the cell values
  virtual Real computeBackflowBoundaryValue() const;
  /// The coefficient of the cell value in the backflow boundary value, so that a derived
  /// condition whose backflow value depends on the cell can keep that part implicit; zero for a
  /// prescribed value
  virtual Real computeBackflowBoundaryValueMatrixContribution() const { return 0.0; }

  /// The value brought in where the flow enters
  const Moose::Functor<Real> & _backflow_value;
  /// The face flux whose sign decides between outflow and backflow, per unit area along the
  /// face normal
  const Moose::Functor<Real> & _face_flux;
};
