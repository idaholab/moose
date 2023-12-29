//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticityUOBase.h"

InputParameters
CrystalPlasticityUOBase::validParams()
{
  InputParameters params = DiscreteElementUserObject::validParams();
  params.addRequiredParam<unsigned int>("variable_size", "The userobject's variable size.");
  params.addClassDescription("Crystal plasticity userobject system base class. Override the "
                             "virtual functions in your class");
  return params;
}

CrystalPlasticityUOBase::CrystalPlasticityUOBase(const InputParameters & parameters)
  : DiscreteElementUserObject(parameters), _variable_size(getParam<unsigned int>("variable_size"))
{
}

unsigned int
CrystalPlasticityUOBase::variableSize() const
{
  return _variable_size;
}
