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

#ifndef WEAKGRADIENTBC_H
#define WEAKGRADIENTBC_H

#include "IntegratedBC.h"

// Forward Declarations
class WeakGradientBC;

template <>
InputParameters validParams<WeakGradientBC>();

/**
 * A FluxBC which is consistent with the boundary terms arising from
 * the Diffusion Kernel. The residual contribution is:
 *
 * \f$ F(u) = - \int_{\Gamma} \nabla u * \hat n * \phi d\Gamma \f$
 *
 * This class is essentially identical to the DiffusionFluxBC, but it
 * is not a part of the FluxBC hierarchy. It does not actually impose
 * any boundary condition, instead it computes the residual
 * contribution due to the boundary term arising from integration by
 * parts of the Diffusion Kernel.
 */
class WeakGradientBC : public IntegratedBC
{
public:
  WeakGradientBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
};

#endif // WEAKGRADIENTBC_H
