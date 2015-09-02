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
#ifndef ADVALUETEST_H_
#define ADVALUETEST_H_

#include "ADKernel.h"

class ADValueTest : public ADKernel
{
public:
  ADValueTest(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();
};

template<>
InputParameters validParams<ADValueTest>();

#endif /* ADVALUETEST_H_ */
