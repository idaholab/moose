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

#ifndef ADDIFFUSION_H
#define ADDIFFUSION_H

#include "ADKernel.h"

class ADDiffusion;

template<>
InputParameters validParams<ADDiffusion>();


class ADDiffusion : public ADKernel
{
public:
  ADDiffusion(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();
};


#endif /* ADDIFFUSION_H */
