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

class InterfaceQpValueUserObject;

template <>
InputParameters validParams<InterfaceQpValueUserObject>();

/**
 * This userobject collect values of a variable across an interface for each QP and compute a
 * scalar. The computed scalar value depends on the given parameter _interface_value_type\
 * _interface_value_type (see IntervafeValueTools).
 */
class InterfaceQpValueUserObject : public InterfaceQpUserObjectBase
{
public:
  static InputParameters validParams();
  InterfaceQpValueUserObject(const InputParameters & parameters);
  virtual ~InterfaceQpValueUserObject(){};

protected:
  virtual Real computeRealValueMaster(const unsigned int qp) override { return _u[qp]; };
  virtual Real computeRealValueSlave(const unsigned int qp) override { return _u_neighbor[qp]; };
  const VariableValue & _u;
  const VariableValue & _u_neighbor;
};
