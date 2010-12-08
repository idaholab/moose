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

#include "BoundaryCondition.h"


//Forward Declarations
class WeakGradientBC;

template<>
InputParameters validParams<WeakGradientBC>();

/**
 * Implements a simple constant Neumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class WeakGradientBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  WeakGradientBC(const std::string & name, InputParameters parameters);
  
virtual ~WeakGradientBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  
private:
  /**
   * Value of grad(u) on the boundary.
   */
  Real _value;
};

#endif //WEAKGRADIENTBC_H
