//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVMomentumBuoyancy.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "NS.h"
#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", LinearFVMomentumBuoyancy);

InputParameters
LinearFVMomentumBuoyancy::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription("Represents the buoyancy term in the Navier Stokes momentum "
                             "equations, added to the right hand side.");


  params.addRequiredParam<RealVectorValue>("gravity", "Gravitational acceleration vector.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The value for the density");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addParam<RealVectorValue>("pressure_ref_point", RealVectorValue(0, 0, 0), "The value for the pressure reference point.");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");

  return params;
}

LinearFVMomentumBuoyancy::LinearFVMomentumBuoyancy(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _index(getParam<MooseEnum>("momentum_component")),
    _rho(getFunctor<Real>(NS::density)),
    _gravity(getParam<RealVectorValue>("gravity")),
    _ref_point(getParam<RealVectorValue>("pressure_ref_point"))
{
}

Real
LinearFVMomentumBuoyancy::computeMatrixContribution()
{
  return 0.0;
}

Real
LinearFVMomentumBuoyancy::computeRightHandSideContribution()
{
  const auto elem = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();
  return - _rho.gradient(elem, state)(_index) *_gravity *  (_current_elem_info->centroid()-_ref_point) *_current_elem_volume;
}
