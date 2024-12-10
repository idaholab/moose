//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubChannelDelta.h"
#include "SolutionHandle.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "libmesh/system.h"

registerMooseObject("SubChannelApp", SubChannelDelta);

InputParameters
SubChannelDelta::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Calculates an absolute overall mass-flow-rate averaged difference, of a chosen "
      "variable, for the whole subchannel assembly, from inlet to outlet");
  params.addRequiredParam<AuxVariableName>("variable", "Variable you want the delta of");
  return params;
}

SubChannelDelta::SubChannelDelta(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(libMesh::cast_ref<SubChannelMesh &>(_fe_problem.mesh())),
    _variable(getParam<AuxVariableName>("variable")),
    _value(0)
{
}

void
SubChannelDelta::execute()
{
  auto nz = _mesh.getNumOfAxialCells();
  auto n_channels = _mesh.getNumOfChannels();
  auto Soln = SolutionHandle(_fe_problem.getVariable(0, _variable));
  auto mdot_soln = SolutionHandle(_fe_problem.getVariable(0, "mdot"));

  auto mass_flow_in = 0.0;
  auto sum_Delta_mass_flow_in = 0.0;
  for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
  {
    auto * node_in = _mesh.getChannelNode(i_ch, 0);
    auto * node_out = _mesh.getChannelNode(i_ch, nz);
    mass_flow_in += mdot_soln(node_in);
    auto Delta = abs(Soln(node_in) - Soln(node_out));
    sum_Delta_mass_flow_in += Delta * mdot_soln(node_in);
  }

  _value = sum_Delta_mass_flow_in / mass_flow_in;
}

Real
SubChannelDelta::getValue() const
{
  return _value;
}
