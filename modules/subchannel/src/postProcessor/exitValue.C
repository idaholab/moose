#include "exitValue.h"
#include "SolutionHandle.h"

registerMooseObject("SubChannelApp", exitValue);

InputParameters
exitValue::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredCoupledVar("value", "variable you want the value at the exit");
  params.addParam<std::string>("file_name",
                               "name of the value used to printing in the correct file name");
  params.addClassDescription(
      "Prints out a user selected value in a matrix format to be used for post-processing");
  return params;
}

exitValue::exitValue(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    Coupleable(this, true),
    _mesh(dynamic_cast<SubChannelMesh &>(_fe_problem.mesh())),
    _value(coupledValue("value")),
    _file_name(getParam<std::string>("file_name"))
{
  _exitValue.resize(_mesh._ny, _mesh._nx);
}

void
exitValue::initialize()
{
}
void
exitValue::finalize()
{
}

void
exitValue::execute()
{
  auto val_soln = SolutionHandle(*getFieldVar("value", 0));

  for (unsigned int i_ch = 0; i_ch < _mesh._n_channels; i_ch++)
  {
    auto * node_out = _mesh._nodes[i_ch][_mesh._nz];
    unsigned int i = (i_ch / _mesh._nx);   // row
    unsigned int j = i_ch - i * _mesh._nx; // column
    _exitValue(i, j) = val_soln(node_out);
  }

  std::string fullName = _file_name + "_out" + ".txt";

  std::ofstream myfile1;
  myfile1.open(fullName, std::ofstream::trunc);
  myfile1 << std::setprecision(3) << std::fixed << _exitValue << "\n";
  myfile1.close();
}
