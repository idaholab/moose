//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef FLUXLIMITEDTVDADVECTION_H
#define FLUXLIMITEDTVDADVECTION_H

#include "Kernel.h"
#include "FluxLimiter.h"

// Forward Declaration
class FluxLimitedTVDAdvection;

/**
 * Advection of the variable by the velocity provided by the user.
 * This implements the flux-limited TVD scheme detailed in
 * D Kuzmin and S Turek "High-resolution FEM-TVD shcemes based on a fully multidimensional flux
 * limiter" Journal of Computational Physics 198 (2004) 131-158
 */
template <>
InputParameters validParams<FluxLimitedTVDAdvection>();

class FluxLimitedTVDAdvection : public Kernel
{
public:
  FluxLimitedTVDAdvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual void computeResidual() override;

  /// advection velocity
  RealVectorValue _velocity;

  /// Nodal value of u
  const VariableValue & _u_nodal;

  /**
   * _flux_out[i] = mass of fluid moving out of node i into other nodes of this element
   * So _flux_out[i] is the contribution to the residual of node i from this element
   * (I could have used _local_re, but the name _flux_out is more descriptive for now)
   */
  DenseVector<Real> _flux_out;

  /// Do Kuzmin-Turek algorithm
  void tvd();

  const FluxLimiter & _fluo;
};

#endif // FLUXLIMITEDTVDADVECTION_H
