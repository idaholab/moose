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
#ifndef FORCINGFN_H_
#define FORCINGFN_H_

#include "Kernel.h"

class ForcingFn : public Kernel
{
public:
  ForcingFn(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real funcValue();
};

template <>
InputParameters validParams<ForcingFn>();

#endif /* FORCINGFN_H_ */
