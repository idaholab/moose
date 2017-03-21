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

#ifndef DGDIFFUSION_H
#define DGDIFFUSION_H

#include "DGKernel.h"

// Forward Declarations
class DGDiffusion;

template <>
InputParameters validParams<DGDiffusion>();

/**
 * DG kernel for diffusion
 *
 * General DG kernel that this class can handle is:
 * \f$ { \nabla u * n_e} [v] + epsilon { \nabla v * n_e } [u] + (sigma / |e| * [u][v]) \f$
 *
 * \f$ [a] = [ a_1 - a_2 ] \f$
 * \f$ {a} = 0.5 * (a_1 + a_2) \f$
 *
 */
class DGDiffusion : public DGKernel
{
public:
  DGDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  Real _epsilon;
  Real _sigma;
  const MaterialProperty<Real> & _diff;
  const MaterialProperty<Real> & _diff_neighbor;
};

#endif
