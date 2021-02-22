//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVPorosityMomentumMatAdvectionOutflowBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVPorosityMomentumMatAdvectionOutflowBC);

InputParameters
NSFVPorosityMomentumMatAdvectionOutflowBC::validParams()
{
  InputParameters params = FVMatAdvectionOutflowBC::validParams();
  params.addClassDescription(
      "Computes the residual of advective term with porosity using finite volume method.");
  MooseEnum one_over_porosity_interp_method("average upwind", "average");

  params.addParam<MooseEnum>("one_over_porosity_interp_method",
                             one_over_porosity_interp_method,
                             "The interpolation to use for the one/porosity prefactor. Options are "
                             "'upwind' and 'average', with the default being 'average'.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

NSFVPorosityMomentumMatAdvectionOutflowBC::NSFVPorosityMomentumMatAdvectionOutflowBC(
    const InputParameters & params)
  : FVMatAdvectionOutflowBC(params),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity)),
    _p_elem(getADMaterialProperty<Real>(NS::pressure)),
    _p_neighbor(getNeighborADMaterialProperty<Real>(NS::pressure)),
    _index(getParam<MooseEnum>("momentum_component"))
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
NSFVPorosityMomentumMatAdvectionOutflowBC::computeQpResidual()
{
  using namespace Moose::FV;

  ADReal one_over_porosity_face;

  auto residual = FVMatAdvectionOutflowBC::computeQpResidual();

  const auto one_over_porosity_elem = 1 / _eps_elem[_qp];
  const auto one_over_porosity_neighbor = 1 / _eps_neighbor[_qp];

  interpolate(_one_over_porosity_interp_method,
              one_over_porosity_face,
              one_over_porosity_elem,
              one_over_porosity_neighbor,
              _v,
              *_face_info,
              true);

  ADReal eps_p_face;
  interpolate(InterpMethod::Average,
              eps_p_face,
              _eps_elem[_qp] * _p_elem[_qp],
              _eps_neighbor[_qp] * _p_neighbor[_qp],
              *_face_info,
              true);

  return residual * one_over_porosity_face + _normal(_index) * eps_p_face;
}
