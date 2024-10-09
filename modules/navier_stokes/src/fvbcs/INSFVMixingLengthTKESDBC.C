//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMixingLengthTKESDBC.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMixingLengthTKESDBC);

InputParameters
INSFVMixingLengthTKESDBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription("Adds inlet boundary condition for the turbulent kinetic energy "
                             "specific dissipation rate based on characteristic length.");
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "The turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>("characteristic_length",
                                            "Characteristic length of the inlet in the problem.");
  return params;
}

INSFVMixingLengthTKESDBC::INSFVMixingLengthTKESDBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    _k(getFunctor<ADReal>(NS::TKE)),
    _characteristic_length(getFunctor<ADReal>("characteristic_length"))
{
}

ADReal
INSFVMixingLengthTKESDBC::boundaryValue(const FaceInfo & fi) const
{
  const auto boundary_face = singleSidedFaceArg(&fi);
  const auto state = determineState();

  return std::sqrt(_k(boundary_face, state)) /
         (0.07 * _characteristic_length(boundary_face, state));
}
