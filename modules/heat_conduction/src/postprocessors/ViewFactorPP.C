//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ViewFactorPP.h"
#include "ViewFactorBase.h"

registerMooseObject("HeatConductionApp", ViewFactorPP);

InputParameters
ViewFactorPP::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>("view_factor_object_name",
                                          "Name of the ViewFactor userobjects.");
  params.addRequiredParam<BoundaryName>("from_boundary",
                                        "The boundary from which to compute the view factor.");
  params.addRequiredParam<BoundaryName>("to_boundary",
                                        "The boundary from which to compute the view factor.");
  params.addClassDescription(
      "This postprocessor allows to extract view factors from ViewFactor userobjects.");
  return params;
}

ViewFactorPP::ViewFactorPP(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _vf_uo(getUserObject<ViewFactorBase>("view_factor_object_name")),
    _from_bnd_id(_fe_problem.mesh().getBoundaryID(getParam<BoundaryName>("from_boundary"))),
    _to_bnd_id(_fe_problem.mesh().getBoundaryID(getParam<BoundaryName>("to_boundary")))
{
}

PostprocessorValue
ViewFactorPP::getValue()
{
  return _vf_uo.getViewFactor(_from_bnd_id, _to_bnd_id);
}
