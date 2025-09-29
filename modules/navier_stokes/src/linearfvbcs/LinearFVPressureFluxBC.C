//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPressureFluxBC.h"

registerMooseObject("NavierStokesApp", LinearFVPressureFluxBC);

InputParameters
LinearFVPressureFluxBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription(
      "Adds a fixed diffusive flux BC which can be used for the assembly of linear "
      "finite volume system and whose normal face gradient values are determined "
      "using a functor. This kernel is only designed to work with advection-diffusion problems.");
  params.addRequiredParam<MooseFunctorName>("HbyA_flux", "The total HbyA face flux value.");
  params.addRequiredParam<MooseFunctorName>("Ainv", "The 1/A momentum system diagonal vector.");
  return params;
}

LinearFVPressureFluxBC::LinearFVPressureFluxBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _HbyA_flux(getFunctor<Real>("HbyA_flux")),
    _Ainv(getFunctor<RealVectorValue>("Ainv"))
{
}

Real
LinearFVPressureFluxBC::computeBoundaryValue() const
{
  const auto face_arg = makeCDFace(*_current_face_info);
  const auto elem_arg = makeElemArg(_current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                        ? _current_face_info->elemPtr()
                                        : _current_face_info->neighborPtr());
  const Real distance = computeCellToFaceDistance();

  if (_Ainv(face_arg, determineState())(0) != 0.0)
    return raw_value(_var(elem_arg, determineState())) -
           _HbyA_flux(singleSidedFaceArg(_current_face_info), determineState()) /
               _Ainv(face_arg, determineState())(0) * distance;
  else
    return raw_value(_var(elem_arg, determineState())); // zero-term expansion
}

Real
LinearFVPressureFluxBC::computeBoundaryNormalGradient() const
{
  const auto face_arg = makeCDFace(*_current_face_info);

  const Real distance = computeCellToFaceDistance();

  if (_Ainv(face_arg, determineState())(0) != 0.0)
    return -_HbyA_flux(singleSidedFaceArg(_current_face_info), determineState()) /
           _Ainv(face_arg, determineState())(0) * distance;
  else
    return 0.0;
}

Real
LinearFVPressureFluxBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0;
}

Real
LinearFVPressureFluxBC::computeBoundaryValueRHSContribution() const
{
  const auto face_arg = makeCDFace(*_current_face_info);
  const Real distance = computeCellToFaceDistance();

  if (_Ainv(face_arg, determineState())(0) != 0.0)
    return -_HbyA_flux(singleSidedFaceArg(_current_face_info), determineState()) /
           _Ainv(face_arg, determineState())(0) * distance;
  else
    return 0.0;
}

Real
LinearFVPressureFluxBC::computeBoundaryGradientMatrixContribution() const
{
  return 0.0;
}

Real
LinearFVPressureFluxBC::computeBoundaryGradientRHSContribution() const
{
  return -_HbyA_flux(singleSidedFaceArg(_current_face_info), determineState());
}
