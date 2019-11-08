//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

// Forward Declarations
class Function;

/**
 * Implements a simple dirichlet BC for DG with material property
 *
 * BC derived from diffusion problem that can handle:
 * { \grad u * n_e} [v] + epsilon { \grad v * n_e } [u] + (sigma / |e| * [u][v])
 *
 *  [a] = [ a_1 - a_2 ]
 *  {a} = 0.5 * (a_1 + a_2)
 */
class DGMDDBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  DGMDDBC(const InputParameters & parameters);

  virtual ~DGMDDBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  const Function & _func;

  const MaterialProperty<Real> & _diff; // diffusivity

  Real _epsilon;
  Real _sigma;
};
