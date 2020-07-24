#include "NormalSliceValues.h"
#include "SolutionHandle.h"

registerMooseObject("SubChannelApp", NormalSliceValues);

InputParameters
NormalSliceValues::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredCoupledVar("value", "variable you want the value off at the exit");
  params.addParam<Real>("height", "Axial location of normal slice [m]");
  params.addParam<std::string>("file_name",
                               "name of the value used to printing the correct file name");
  params.addClassDescription("Prints out a user selected value at a user selected axial height in "
                             "a matrix format to be used for post-processing");
  return params;
}

NormalSliceValues::NormalSliceValues(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    Coupleable(this, true),
    _mesh(dynamic_cast<SubChannelMesh &>(_fe_problem.mesh())),
    _height(getParam<Real>("height")),
    _value(coupledValue("value")),
    _file_name(getParam<std::string>("file_name"))
{
  _exitValue.resize(_mesh._ny, _mesh._nx);
}

void
NormalSliceValues::initialize()
{
}
void
NormalSliceValues::finalize()
{
}

void
NormalSliceValues::execute()
{
  auto val_soln = SolutionHandle(*getFieldVar("value", 0));
  Node * node;

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

  std::string fullName = _file_name + "_out" + ".txt";

  std::ofstream myfile1;
  myfile1.open(fullName, std::ofstream::trunc);
  myfile1 << std::setprecision(3) << std::fixed << _exitValue << "\n";
  myfile1.close();
}
