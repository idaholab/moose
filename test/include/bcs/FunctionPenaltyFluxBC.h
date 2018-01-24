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
#ifndef FUNCTIONPENALTYFLUXBC_H
#define FUNCTIONPENALTYFLUXBC_H

#include "IntegratedBC.h"

class FunctionPenaltyFluxBC;
class Function;

template <>
InputParameters validParams<FunctionPenaltyFluxBC>();

/**
 * Penalizes the difference between the current flux and desired flux,
 * similarly to penalty Dirichlet BCs. Implements the term:
 *
 * \int (p * (du/dn - dg/dn) * dv/dn) dx
 *
 * where p is the (large) penalty parameter, du/dn is the normal flux,
 * and dg/dn is the normal component of the gradient of the true solution.
 *
 * We allow the user to provide the components of the true flux, and
 * then compute g for them by dotting those components with the
 * outward unit normal. This class is designed to impose a given value
 * of the flux as an essential BC in the biharmonic problem.
 */
class FunctionPenaltyFluxBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  FunctionPenaltyFluxBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  Function & _func;
  Real _p;
};

#endif
