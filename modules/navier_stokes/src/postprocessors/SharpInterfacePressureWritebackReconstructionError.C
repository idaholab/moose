//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "SharpInterfacePressureWritebackReconstructionError.h"

#include "Assembly.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", SharpInterfacePressureWritebackReconstructionError);

InputParameters
SharpInterfacePressureWritebackReconstructionError::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<RealVectorValue>(
      "constant_vector",
      "The constant vector to recover from the sharp-interface pressure writeback face storage.");
  params.addClassDescription(
      "Computes the face-to-cell reconstruction error for the sharp-interface pressure writeback "
      "storage convention normal * ((value dot normal) * face_coord).");
  return params;
}

SharpInterfacePressureWritebackReconstructionError::
    SharpInterfacePressureWritebackReconstructionError(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _constant_vector(getParam<RealVectorValue>("constant_vector")),
    _reconstruction_error(0.0),
    _face_values(_subproblem.mesh(), _subproblem.mesh().meshSubdomains(), "face_values")
{
}

void
SharpInterfacePressureWritebackReconstructionError::initialize()
{
  const auto state = determineState();
  _reconstruction_error = 0.0;

  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    const Real normal_projection = _constant_vector * fi->normal();
    _face_values[fi->id()] = fi->normal() * (normal_projection * fi->faceCoord());
  }

  for (const Elem * elem : _fe_problem.mesh().getMesh().active_local_element_ptr_range())
  {
    const auto elem_arg = makeElemArg(elem);
    const Real elem_volume = this->_assembly.elementVolume(elem);
    const RealVectorValue diff = _constant_vector - _face_values(elem_arg, state);
    _reconstruction_error += diff * diff * elem_volume;
  }
}

void
SharpInterfacePressureWritebackReconstructionError::finalize()
{
  gatherSum(_reconstruction_error);
  _reconstruction_error = std::sqrt(_reconstruction_error);
}

PostprocessorValue
SharpInterfacePressureWritebackReconstructionError::getValue() const
{
  return _reconstruction_error;
}
