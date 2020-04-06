//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiscreteElementUserObject.h"

/**
 * Crystal plasticity system userobject base class.
 */
class CrystalPlasticityUOBase : public DiscreteElementUserObject
{
public:
  static InputParameters validParams();

  CrystalPlasticityUOBase(const InputParameters & parameters);

  /// Returns the size of variable
  virtual unsigned int variableSize() const;

protected:
  unsigned int _variable_size;
};
