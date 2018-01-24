/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrystalPlasticityUOBase.h"

template <>
InputParameters
validParams<CrystalPlasticityUOBase>()
{
  InputParameters params = validParams<DiscreteElementUserObject>();
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
