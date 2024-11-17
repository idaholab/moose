//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVMomentumBoussinesq.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVMomentumBoussinesq);

InputParameters
LinearFVMomentumBoussinesq::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription("Represents the Boussinesq term in the Navier Stokes momentum "
                             "equations, added to the right hand side.");
  params.addParam<VariableName>(NS::T_fluid, "The fluid temperature variable.");
  params.addRequiredParam<RealVectorValue>("gravity", "Gravitational acceleration vector.");
  params.addParam<MooseFunctorName>("alpha_name",
                                    NS::alpha_boussinesq,
                                    "The name of the thermal expansion coefficient"
                                    "this is of the form rho = rho_ref*(1-alpha (T-T_ref))");
  params.addRequiredParam<Real>("ref_temperature", "The value for the reference temperature.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The value for the density");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

LinearFVMomentumBoussinesq::LinearFVMomentumBoussinesq(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _index(getParam<MooseEnum>("momentum_component")),
    _temperature_var(getTemperatureVariable(NS::T_fluid)),
    _gravity(getParam<RealVectorValue>("gravity")),
    _alpha(getFunctor<Real>("alpha_name")),
    _ref_temperature(getParam<Real>("ref_temperature")),
    _rho(getFunctor<Real>(NS::density))
{
  if (!_rho.isConstant())
    paramError(NS::density, "The density in the boussinesq term is not constant!");
}

const MooseLinearVariableFV<Real> &
LinearFVMomentumBoussinesq::getTemperatureVariable(const std::string & vname)
{
  auto * ptr = dynamic_cast<MooseLinearVariableFV<Real> *>(
      &_fe_problem.getVariable(_tid, getParam<VariableName>(vname)));

  if (!ptr)
    paramError(NS::T_fluid,
               "The fluid temperature variable should be of type MooseLinearVariableFVReal!");

  return *ptr;
}

Real
LinearFVMomentumBoussinesq::computeMatrixContribution()
{
  return 0.0;
}

Real
LinearFVMomentumBoussinesq::computeRightHandSideContribution()
{
  const auto elem = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();
  return -_alpha(elem, state) * _gravity(_index) * _rho(elem, state) *
         (_temperature_var.getElemValue(*_current_elem_info, state) - _ref_temperature) *
         _current_elem_volume;
}
