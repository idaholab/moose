//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstant2DArray.h"

registerMooseObject("MooseApp", GenericConstant2DArray);

InputParameters
GenericConstant2DArray::validParams()
{

  InputParameters params = Material::validParams();
  params.addRequiredParam<std::string>("prop_name",
                                       "The names of the properties this material will have");
  params.addRequiredParam<RealEigenMatrix>("prop_value",
                                           "The values associated with the named properties");
  params.declareControllable("prop_value");
  params.addClassDescription(
      "A material evaluating one material property in type of RealEigenMatrix");
  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";
  return params;
}

GenericConstant2DArray::GenericConstant2DArray(const InputParameters & parameters)
  : Material(parameters),
    _prop_name(getParam<std::string>("prop_name")),
    _prop_value(getParam<RealEigenMatrix>("prop_value")),
    _property(declareProperty<RealEigenMatrix>(_prop_name))
{
}

void
GenericConstant2DArray::initQpStatefulProperties()
{
  computeQpProperties();
}

void
GenericConstant2DArray::computeQpProperties()
{
  _property[_qp] = _prop_value;
}
