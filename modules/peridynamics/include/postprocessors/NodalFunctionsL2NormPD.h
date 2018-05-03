//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALFUNCTIONSL2NORMPD_H
#define NODALFUNCTIONSL2NORMPD_H

#include "NodalIntegralPostprocessorBasePD.h"

class NodalFunctionsL2NormPD;
class Function;

template <>
InputParameters validParams<NodalFunctionsL2NormPD>();

/**
 * Postprocessor class to compute L2 norm of a given function for peridynamic discretization
 */
class NodalFunctionsL2NormPD : public NodalIntegralPostprocessorBasePD
{
public:
  NodalFunctionsL2NormPD(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeNodalValue() override;

  ///@{ Known functions
  const bool _has_func_0;
  Function * _func_0;

  const bool _has_func_1;
  Function * _func_1;

  const bool _has_func_2;
  Function * _func_2;
  ///@}
};

#endif // NODALFUNCTIONSL2NORMPD
