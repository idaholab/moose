/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef DGFUNCTIONDIFFUSIONDIRICHLETBC_H
#define DGFUNCTIONDIFFUSIONDIRICHLETBC_H

#include "IntegratedBC.h"

// Forward Declarations
class DGFunctionDiffusionDirichletBC;

template <>
InputParameters validParams<DGFunctionDiffusionDirichletBC>();

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
  DGFunctionDiffusionDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  Function & _func;

  Real _epsilon;
  Real _sigma;
  const MaterialProperty<Real> & _diff;
};

#endif // DGFUNCTIONDIFFUSIONDIRICHLETBC_H
