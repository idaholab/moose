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

#ifndef FUNCTIONSCALARIC_H
#define FUNCTIONSCALARIC_H

#include "ScalarInitialCondition.h"

// Forward Declarations
class FunctionScalarIC;
class Function;

template <>
InputParameters validParams<FunctionScalarIC>();

class FunctionScalarIC : public ScalarInitialCondition
{
public:
  FunctionScalarIC(const InputParameters & parameters);

protected:
  virtual Real value() override;

  unsigned int _ncomp;
  std::vector<Function *> _func;
};

#endif // FUNCTIONSCALARIC_H
