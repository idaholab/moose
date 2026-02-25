//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearWCNSFVSmagorinskyTurbulentViscosityAux.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearWCNSFVSmagorinskyTurbulentViscosityAux);

InputParameters
LinearWCNSFVSmagorinskyTurbulentViscosityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the turbulent viscosity for the mixing length model.");
  params.addRequiredParam<SolverVariableName>("u", "The velocity in the x direction.");
  params.addParam<SolverVariableName>("v", "The velocity in the y direction.");
  params.addParam<SolverVariableName>("w", "The velocity in the z direction.");
  params.addParam<Real>("smagorinsky_constant", 0.18, "Value of Smagorinsky's constant to use");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density functor");
  return params;
}

LinearWCNSFVSmagorinskyTurbulentViscosityAux::LinearWCNSFVSmagorinskyTurbulentViscosityAux(
    const InputParameters & params)
  : AuxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<MooseLinearVariableFVReal *>(
        &_subproblem.getVariable(_tid, getParam<SolverVariableName>("u")))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<MooseLinearVariableFVReal *>(
                     &_subproblem.getVariable(_tid, getParam<SolverVariableName>("v")))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<MooseLinearVariableFVReal *>(
                     &_subproblem.getVariable(_tid, getParam<SolverVariableName>("w")))
               : nullptr),
    _rho(getFunctor<Real>(NS::density)),
    _Cs(getParam<Real>("smagorinsky_constant"))
{
  if (!_u_var)
    paramError("u", "the u velocity must be a MooseLinearVariableFVReal.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be a "
               "MooseLinearVariableFVReal.");

  if (_dim >= 3 && !_w_var)
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be a "
               "MooseLinearVariableFVReal.");

  _u_var->computeCellGradients();
  if (_v_var)
    _v_var->computeCellGradients();
  if (_w_var)
    _w_var->computeCellGradients();
}

Real
LinearWCNSFVSmagorinskyTurbulentViscosityAux::computeValue()
{
  const auto & elem_info = _mesh.elemInfo(_current_elem->id());

  const auto grad_u = _u_var->gradSln(elem_info);
  Real symmetric_strain_tensor_norm = 2.0 * Utility::pow<2>(grad_u(0));
  if (_dim >= 2)
  {
    const auto grad_v = _v_var->gradSln(elem_info);
    symmetric_strain_tensor_norm +=
        2.0 * Utility::pow<2>(grad_v(1)) + Utility::pow<2>(grad_v(0) + grad_u(1));
    if (_dim >= 3)
    {
      const auto grad_w = _w_var->gradSln(elem_info);
      symmetric_strain_tensor_norm += 2.0 * Utility::pow<2>(grad_w(2)) +
                                      Utility::pow<2>(grad_u(2) + grad_w(0)) +
                                      Utility::pow<2>(grad_v(2) + grad_w(1));
    }
  }

  symmetric_strain_tensor_norm = std::sqrt(symmetric_strain_tensor_norm);

  return _rho(makeElemArg(_current_elem), determineState()) * symmetric_strain_tensor_norm *
         libMesh::Utility::pow<2>(_Cs * _current_elem->hmax());
}
