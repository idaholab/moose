//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "SCMBulkOutletTemperature.h"

#include "SCM.h"
#include "SinglePhaseFluidProperties.h"
#include "SolutionHandle.h"

registerMooseObject("SubChannelApp", SCMBulkOutletTemperature);

InputParameters
SCMBulkOutletTemperature::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<PostprocessorName>(
      "pressure", "Outlet pressure used for the enthalpy-to-temperature conversion [Pa]");
  params.addRequiredParam<UserObjectName>(
      "fp", "SinglePhaseFluidProperties object used by the subchannel problem");
  params.addClassDescription(
      "Calculates the bulk outlet temperature from mass-flow-weighted outlet enthalpy.");
  return params;
}

SCMBulkOutletTemperature::SCMBulkOutletTemperature(const InputParameters & params)
  : GeneralPostprocessor(params),
    _mesh(SCM::getConstMesh<SubChannelMesh>(_fe_problem.mesh())),
    _pressure(getPostprocessorValue("pressure")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp")),
    _value(0.0)
{
}

void
SCMBulkOutletTemperature::execute()
{
  const auto n_channels = _mesh.getNumOfChannels();
  const auto n_cells = _mesh.getNumOfAxialCells();
  auto mdot_soln = SolutionHandle(_fe_problem.getVariable(0, "mdot"));
  auto h_soln = SolutionHandle(_fe_problem.getVariable(0, "h"));

  Real mass_flow_out = 0.0;
  Real enthalpy_flow_out = 0.0;

  for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
  {
    auto * node_out = _mesh.getChannelNode(i_ch, n_cells);
    const Real mdot = mdot_soln(node_out);
    mass_flow_out += mdot;
    enthalpy_flow_out += mdot * h_soln(node_out);
  }

  if (mass_flow_out <= 0.0)
    mooseError("SCMBulkOutletTemperature requires a positive total outlet mass flow rate.");

  const Real h_bulk_out = enthalpy_flow_out / mass_flow_out;
  _value = _fp.T_from_p_h(_pressure, h_bulk_out);
}

Real
SCMBulkOutletTemperature::getValue() const
{
  return _value;
}
