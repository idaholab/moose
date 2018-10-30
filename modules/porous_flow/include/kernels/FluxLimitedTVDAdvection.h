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
#include "AdvectiveFluxCalculator.h"

// Forward Declaration
class FluxLimitedTVDAdvection;

/**
 * Advection of the variable with velocity set in the AdvectiveFluxCalculator
 *
 * This implements the flux-limited TVD scheme detailed in
 * D Kuzmin and S Turek "High-resolution FEM-TVD schemes based on a fully multidimensional flux
 * limiter" Journal of Computational Physics 198 (2004) 131-158
 *
 * Kuzmin and Turek's K_ij matrix, and their R+ and R- quantities are
 * computed by the AdvectiveFluxCalculator, and this Kernel adds the diffusion
 * and antidiffusion as specified by Kuzmin and Turek.
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

  /// The user object that computes Kuzmin and Turek's K_ij, and R+ and R- flux-limiting quantities
  const AdvectiveFluxCalculator & _fluo;
};

#endif // FLUXLIMITEDTVDADVECTION_H
