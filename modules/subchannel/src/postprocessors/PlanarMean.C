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

#include "PlanarMean.h"
#include "SolutionHandle.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "libmesh/system.h"

registerMooseObject("SubChannelApp", PlanarMean);

InputParameters
PlanarMean::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Calculates an overall mass-flow-averaged mean of the chosen "
                             "variable on a z-plane at a user defined height");
  params.addRequiredParam<AuxVariableName>("variable", "Variable you want the mean of");
  params.addRequiredParam<Real>("height", "Axial location of plane [m]");
  return params;
}

PlanarMean::PlanarMean(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(dynamic_cast<SubChannelMesh &>(_fe_problem.mesh())),
    _variable(getParam<AuxVariableName>("variable")),
    _height(getParam<Real>("height")),
    _mean_value(0)
{
}

void
PlanarMean::execute()
{
  auto nz = _mesh.getNumOfAxialCells();
  auto n_channels = _mesh.getNumOfChannels();
  auto Soln = SolutionHandle(_fe_problem.getVariable(0, _variable));
  auto mdot_soln = SolutionHandle(_fe_problem.getVariable(0, "mdot"));
  auto z_grid = _mesh.getZGrid();
  auto total_length =
      _mesh.getHeatedLength() + _mesh.getHeatedLengthEntry() + _mesh.getHeatedLengthExit();

  auto mass_flow = 0.0;
  auto sum_sol_mass_flow = 0.0;

  if (_height >= total_length)
  {
    for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
    {
      auto * node_out = _mesh.getChannelNode(i_ch, nz);
      mass_flow += mdot_soln(node_out);
      sum_sol_mass_flow += Soln(node_out) * mdot_soln(node_out);
    }
    _mean_value = sum_sol_mass_flow / mass_flow;
  }
  else
  {
    for (unsigned int iz = 0; iz < nz; iz++)
    {
      if (_height >= z_grid[iz] && _height < z_grid[iz + 1])
      {
        for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
        {
          auto * node_out = _mesh.getChannelNode(i_ch, iz + 1);
          auto * node_in = _mesh.getChannelNode(i_ch, iz);
          auto average_solution = Soln(node_in) + (Soln(node_out) - Soln(node_in)) *
                                                      (_height - z_grid[iz]) /
                                                      (z_grid[iz + 1] - z_grid[iz]);
          auto average_mass_flow = mdot_soln(node_in) + (mdot_soln(node_out) - mdot_soln(node_in)) *
                                                            (_height - z_grid[iz]) /
                                                            (z_grid[iz + 1] - z_grid[iz]);
          mass_flow += average_mass_flow;
          sum_sol_mass_flow += average_solution * average_mass_flow;
        }
        _mean_value = sum_sol_mass_flow / mass_flow;
        break;
      }
    }
  }
}

Real
PlanarMean::getValue() const
{
  return _mean_value;
}
