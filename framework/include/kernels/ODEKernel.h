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

#ifndef ODEKERNEL_H
#define ODEKERNEL_H

#include "ScalarKernel.h"

//Forward Declarations
class ODEKernel;

template<>
InputParameters validParams<ODEKernel>();

/**
 *
 */
class ODEKernel : public ScalarKernel
{
public:
  ODEKernel(const std::string & name, InputParameters parameters);
  virtual ~ODEKernel();

  virtual void reinit();
  virtual void computeResidual();
  virtual void computeJacobian();

protected:
  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
};


#endif /* ODEKERNEL_H */
