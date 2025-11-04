//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTHPowerPostprocessor.h"
#include "FEProblemBase.h"
#include "SolutionHandle.h"
#include "MooseMesh.h"
#include "SCM.h"
#include "SubChannel1PhaseProblem.h"

registerMooseObject("SubChannelApp", SCMTHPowerPostprocessor);

InputParameters
SCMTHPowerPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Calculates the total power of the subchannel assembly $[W]$ via the "
      "thermal-hydraulic enthalpy flow rate balance between inlet and outlet.");
  return params;
}

SCMTHPowerPostprocessor::SCMTHPowerPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(SCM::getConstMesh<SubChannelMesh>(_fe_problem.mesh())),
    _value(0)
{
}

void
SCMTHPowerPostprocessor::execute()
{
  const auto scm_problem = dynamic_cast<SubChannel1PhaseProblem *>(&_fe_problem);
  const auto n_channels = _mesh.getNumOfChannels();
  auto n_cells = _mesh.getNumOfAxialCells();
  auto mdot_soln = SolutionHandle(_fe_problem.getVariable(0, "mdot"));
  auto h_soln = SolutionHandle(_fe_problem.getVariable(0, "h"));

  if (!scm_problem)
    mooseError("SCMTHPowerPostprocessor can only be used within a subchannel 1phase problem.");

  Real power_in = 0.0;
  Real power_out = 0.0;

  for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
  {
    auto * node_in = _mesh.getChannelNode(i_ch, 0);
    auto * node_out = _mesh.getChannelNode(i_ch, n_cells);

    const Real mdot_in = mdot_soln(node_in);
    const Real h_in = h_soln(node_in);
    const Real mdot_out = mdot_soln(node_out);
    const Real h_out = h_soln(node_out);

    power_in += mdot_in * h_in;
    power_out += mdot_out * h_out;
  }

  _value = power_out - power_in;
}

Real
SCMTHPowerPostprocessor::getValue() const
{
  return _value;
}
