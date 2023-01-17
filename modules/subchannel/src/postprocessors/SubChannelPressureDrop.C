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

#include "SubChannelPressureDrop.h"
#include "SolutionHandle.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "libmesh/system.h"

registerMooseObject("SubChannelApp", SubChannelPressureDrop);

InputParameters
SubChannelPressureDrop::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Calculates an overall mass-flow-averaged pressure drop for the subchannel assembly");
  return params;
}

SubChannelPressureDrop::SubChannelPressureDrop(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(dynamic_cast<SubChannelMesh &>(_fe_problem.mesh())),
    _value(0)
{
}

void
SubChannelPressureDrop::execute()
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
SubChannelPressureDrop::getValue()
{
  return _value;
}
