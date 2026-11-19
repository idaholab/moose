//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMPinSurfaceTemperature.h"
#include "SolutionHandle.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "libmesh/system.h"
#include "SCM.h"

registerMooseObject("SubChannelApp", SCMPinSurfaceTemperature);

InputParameters
SCMPinSurfaceTemperature::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Returns the surface temperature of a specific fuel pin at a user defined height. Applies a "
      "linear reconstruction for the temperature.");
  params.addRequiredParam<Real>("height", "Axial location on fuel pin [m]");
  params.addRequiredParam<unsigned int>("index", "Index of fuel pin");
  return params;
}

SCMPinSurfaceTemperature::SCMPinSurfaceTemperature(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(SCM::getConstMesh<SubChannelMesh>(_fe_problem.mesh())),
    _height(getParam<Real>("height")),
    _i_pin(getParam<unsigned int>("index")),
    _value(0)
{
  if (!_mesh.pinMeshExist())
    mooseError(
        name(),
        " : The SCMPinSurfaceTemperature post processor calculates temperature on pins. A Pin "
        "Mesh should be defined.");
  // getNumOfPins() is zero off rank 0, so every rank checks against the rank 0 count
  auto n_pins = _mesh.getNumOfPins();
  _communicator.broadcast(n_pins);
  if (_i_pin >= n_pins)
    paramError("index", "is out of range; valid pin indices are 0 to ", n_pins - 1, ".");
  if (_height < _mesh.getZGrid().front())
    paramError("height", "is below the inlet of the mesh at ", _mesh.getZGrid().front(), ".");
  std::tie(_iz, _weight) = _mesh.getAxialInterpolation(_height);
}

void
SCMPinSurfaceTemperature::execute()
{
  // Creating a solution handle is collective, so it must happen on every processor
  const auto Tpin_soln = SolutionHandle(_fe_problem.getVariable(0, "Tpin"));
  // No data on other ranks
  if (processor_id() != 0)
    return;
  const auto T_in = Tpin_soln(_mesh.getPinNode(_i_pin, _iz));
  const auto T_out = Tpin_soln(_mesh.getPinNode(_i_pin, _iz + 1));
  _value = T_in + (T_out - T_in) * _weight;
}

void
SCMPinSurfaceTemperature::finalize()
{
  // The subchannel solution is only available on processor 0
  _communicator.broadcast(_value);
}

Real
SCMPinSurfaceTemperature::getValue() const
{
  return _value;
}
