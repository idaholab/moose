//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef OPTIONALLYCOUPLEDFORCE2_H
#define OPTIONALLYCOUPLEDFORCE2_H

#include "Kernel.h"

// Forward Declaration
class OptionallyCoupledForce2;

template <>
InputParameters validParams<OptionallyCoupledForce2>();

class OptionallyCoupledForce2 : public Kernel
{
public:
  OptionallyCoupledForce2(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  std::vector<unsigned int> _v_var;
  std::vector<const VariableValue *> _v;
};

#endif // OptionallyCoupledForce2_H
