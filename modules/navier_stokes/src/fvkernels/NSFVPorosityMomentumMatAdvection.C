//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVPorosityMomentumMatAdvection.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVPorosityMomentumMatAdvection);

InputParameters
NSFVPorosityMomentumMatAdvection::validParams()
{
  InputParameters params = NSFVPorosityMatAdvection::validParams();
  params.addClassDescription("Computes the residual of the momentum advective term with porosity "
                             "using finite volume method.");
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

NSFVPorosityMomentumMatAdvection::NSFVPorosityMomentumMatAdvection(const InputParameters & params)
  : NSFVPorosityMatAdvection(params),
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
NSFVPorosityMomentumMatAdvection::computeQpResidual()
{
  using namespace Moose::FV;

  ADReal one_over_porosity_interface, u_interface, eps_p_interface, eps_interface;
  Real elem_coeff, neighbor_coeff;

  interpolate(InterpMethod::Average, _v, _vel_elem[_qp], _vel_neighbor[_qp], *_face_info, true);
  if (MetaPhysicL::raw_value(_v) * _face_info->normal() > 0)
  {
    elem_coeff = 1;
    neighbor_coeff = 0;
    eps_interface = _eps_elem[_qp];
  }
  else
  {
    elem_coeff = 0;
    neighbor_coeff = 1;
    eps_interface = _eps_neighbor[_qp];
  }
  _v = _vel_elem[_qp] * elem_coeff + _vel_neighbor[_qp] * neighbor_coeff;
  u_interface = _adv_quant_elem[_qp] * elem_coeff + _adv_quant_neighbor[_qp] * neighbor_coeff;
  one_over_porosity_interface = 1 / eps_interface;

  interpolate(InterpMethod::Average,
              eps_p_interface,
              _eps_elem[_qp] * _p_elem[_qp],
              _eps_neighbor[_qp] * _p_neighbor[_qp],
              *_face_info,
              true);

  return _normal * _v * u_interface * one_over_porosity_interface +
         _normal(_index) * eps_p_interface;
}
