//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVPorosityMatAdvection.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVPorosityMatAdvection);

InputParameters
NSFVPorosityMatAdvection::validParams()
{
  InputParameters params = FVMatAdvection::validParams();
  params.addClassDescription(
      "Computes the residual of advective term with porosity using finite volume method.");
  MooseEnum one_over_porosity_interp_method("average upwind", "average");

  params.addParam<MooseEnum>("one_over_porosity_interp_method",
                             one_over_porosity_interp_method,
                             "The interpolation to use for the one/porosity prefactor. Options are "
                             "'upwind' and 'average', with the default being 'average'.");
  return params;
}

NSFVPorosityMatAdvection::NSFVPorosityMatAdvection(const InputParameters & params)
  : FVMatAdvection(params),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity))
{
  using namespace Moose::FV;

  const auto & one_over_porosity_interp_method =
      getParam<MooseEnum>("one_over_porosity_interp_method");
  if (one_over_porosity_interp_method == "average")
    _one_over_porosity_interp_method = InterpMethod::Average;
  else if (one_over_porosity_interp_method == "upwind")
    _one_over_porosity_interp_method = InterpMethod::Upwind;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(one_over_porosity_interp_method));
}

ADReal
NSFVPorosityMatAdvection::computeQpResidual()
{
  auto residual = FVMatAdvection::computeQpResidual();

  ADReal one_over_porosity_interface;
  const auto one_over_porosity_elem = 1 / _eps_elem[_qp];
  const auto one_over_porosity_neighbor = 1 / _eps_neighbor[_qp];

  using namespace Moose::FV;

  interpolate(_one_over_porosity_interp_method,
              one_over_porosity_interface,
              one_over_porosity_elem,
              one_over_porosity_neighbor,
              _v,
              *_face_info,
              true);
  return residual * one_over_porosity_interface;
}
