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

#ifndef COUPLEDCONVECTION_H
#define COUPLEDCONVECTION_H

#include "Kernel.h"

class CoupledConvection;

template<>
InputParameters validParams<CoupledConvection>();

/**
 * Define the Kernel for a convection operator that looks like:
 *
 * grad_some_var dot u'
 * 
 */
class CoupledConvection : public Kernel
{
public:
  CoupledConvection(const std::string & name, MooseSystem &sys, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  VariableGradient & _velocity_vector;
};

#endif //COUPLEDCONVECTION_H
