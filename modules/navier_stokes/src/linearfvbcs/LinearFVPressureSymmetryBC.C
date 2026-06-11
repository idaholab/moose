//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPressureSymmetryBC.h"

registerMooseObject("NavierStokesApp", LinearFVPressureSymmetryBC);

InputParameters
LinearFVPressureSymmetryBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription("Adds a symmetry boundary condition for pressure in a segregated "
                             "velocity and pressure solve.");
  params.addRequiredParam<MooseFunctorName>("HbyA_flux", "The total HbyA face flux value.");
  return params;
}

LinearFVPressureSymmetryBC::LinearFVPressureSymmetryBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters), _HbyA_flux(getFunctor<Real>("HbyA_flux"))
{
}

Real
LinearFVPressureSymmetryBC::computeBoundaryValue() const
{
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();
  return _var.getElemValue(*elem_info, determineState());
}

Real
LinearFVPressureSymmetryBC::computeBoundaryNormalGradient() const
{
  return 0.0;
}

Real
LinearFVPressureSymmetryBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0;
}

Real
LinearFVPressureSymmetryBC::computeBoundaryValueRHSContribution() const
{
  return 0.0;
}

Real
LinearFVPressureSymmetryBC::computeBoundaryGradientMatrixContribution() const
{
  return 0.0;
}

Real
LinearFVPressureSymmetryBC::computeBoundaryGradientRHSContribution() const
{
  return -_HbyA_flux(singleSidedFaceArg(_current_face_info), determineState());
}
