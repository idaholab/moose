//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVMomentumPressure.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "NS.h"
#include "FEProblemBase.h"
#include "LinearFVGradientInterface.h"

registerMooseObject("NavierStokesApp", LinearFVMomentumPressure);

InputParameters
LinearFVMomentumPressure::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription("Represents the pressure gradient term in the Navier Stokes momentum "
                             "equations, added to the right hand side.");
  params.addParam<VariableName>(NS::pressure,
                                "The pressure variable whose gradient should be used.");
  params.addParam<GradientMethodName>(
      "gradient_method",
      "Gradient method to use for the pressure gradient in this kernel. If omitted, the pressure "
      "variable's default gradient method is used.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

LinearFVMomentumPressure::LinearFVMomentumPressure(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _index(getParam<MooseEnum>("momentum_component")),
    _pressure_var(getPressureVariable(NS::pressure)),
    _pressure_gradient_field(registerPressureGradientField())
{
}

MooseLinearVariableFV<Real> &
LinearFVMomentumPressure::getPressureVariable(const std::string & vname)
{
  auto * ptr = dynamic_cast<MooseLinearVariableFV<Real> *>(
      &_fe_problem.getVariable(_tid, getParam<VariableName>(vname)));

  if (!ptr)
    paramError(NS::pressure, "The pressure variable should be of type MooseLinearVariableFVReal!");

  return *ptr;
}

const LinearFVGradientReader &
LinearFVMomentumPressure::registerPressureGradientField()
{
  if (!isParamValid("gradient_method"))
    return _pressure_var.computeCellGradients();

  return _pressure_var.computeCellGradients(getParam<GradientMethodName>("gradient_method"));
}

Real
LinearFVMomentumPressure::computeMatrixContribution()
{
  return 0.0;
}

Real
LinearFVMomentumPressure::computeRightHandSideContribution()
{
  const Real pressure_gradient = _pressure_gradient_field.component(*_current_elem_info, _index);
  return -pressure_gradient * _current_elem_volume;
}
