//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapConductanceConstant.h"

registerMooseObject("HeatConductionApp", GapConductanceConstant);

InputParameters
GapConductanceConstant::validParams()
{
  InputParameters params = Material::validParams();
  params += GapConductanceConstant::actionParameters();
  // We can't just make it required in the first place because then it would always
  // be required in the Action, even if this model isn't used.
  params.makeParamRequired<Real>("gap_conductance");
  params.addClassDescription("Material to compute a constant, prescribed gap conductance");

  return params;
}

InputParameters
GapConductanceConstant::actionParameters()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::string>(
      "appended_property_name", "", "Name appended to material properties to make them unique");
  params.addParam<Real>("gap_conductance", 0.0, "Gap conductance");
  return params;
}

GapConductanceConstant::GapConductanceConstant(const InputParameters & params)
  : Material(params),
    _prescribed_gap_conductance(getParam<Real>("gap_conductance")),
    _appended_property_name(getParam<std::string>("appended_property_name")),
    _gap_conductance(declareProperty<Real>("gap_conductance" + _appended_property_name)),
    _gap_conductance_dT(declareProperty<Real>("gap_conductance" + _appended_property_name + "_dT"))
{
  if (!params.isParamSetByUser("gap_conductance"))
    mooseError("gap_conductance must be specified");
}

void
GapConductanceConstant::computeQpProperties()
{
  _gap_conductance[_qp] = _prescribed_gap_conductance;
  _gap_conductance_dT[_qp] = 0.0;
}
