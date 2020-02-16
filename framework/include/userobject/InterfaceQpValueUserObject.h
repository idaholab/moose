//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceQpUserObjectBase.h"

/**
 * Specialization of InterfaceQpUserObjectBase for scalar variables.
 */
class InterfaceQpValueUserObject : public InterfaceQpUserObjectBase
{
public:
  static InputParameters validParams();
  InterfaceQpValueUserObject(const InputParameters & parameters);
  virtual ~InterfaceQpValueUserObject(){};

protected:
  virtual Real computeRealValue(const unsigned int qp) override;

  /// the variable and neighbor variable values or rate
  ///@{
  const VariableValue & _u;
  const VariableValue & _u_neighbor;
  ///@}
};
