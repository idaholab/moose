//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubChannelPointValue.h"
#include "SolutionHandle.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "SCM.h"

registerMooseObject("SubChannelApp", SubChannelPointValue);

InputParameters
SubChannelPointValue::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<VariableName>("variable", "Variable you want the value of");
  params.addRequiredParam<Real>("height", "Axial location of point [m]");
  params.addRequiredParam<unsigned int>("index", "Index of subchannel");
  params.addClassDescription(
      "Prints out a user selected value of a specified subchannel at a user selected axial height");
  return params;
}

SubChannelPointValue::SubChannelPointValue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(SCM::getConstMesh<SubChannelMesh>(_fe_problem.mesh())),
    _height(getParam<Real>("height")),
    _i_ch(getParam<unsigned int>("index")),
    _variable(getParam<VariableName>("variable")),
    _value(0)
{
  // getNumOfChannels() is zero off rank 0, so every rank checks against the rank 0 count
  auto n_channels = _mesh.getNumOfChannels();
  _communicator.broadcast(n_channels);
  if (_i_ch >= n_channels)
    paramError("index", "is out of range; valid subchannel indices are 0 to ", n_channels - 1, ".");
  const auto & z_grid = _mesh.getZGrid();
  if (_height < z_grid.front() || _height > z_grid.back())
    paramError("height",
               "is outside the axial extent of the mesh, ",
               z_grid.front(),
               " to ",
               z_grid.back(),
               ".");
  std::tie(_iz, _weight) = _mesh.getAxialInterpolation(_height);
}

void
SubChannelPointValue::execute()
{
  // Creating a solution handle is collective, so it must happen on every processor
  const auto soln = SolutionHandle(_fe_problem.getVariable(0, _variable));
  // The subchannel solution is only available on processor 0
  if (processor_id() != 0)
    return;
  const auto value_in = soln(_mesh.getChannelNode(_i_ch, _iz));
  const auto value_out = soln(_mesh.getChannelNode(_i_ch, _iz + 1));
  _value = value_in + (value_out - value_in) * _weight;
}

void
SubChannelPointValue::finalize()
{
  _communicator.broadcast(_value);
}

Real
SubChannelPointValue::getValue() const
{
  return _value;
}
