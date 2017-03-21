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
#ifndef FUNCTIONPENALTYDIRICHLETBC_H
#define FUNCTIONPENALTYDIRICHLETBC_H

#include "IntegratedBC.h"

class FunctionPenaltyDirichletBC;
class Function;

template <>
InputParameters validParams<FunctionPenaltyDirichletBC>();

/**
 * A different approach to applying Dirichlet BCs
 *
 * uses \f$ \int(p u \cdot \phi)=\int(p f \cdot \phi)\f$ on \f$d\omega\f$
 *
 */

class FunctionPenaltyDirichletBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  FunctionPenaltyDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  Function & _func;

  Real _p;
};

#endif
