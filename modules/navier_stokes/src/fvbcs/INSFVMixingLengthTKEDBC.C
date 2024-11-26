//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMixingLengthTKEDBC.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMixingLengthTKEDBC);

InputParameters
INSFVMixingLengthTKEDBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription("Adds inlet boundary condition for the turbulent kinetic energy "
                             "dissipation rate based on characteristic length.");
  params.addParam<MooseFunctorName>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  params.addRequiredParam<MooseFunctorName>("k", "The turbulent kinetic energy.");
  params.deprecateParam("k", NS::TKE, "01/01/2025");
  params.addRequiredParam<MooseFunctorName>("characteristic_length",
                                            "Characteristic length of the inlet in the problem.");
  return params;
}

INSFVMixingLengthTKEDBC::INSFVMixingLengthTKEDBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    _C_mu(getFunctor<ADReal>("C_mu")),
    _k(getFunctor<ADReal>(NS::TKE)),
    _characteristic_length(getFunctor<ADReal>("characteristic_length"))
{
}

ADReal
INSFVMixingLengthTKEDBC::boundaryValue(const FaceInfo & fi, const Moose::StateArg & state) const
{
  const auto boundary_face = singleSidedFaceArg(&fi);

  return std::pow(_C_mu(boundary_face, state), 0.75) * std::pow(_k(boundary_face, state), 1.5) /
         (0.07 * _characteristic_length(boundary_face, state));
}
