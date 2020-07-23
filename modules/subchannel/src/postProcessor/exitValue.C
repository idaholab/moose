#include "exitValue.h"

registerMooseObject("SubChannelApp", exitValue);

InputParameters
exitValue::validParams()
{
  InputParameters params = exitValue::validParams();
  params.addRequiredCoupledVar("value", "variable you want the value at the exit");
  params.addParam<std::string>("valueName", "name of the value";
  params.addClassDescription("Prints out a user selected value in csv format to be used for post-processing");
  return params;
}

exitValue::exitValue(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mesh(dynamic_cast<SubChannelMesh &>(_fe_problem.mesh())),
    _value(coupledValue("value")),
    _valueName(getParam<std::string>("valueName"))
{
  _exitValue.resize(_mesh._ny, _mesh._nx);
}

void
exitValue::print_exitValue()
{
  for (unsigned int i_ch = 0; i_ch < _mesh._n_channels; i_ch++)
  {
    auto * node_out = _mesh._nodes[i_ch][_mesh._nz];
    unsigned int i = (i_ch / _subchannel_mesh._nx);   // row
    unsigned int j = i_ch - i * _subchannel_mesh._nx; // column
    _exitValue(i, j) = _value(node_out);
  }

  string fullName = _valueName + "_out " +
                    ".txt"

                    std::ofstream myfile1;
  myfile1.open("fullName", std::ofstream::trunc);
  myfile1 << std::setprecision(3) << std::fixed << _exitValue << "\n";
  myfile1.close();
}
