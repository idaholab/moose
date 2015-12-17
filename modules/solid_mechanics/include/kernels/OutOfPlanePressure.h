/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef OUTOFPLANEPRESSURE_H
#define OUTOFPLANEPRESSURE_H

#include "Kernel.h"

//Forward Declarations
class Function;
class OutOfPlanePressure;

template<>
InputParameters validParams<OutOfPlanePressure>();

class OutOfPlanePressure : public Kernel
{
public:

  OutOfPlanePressure(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

private:
  const PostprocessorValue * const _postprocessor;
  Function * const _function;
  const Real _factor;
};
#endif //OUTOFPLANEPRESSURE_H
