//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideFVFluxBCIntegral.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", SideFVFluxBCIntegral);


InputParameters
SideFVFluxBCIntegral::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredParam<std::vector<std::string>>("fvbcs");
  params.addClassDescription(
      "Computes the side integral of different finite volume flux boundary conditions.");
  return params;
}

SideFVFluxBCIntegral::SideFVFluxBCIntegral(
    const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
  _bc_names(getParam<std::vector<std::string>>("fvbcs"))
{
  _qp_integration = false;
}

void
SideFVFluxBCIntegral::initialSetup()
{
  SideIntegralPostprocessor::initialSetup();

  // BCs are constructed after the postprocessors so we shall do this
  // in the initialization step.
  auto base_query = _fe_problem.theWarehouse()
                        .query()
                        .condition<AttribSystem>("FVFluxBC")
                        .condition<AttribThread>(_tid);

  _bc_objects.clear();
  for (const auto & name : _bc_names)
  {
    std::vector<const FVFluxBC *> flux_bcs;
    base_query.condition<AttribName>(name).queryInto(flux_bcs);
    mooseAssert(flux_bcs.size() == 1,
                "There should be only one boundary condition from this search.");

    _bc_objects.push_back(flux_bcs[0]);
  }
}

Real
SideFVFluxBCIntegral::computeFaceInfoIntegral(const FaceInfo * const fi)
{
  return 0.0;
}

Real
SideFVFluxBCIntegral::computeQpIntegral()
{
  return 0.0;
}
