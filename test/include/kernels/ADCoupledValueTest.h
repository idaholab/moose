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
#ifndef ADCOUPLEDVALUETEST_H_
#define ADCOUPLEDVALUETEST_H_

#include "ADKernel.h"

class ADCoupledValueTest : public ADKernel
{
public:
  ADCoupledValueTest(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  ADVariableValue & _v;
};

template<>
InputParameters validParams<ADCoupledValueTest>();

#endif /* ADCOUPLEDVALUETEST_H_ */
