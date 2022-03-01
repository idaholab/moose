//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstantArray.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", GenericConstantArray);

InputParameters
GenericConstantArray::validParams()
{

  InputParameters params = Material::validParams();
  params.addRequiredParam<std::string>("prop_name",
                                       "The name of the property this material will have");
  params.addRequiredParam<RealEigenVector>("prop_value",
                                           "The values associated with the named property");
  params.declareControllable("prop_value");
  params.addClassDescription(
      "A material evaluating one material property in type of RealEigenVector");
  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";
  return params;
}

GenericConstantArray::GenericConstantArray(const InputParameters & parameters)
  : Material(parameters),
    _prop_name(getParam<std::string>("prop_name")),
    _prop_value(getParam<RealEigenVector>("prop_value")),
    _property(declareProperty<RealEigenVector>(_prop_name))
{
}

void
GenericConstantArray::initQpStatefulProperties()
{
  computeQpProperties();
}

void
GenericConstantArray::computeQpProperties()
{
  _property[_qp] = _prop_value;
}
