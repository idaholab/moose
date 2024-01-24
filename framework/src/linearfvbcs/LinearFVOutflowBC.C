//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVOutflowBC.h"
#include "GreenGaussGradient.h"

registerMooseObject("MooseApp", LinearFVOutflowBC);

InputParameters
LinearFVOutflowBC::validParams()
{
  InputParameters params = LinearFVBoundaryCondition::validParams();
  params.addClassDescription(
      "Adds a boundary condition which represents a surface with outflowing material.");
  params.addParam<bool>(
      "use_two_term_expansion",
      false,
      "If an approximate linear expansion should be used to compute the face value.");
  params.addRequiredParam<RealVectorValue>("velocity", "The velocity at the outflow surface.");
  return params;
}

LinearFVOutflowBC::LinearFVOutflowBC(const InputParameters & parameters)
  : LinearFVBoundaryCondition(parameters),
    _two_term_expansion(getParam<bool>("use_two_term_expansion")),
    _velocity(getParam<RealVectorValue>("velocity"))
{
  if (_two_term_expansion)
    _var->computeCellGradients();
}

Real
LinearFVOutflowBC::computeBoundaryValue()
{
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();
  auto boundary_value =
      (*_linear_system.solution)(elem_info->dofIndices()[_var->sys().number()][_var->number()]);

  if (_two_term_expansion)
    boundary_value += _var->gradSln(elem_info) * computeCellToFaceVector();

  return boundary_value;
}

RealVectorValue
LinearFVOutflowBC::computeCellToFaceVector() const
{
  const auto is_on_mesh_boundary = !_current_face_info->neighborPtr();
  const auto defined_on_elem =
      is_on_mesh_boundary ? true : (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM);
  if (is_on_mesh_boundary)
    return _current_face_info->dCN();
  else
    return (_current_face_info->faceCentroid() - (defined_on_elem
                                                      ? _current_face_info->elemCentroid()
                                                      : _current_face_info->neighborCentroid()));
}

Real
LinearFVOutflowBC::computeBoundaryNormalGradient()
{
  mooseError("Not implemented yet");
}

Real
LinearFVOutflowBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0;
}
Real
LinearFVOutflowBC::computeBoundaryValueRHSContribution() const
{
  Real contribution = 0.0;

  if (_two_term_expansion)
  {

    const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
    contribution = _var->gradSln(elem_info) * computeCellToFaceVector();
  }

  return contribution;
}

Real
LinearFVOutflowBC::computeBoundaryGradientMatrixContribution() const
{
  mooseError("Not implemented yet");
}

Real
LinearFVOutflowBC::computeBoundaryGradientRHSContribution() const
{
  mooseError("Not implemented yet");
}
