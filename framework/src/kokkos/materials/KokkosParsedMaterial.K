//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosParsedMaterial.h"

registerKokkosMaterial("MooseApp", KokkosParsedMaterial);

InputParameters
KokkosParsedMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params += ParsedObjectBase::validParams();

  params.addRequiredParam<MaterialPropertyName>("property_name",
                                                "Name of the parsed material property");
  params.addParam<bool>(
      "use_dt", false, "Make  time step (dt) variable available in the function expression.");
  params.addClassDescription("Evaluates a property with a parsed expression.");

  return params;
}

KokkosParsedMaterial::KokkosParsedMaterial(const InputParameters & parameters)
  : Material(parameters),
    ParsedObjectBase(this),
    _prop(declareKokkosProperty<Real>("property_name"))
{
  if (getParam<bool>("use_dt"))
  {
    checkDuplicateSymbols(std::vector<std::string>{"dt"}, "use_dt");

    auto & dt = *_dt;
    addScalar("dt", &dt);
  }

  initParsed(this, coupledNames("coupled_variables"));
}
