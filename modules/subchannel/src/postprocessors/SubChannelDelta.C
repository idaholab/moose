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
  params.addClassDescription("Calculates an overall mass-flow-averaged Delta of the chosen "
                             "variable for the subchannel assembly");
  params.addRequiredParam<AuxVariableName>("variable", "Variable you want the delta of");
  return params;
}

SubChannelDelta::SubChannelDelta(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(dynamic_cast<SubChannelMesh &>(_fe_problem.mesh())),
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
