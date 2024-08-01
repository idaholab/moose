//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/*
 *Computes the mixing length for the mixing length turbulence model.
 */
class kOmegaSSTF2BlendingAux : public AuxKernel
{
public:
  static InputParameters validParams();

  virtual void initialSetup() override;

  kOmegaSSTF2BlendingAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// The dimension of the domain
  const unsigned int _dim;

  /// Turbulent kinetic energy
  const Moose::Functor<ADReal> & _k;
  /// Turbulent kinetic energy dissipation rate
  const Moose::Functor<ADReal> & _omega;

  /// Density
  const Moose::Functor<ADReal> & _rho;
  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// Wall boundaries
  const std::vector<BoundaryName> & _wall_boundary_names;

  /// Map for wall distance
  std::map<const Elem *, Real> _dist;

  /// C-mu closure coefficient
  static constexpr Real _sigma_omega_2 = 1.168;
};
