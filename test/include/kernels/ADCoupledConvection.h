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
#ifndef ADCOUPLEDCONVECTION_H_
#define ADCOUPLEDCONVECTION_H_

#include "ADKernel.h"

class ADCoupledConvection;

template<>
InputParameters validParams<ADCoupledConvection>();

/**
 * Define the ADKernel for a convection operator that looks like:
 *
 * grad_some_var dot u'
 *
 */
class ADCoupledConvection : public ADKernel
{
public:
  ADCoupledConvection(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

private:
  ADVariableGradient & _velocity_vector;
};

#endif //ADCOUPLEDCONVECTION_H
