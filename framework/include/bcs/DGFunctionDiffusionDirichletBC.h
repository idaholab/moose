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

/**
 * Implements a simple BC for DG
 *
 * BC derived from diffusion problem that can handle:
 * \f$ { \nabla u \cdot n_e} [v] + \epsilon { \nabla v \cdot n_e } [u] + (\frac{\sigma}{|e|} \cdot
 * [u][v]) \f$
 *
 * \f$ [a] = [ a_1 - a_2 ] \f$
 * \f$ {a} = 0.5 * (a_1 + a_2) \f$
 */
class DGFunctionDiffusionDirichletBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  static InputParameters validParams();

  DGFunctionDiffusionDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  const Function & _func;

  Real _epsilon;
  Real _sigma;
  const MaterialProperty<Real> & _diff;
};
