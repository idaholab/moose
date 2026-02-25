//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class INSFVVelocityVariable;

/*
 * Computes the value of the eddy viscosity for the mixing length model.
 */
class LinearWCNSFVSmagorinskyTurbulentViscosityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  LinearWCNSFVSmagorinskyTurbulentViscosityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// the dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  MooseLinearVariableFVReal * const _u_var;
  /// y-velocity
  MooseLinearVariableFVReal * const _v_var;
  /// z-velocity
  MooseLinearVariableFVReal * const _w_var;

  /// Density
  const Moose::Functor<Real> & _rho;

  /// Smagorinsky constant
  const Real _Cs;
};
