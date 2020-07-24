#include "NormalSliceValuesCSV.h"
#include "SolutionHandle.h"

registerMooseObject("SubChannelApp", NormalSliceValuesCSV);

InputParameters
NormalSliceValuesCSV::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addRequiredParam<VariableName>("variable", "variable you want the value off at the exit");
  params.addRequiredParam<Real>("height", "Axial location of normal slice [m]");
  params.addClassDescription("Prints out a user selected value at a user selected axial height in "
                             "a csv line format to be used for post-processing");
  return params;
}

NormalSliceValuesCSV::NormalSliceValuesCSV(const InputParameters & parameters)
  : FileOutput(parameters),
    _mesh(dynamic_cast<SubChannelMesh &>(*_mesh_ptr)),
    _variable(getParam<VariableName>("variable")),
    _height(getParam<Real>("height"))
{
  _exitValue.resize(_mesh._ny, _mesh._nx);
}

void
NormalSliceValuesCSV::output(const ExecFlagType & /*type*/)
{
  auto val_soln = SolutionHandle(_problem_ptr->getVariable(0, _variable));

  if (_height >= _mesh._heated_length)
  {
    for (unsigned int i_ch = 0; i_ch < _mesh._n_channels; i_ch++)
    {
      auto * node = _mesh._nodes[i_ch][_mesh._nz];
      unsigned int i = (i_ch / _mesh._nx);   // row
      unsigned int j = i_ch - i * _mesh._nx; // column
      _exitValue(i, j) = val_soln(node);
    }
  }
  else
  {
    for (unsigned int iz = 1; iz < _mesh._nz + 1; iz++)
    {
      if (_height > _mesh._z_grid[iz])
        continue;
      else if (_height == _mesh._z_grid[iz])
      {
        for (unsigned int i_ch = 0; i_ch < _mesh._n_channels; i_ch++)
        {
          auto * node = _mesh._nodes[i_ch][iz];
          unsigned int i = (i_ch / _mesh._nx);   // row
          unsigned int j = i_ch - i * _mesh._nx; // column
          _exitValue(i, j) = val_soln(node);
        }
      }
      else
      {
        for (unsigned int i_ch = 0; i_ch < _mesh._n_channels; i_ch++)
        {
          auto * node_out = _mesh._nodes[i_ch][iz];
          auto * node_in = _mesh._nodes[i_ch][iz - 1];
          unsigned int i = (i_ch / _mesh._nx);   // row
          unsigned int j = i_ch - i * _mesh._nx; // column
          _exitValue(i, j) = val_soln(node_in) + (val_soln(node_out) - val_soln(node_in)) *
                                                     (_height - _mesh._z_grid[iz - 1]) /
                                                     (_mesh._z_grid[iz] - _mesh._z_grid[iz - 1]);
        }
      }
    }
  }

  _exitValue.resize(1, _mesh._ny * _mesh._nx);

  const static Eigen::IOFormat CSVFormat(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", "\n");

  std::ofstream myfile1;
  myfile1.open(_file_base, std::ofstream::trunc);
  myfile1 << std::setprecision(3) << std::fixed << _exitValue.format(CSVFormat) << "\n";
  myfile1.close();
}
