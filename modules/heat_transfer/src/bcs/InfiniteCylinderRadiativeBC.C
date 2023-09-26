//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InfiniteCylinderRadiativeBC.h"
#include "MathUtils.h"

registerMooseObject("HeatConductionApp", InfiniteCylinderRadiativeBC);
registerMooseObject("HeatConductionApp", ADInfiniteCylinderRadiativeBC);

template <bool is_ad>
InputParameters
InfiniteCylinderRadiativeBCTempl<is_ad>::validParams()
{
  InputParameters params = RadiativeHeatFluxBCBaseTempl<is_ad>::validParams();
  params.addParam<Real>("cylinder_emissivity",
                        1,
                        "Emissivity of the cylinder in radiative heat transfer with the boundary.");
  params.addRequiredParam<Real>("boundary_radius",
                                "Radius of the boundary approximated as cylinder.");
  params.addRequiredParam<Real>("cylinder_radius",
                                "Radius of the cylinder on the outside of the boundary.");
  params.addClassDescription("Boundary condition for radiative heat exchange with a cylinder"
                             "where the boundary is approximated as a cylinder as well.");
  return params;
}

template <bool is_ad>
InfiniteCylinderRadiativeBCTempl<is_ad>::InfiniteCylinderRadiativeBCTempl(
    const InputParameters & parameters)
  : RadiativeHeatFluxBCBaseTempl<is_ad>(parameters),
    _eps_cylinder(this->template getParam<Real>("cylinder_emissivity")),
    _boundary_radius(this->template getParam<Real>("boundary_radius")),
    _cylinder_radius(this->template getParam<Real>("cylinder_radius"))
{
  _coefficient = this->_eps_boundary * _eps_cylinder * _cylinder_radius /
                 (_eps_cylinder * _cylinder_radius +
                  this->_eps_boundary * _boundary_radius * (1 - _eps_cylinder));
}

template <bool is_ad>
GenericReal<is_ad>
InfiniteCylinderRadiativeBCTempl<is_ad>::coefficient() const
{
  return _coefficient;
}

template class InfiniteCylinderRadiativeBCTempl<false>;
template class InfiniteCylinderRadiativeBCTempl<true>;
