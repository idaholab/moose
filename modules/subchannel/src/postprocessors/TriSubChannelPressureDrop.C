/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "TriSubChannelPressureDrop.h"
#include "SolutionHandle.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "libmesh/system.h"

registerMooseObject("SubChannelApp", TriSubChannelPressureDrop);

InputParameters
TriSubChannelPressureDrop::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Calculates an overall Total Pressure Drop for the hexagonal subchannel assembly");
  return params;
}

TriSubChannelPressureDrop::TriSubChannelPressureDrop(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(dynamic_cast<TriSubChannelMesh &>(_fe_problem.mesh())),
    _value(0)
{
}

void
TriSubChannelPressureDrop::execute()
{
  auto nz = _mesh.getNumOfAxialCells();
  auto n_channels = _mesh.getNumOfChannels();
  auto P_soln = SolutionHandle(_fe_problem.getVariable(0, "P"));
  auto mdot_soln = SolutionHandle(_fe_problem.getVariable(0, "mdot"));

  auto mass_flow_in = 0.0;
  auto sum_DP_mass_flow_in = 0.0;
  for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
  {
    auto * node_in = _mesh.getChannelNode(i_ch, 0);
    auto * node_out = _mesh.getChannelNode(i_ch, nz);
    mass_flow_in += mdot_soln(node_in);
    auto DP = P_soln(node_in) - P_soln(node_out);
    sum_DP_mass_flow_in += DP * mdot_soln(node_in);
  }

  _value = sum_DP_mass_flow_in / mass_flow_in;
}

Real
TriSubChannelPressureDrop::getValue()
{
  return _value;
}
