//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "RhoFromPTFunctorMaterial.h"
#include "NS.h"

// FluidProperties includes
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", RhoFromPTFunctorMaterial);

InputParameters
RhoFromPTFunctorMaterial::validParams()
{
  auto params = Material::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "fluid userobject");
  params.addClassDescription(
      "Computes the density from coupled pressure and temperature variables");
  params.addRequiredCoupledVar(NS::temperature, "the temperature");
  params.addRequiredCoupledVar(NS::pressure, "the pressure");
  return params;
}

RhoFromPTFunctorMaterial::RhoFromPTFunctorMaterial(const InputParameters & parameters)
  : Material(parameters),
    _pressure(*getVarHelper<MooseVariableFV<Real>>(NS::pressure, 0)),
    _temperature(*getVarHelper<MooseVariableFV<Real>>(NS::temperature, 0)),
    _fluid(getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _rho(declareFunctorProperty<ADReal>(NS::density))
{
  _rho.setFunction(_mesh, blockIDs(), [this](auto & geom_quantity) -> ADReal {
    return _fluid.rho_from_p_T(_pressure(geom_quantity), _temperature(geom_quantity));
  });
}
