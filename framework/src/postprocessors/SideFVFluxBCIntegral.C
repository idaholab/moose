//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideFVFluxBCIntegral.h"
#include "FVFluxBC.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", SideFVFluxBCIntegral);

InputParameters
SideFVFluxBCIntegral::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredParam<std::vector<std::string>>(
      "fvbcs", "List of boundary conditions whose contribution we want to integrate.");
  params.addClassDescription(
      "Computes the side integral of different finite volume flux boundary conditions.");
  return params;
}

SideFVFluxBCIntegral::SideFVFluxBCIntegral(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters), _bc_names(getParam<std::vector<std::string>>("fvbcs"))
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

  // Fetch the mentioned boundary conditions
  _bc_objects.clear();
  for (const auto & name : _bc_names)
  {
    std::vector<FVFluxBC *> flux_bcs;
    base_query.condition<AttribName>(name).queryInto(flux_bcs);
    if (flux_bcs.size() == 0)
      paramError("fvbcs",
                 "The given FVFluxBC with name '",
                 name,
                 "' was not found! This can be due to the boundary condition not existing in the "
                 "'FVBCs' block or the boundary condition not inheriting from FVFluxBC.");

    // We expect the code to error at an earlier stage if there are more than one
    // BCs with the same name
    _bc_objects.push_back(flux_bcs[0]);
  }

  // Check if the boundary restriction of the objects is the same.
  for (const auto bc_ptr : _bc_objects)
    // Comparing ordered sets
    if (this->boundaryIDs() != bc_ptr->boundaryIDs())
      paramError("fvbcs",
                 "The given boundary condition with name ",
                 bc_ptr->name(),
                 " does not have the same boundary restriction as this postprocessor!");
}

Real
SideFVFluxBCIntegral::computeFaceInfoIntegral(const FaceInfo * const fi)
{
  Real flux_value = 0.0;
  for (auto bc_ptr : _bc_objects)
  {
    bc_ptr->updateCurrentFace(*fi);
    flux_value += MetaPhysicL::raw_value(bc_ptr->computeQpResidual());
  }
  return flux_value;
}

Real
SideFVFluxBCIntegral::computeQpIntegral()
{
  mooseError("We should never call this function!");
}
