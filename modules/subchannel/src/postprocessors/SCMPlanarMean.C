//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMPlanarMean.h"
#include "SolutionHandle.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "libmesh/system.h"
#include "SCM.h"

registerMooseObject("SubChannelApp", SCMPlanarMean);

InputParameters
SCMPlanarMean::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Calculates an mass-flow-rate averaged mean of the chosen "
                             "variable on a z-plane at a user defined height over all subchannels");
  params.addRequiredParam<AuxVariableName>("variable", "Variable you want the mean of");
  params.addRequiredParam<Real>("height", "Axial location of plane [m]");
  return params;
}

SCMPlanarMean::SCMPlanarMean(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(SCM::getConstMesh<SubChannelMesh>(_fe_problem.mesh())),
    _variable(getParam<AuxVariableName>("variable")),
    _height(getParam<Real>("height")),
    _mean_value(0)
{
  if (_height < _mesh.getZGrid().front())
    paramError("height", "is below the inlet of the mesh at ", _mesh.getZGrid().front(), ".");
  std::tie(_iz, _weight) = _mesh.getAxialInterpolation(_height);
}

void
SCMPlanarMean::execute()
{
  // Creating a solution handle is collective, so it must happen on every processor
  const auto Soln = SolutionHandle(_fe_problem.getVariable(0, _variable));
  const auto mdot_soln = SolutionHandle(_fe_problem.getVariable(0, "mdot"));
  // The subchannel solution is only available on processor 0
  if (processor_id() != 0)
    return;

  Real mass_flow = 0.0;
  Real sum_sol_mass_flow = 0.0;
  for (const auto i_ch : make_range(_mesh.getNumOfChannels()))
  {
    const auto * node_in = _mesh.getChannelNode(i_ch, _iz);
    const auto * node_out = _mesh.getChannelNode(i_ch, _iz + 1);
    const auto solution = Soln(node_in) + (Soln(node_out) - Soln(node_in)) * _weight;
    const auto mdot = mdot_soln(node_in) + (mdot_soln(node_out) - mdot_soln(node_in)) * _weight;
    mass_flow += mdot;
    sum_sol_mass_flow += solution * mdot;
  }
  _mean_value = sum_sol_mass_flow / mass_flow;
}

void
SCMPlanarMean::finalize()
{
  // The subchannel solution is only available on processor 0
  _communicator.broadcast(_mean_value);
}

Real
SCMPlanarMean::getValue() const
{
  return _mean_value;
}
