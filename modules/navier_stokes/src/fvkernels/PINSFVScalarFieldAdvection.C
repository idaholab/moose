//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVScalarFieldAdvection.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVScalarFieldAdvection);

InputParameters
PINSFVScalarFieldAdvection::validParams()
{
  auto params = INSFVScalarFieldAdvection::validParams();
  params.addClassDescription(
      "Advects an arbitrary quantity, the associated nonlinear 'variable' in porous medium.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity,
                                            "The name of the functor giving the local porosity");

  return params;
}

PINSFVScalarFieldAdvection::PINSFVScalarFieldAdvection(const InputParameters & params)
  : INSFVScalarFieldAdvection(params), _eps(getFunctor<ADReal>(NS::porosity))

{
  if (_add_slip_model)
    mooseError("Slip model and porous medium treatment is not currently supported");
}

ADReal
PINSFVScalarFieldAdvection::computeQpResidual()
{
  const auto state = determineState();
  // Note that we do not use the advected quantity interpolation because we expect to use this
  // with a functor material that does not support upwinding
  const auto eps_face = _eps(makeCDFace(*_face_info, false), state);

  return INSFVScalarFieldAdvection::computeQpResidual() / eps_face;
}
