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

#ifndef DIVERGENCEBC_H
#define DIVERGENCEBC_H

#include "IntegratedBC.h"


class DivergenceBC;

template<>
InputParameters validParams<DivergenceBC>();

/**
 * This provides the term that you get from using the Gauss Divergence Theorem on the second
 * order term in a Laplace/Poisson equation.
 */
class DivergenceBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  DivergenceBC(const std::string & name, InputParameters parameters);


protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};


#endif //DIVERGENCEBC_H
