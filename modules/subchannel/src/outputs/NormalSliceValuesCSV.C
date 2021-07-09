#include "NormalSliceValuesCSV.h"
#include "SolutionHandle.h"
#include "FEProblemBase.h"

registerMooseObject("SubChannelApp", NormalSliceValuesCSV);

InputParameters
NormalSliceValuesCSV::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addRequiredParam<VariableName>("variable", "Variable you want the value of");
  params.addRequiredParam<Real>("height", "Axial location of normal slice [m]");
  params.addClassDescription("Prints out a user selected value at a user selected axial height in "
                             "a CSV line format to be used for post-processing");
  return params;
}

NormalSliceValuesCSV::NormalSliceValuesCSV(const InputParameters & parameters)
  : FileOutput(parameters),
    _mesh(dynamic_cast<QuadSubChannelMesh &>(*_mesh_ptr)),
    _variable(getParam<VariableName>("variable")),
    _height(getParam<Real>("height"))
{
  _exit_value.resize(_mesh.getNy(), _mesh.getNx());
}

void
NormalSliceValuesCSV::output(const ExecFlagType & /*type*/)
{
  auto val_soln = SolutionHandle(_problem_ptr->getVariable(0, _variable));

  auto nz = _mesh.getNumOfAxialCells();
  auto z_grid = _mesh.getZGrid();
  auto n_channels = _mesh.getNumOfChannels();

  if (_height >= _mesh.getHeatedLength())
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
    for (unsigned int iz = 0; iz < nz + 1; iz++)
    {
      if (_height > z_grid[iz])
        ;
      else if (std::fabs(_height - z_grid[iz]) < 1e-6)
      {
        for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
        {
          auto * node = _mesh.getChannelNode(i_ch, iz);
          unsigned int i = (i_ch / _mesh.getNx());   // row
          unsigned int j = i_ch - i * _mesh.getNx(); // column
          _exit_value(i, j) = val_soln(node);
        }
        break;
      }
      else
      {
        for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
        {
          auto * node_out = _mesh.getChannelNode(i_ch, iz);
          auto * node_in = _mesh.getChannelNode(i_ch, iz - 1);
          unsigned int i = (i_ch / _mesh.getNx());   // row
          unsigned int j = i_ch - i * _mesh.getNx(); // column
          _exit_value(i, j) = val_soln(node_in) + (val_soln(node_out) - val_soln(node_in)) *
                                                      (_height - z_grid[iz - 1]) /
                                                      (z_grid[iz] - z_grid[iz - 1]);
        }
        break;
      }
    }
  }

  _exit_value.resize(1, _mesh.getNy() * _mesh.getNx());

  const static Eigen::IOFormat CSVFormat(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", "\n");
  std::ofstream myfile1;
  myfile1.open(_file_base, std::ofstream::trunc);
  myfile1 << std::setprecision(3) << std::fixed << _exit_value.format(CSVFormat) << "\n";
  myfile1.close();
}
