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

#include "QuadSubChannelNormalSliceValues.h"
#include "SolutionHandle.h"
#include "FEProblemBase.h"

registerMooseObject("SubChannelApp", QuadSubChannelNormalSliceValues);

InputParameters
QuadSubChannelNormalSliceValues::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addRequiredParam<VariableName>("variable", "Variable you want the value of");
  params.addRequiredParam<Real>("height", "Axial location of normal slice [m]");
  params.addClassDescription("Prints out a user selected value at a user selected axial height in "
                             "a matrix format to be used for post-processing");
  return params;
}

QuadSubChannelNormalSliceValues::QuadSubChannelNormalSliceValues(const InputParameters & parameters)
  : FileOutput(parameters),
    _mesh(dynamic_cast<QuadSubChannelMesh &>(*_mesh_ptr)),
    _variable(getParam<VariableName>("variable")),
    _height(getParam<Real>("height"))
{
  _exit_value.resize(_mesh.getNy(), _mesh.getNx());
}

void
QuadSubChannelNormalSliceValues::output(const ExecFlagType & /*type*/)
{
  auto val_soln = SolutionHandle(_problem_ptr->getVariable(0, _variable));
  auto nz = _mesh.getNumOfAxialCells();
  auto z_grid = _mesh.getZGrid();
  auto n_channels = _mesh.getNumOfChannels();
  auto total_length =
      _mesh.getHeatedLength() + _mesh.getHeatedLengthEntry() + _mesh.getHeatedLengthExit();

  if (_height >= total_length)
  {
    for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
    {
      auto * node = _mesh.getChannelNode(i_ch, nz);
      unsigned int i = (i_ch / _mesh.getNx());   // row
      unsigned int j = i_ch - i * _mesh.getNx(); // column
      _exit_value(i, j) = val_soln(node);
    }
  }
  else
  {
    for (unsigned int iz = 0; iz < nz; iz++)
    {
      if (_height > z_grid[iz] && _height < z_grid[iz + 1])
      {
        for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
        {
          auto * node_out = _mesh.getChannelNode(i_ch, iz + 1);
          auto * node_in = _mesh.getChannelNode(i_ch, iz);
          unsigned int i = (i_ch / _mesh.getNx());   // row
          unsigned int j = i_ch - i * _mesh.getNx(); // column
          _exit_value(i, j) = val_soln(node_in) + (val_soln(node_out) - val_soln(node_in)) *
                                                      (_height - z_grid[iz]) /
                                                      (z_grid[iz + 1] - z_grid[iz]);
        }
        break;
      }
    }
  }

  std::ofstream myfile;
  myfile.open(_file_base, std::ofstream::trunc);
  myfile << std::setprecision(10) << std::fixed << _exit_value << "\n";
  myfile.close();
}
