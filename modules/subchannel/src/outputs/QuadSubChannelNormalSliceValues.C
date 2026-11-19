//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadSubChannelNormalSliceValues.h"
#include "SolutionHandle.h"
#include "FEProblemBase.h"
#include "SCM.h"

registerMooseObject("SubChannelApp", QuadSubChannelNormalSliceValues);

InputParameters
QuadSubChannelNormalSliceValues::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addRequiredParam<VariableName>("variable", "Variable you want the value of");
  params.addRequiredParam<Real>("height", "Axial location of normal slice [m]");
  params.addClassDescription("Prints out a user selected value at a user selected axial height in "
                             "a matrix format to be used for post-processing");
  params.suppressParameter<bool>("append_date");
  params.suppressParameter<std::string>("append_date_format");
  params.suppressParameter<unsigned int>("padding");
  params.suppressParameter<std::vector<std::string>>("output_if_base_contains");
  return params;
}

QuadSubChannelNormalSliceValues::QuadSubChannelNormalSliceValues(const InputParameters & parameters)
  : FileOutput(parameters),
    _mesh(SCM::getConstMesh<QuadSubChannelMesh>(*_mesh_ptr)),
    _variable(getParam<VariableName>("variable")),
    _height(getParam<Real>("height"))
{
  if (_height < _mesh.getZGrid().front())
    paramError("height", "is below the inlet of the mesh at ", _mesh.getZGrid().front(), ".");
  std::tie(_iz, _weight) = _mesh.getAxialInterpolation(_height);
  _exit_value.resize(_mesh.getNy(), _mesh.getNx());
}

void
QuadSubChannelNormalSliceValues::output()
{
  // Accessing new solution handles should be done in parallel
  auto val_soln = SolutionHandle(_problem_ptr->getVariable(0, _variable));
  if (processor_id() > 0)
    return;
  const auto nx = _mesh.getNx();
  for (const auto i_ch : make_range(_mesh.getNumOfChannels()))
  {
    const auto * node_in = _mesh.getChannelNode(i_ch, _iz);
    const auto * node_out = _mesh.getChannelNode(i_ch, _iz + 1);
    const unsigned int i = i_ch / nx;     // row
    const unsigned int j = i_ch - i * nx; // column
    _exit_value(i, j) = val_soln(node_in) + (val_soln(node_out) - val_soln(node_in)) * _weight;
  }

  std::ofstream myfile;
  myfile.open(_file_base, std::ofstream::trunc);
  myfile << std::setprecision(10) << std::fixed << _exit_value << "\n";
  myfile.close();
}
