//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowSinkBC.h"

registerMooseObject("PorousFlowApp", PorousFlowSinkBC);

InputParameters
PorousFlowSinkBC::validParamsCommon()
{
  InputParameters params = emptyInputParameters();
  params.addCoupledVar("porepressure_var", "Name of the porepressure variable");
  params.addParam<unsigned int>("fluid_phase",
                                "Evaluate enthalpy at the pressure of this fluid phase.");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addRequiredParam<Real>("T_in", "Specified inlet temperature (measured in K)");
  params.addRequiredParam<FunctionName>(
      "flux_function",
      "The flux.  The flux is OUT of the medium: hence positive values of "
      "this function means this BC will act as a SINK, while negative values "
      "indicate this flux will be a SOURCE. However, this BC only makes physical sense if "
      "flux_function <= 0.  This function is measured in kg.m^-2.s^-1.");
  return params;
}

InputParameters
PorousFlowSinkBC::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += PorousFlowSinkBC::validParamsCommon();
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where this boundary condition applies");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");

  params.addClassDescription(
      "BC corresponding to hot/cold fluid injection. This BC is only valid for "
      "single-phase, non-isothermal simulations using (P, T) variables. This BC adds fluid mass "
      "and heat energy. It is only meaningful if flux_function <= 0");

  params.registerBase("PorousFlowSinkBC");

  return params;
}

PorousFlowSinkBC::PorousFlowSinkBC(const InputParameters & parameters) : MooseObject(parameters) {}
