//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TOTALDISPLACEMENTAUX_H
#define TOTALDISPLACEMENTAUX_H

#include "AuxKernel.h"

// Forward Declarations
class TotalDisplacementAux;

template <>
InputParameters validParams<TotalDisplacementAux>();

class TotalDisplacementAux : public AuxKernel
{
public:
  TotalDisplacementAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  const unsigned int _var_num;
  std::vector<const VariableValue *> _disp;
};
#endif // TOTALDISPLACEMENTAUX_H
