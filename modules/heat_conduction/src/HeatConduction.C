/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "HeatConduction.h"
#include "MooseMesh.h"

#include "XFEM.h"

template<>
InputParameters validParams<HeatConductionKernel>()
{
  InputParameters params = validParams<Diffusion>();
  params.addClassDescription("Compute thermal conductivity");
  params.addParam<MaterialPropertyName>("diffusion_coefficient_name",
                                        "thermal_conductivity",
                                        "Property name of the diffusivity (Default: thermal_conductivity)");
  params.addParam<MaterialPropertyName>("diffusion_coefficient_dT_name",
                                        "thermal_conductivity_dT",
                                        "Property name of the derivative of the diffusivity with respect "
                                        "to the variable (Default: thermal_conductivity_dT)");
  params.set<bool>("use_displaced_mesh") = true;
  params.addCoupledVar("xfem_volfrac", "Coupled XFEM Volume Fraction");
  return params;
}

HeatConductionKernel::HeatConductionKernel(const InputParameters & parameters) :
    Diffusion(parameters),
    _dim(_subproblem.mesh().dimension()),
    _diffusion_coefficient(getMaterialProperty<Real>("diffusion_coefficient_name")),
    _diffusion_coefficient_dT(hasMaterialProperty<Real>("diffusion_coefficient_dT_name") ?
                              &getMaterialProperty<Real>("diffusion_coefficient_dT_name") : NULL),
  _has_xfem_volfrac(isCoupled("xfem_volfrac")),
  _xfem_volfrac(_has_xfem_volfrac ? coupledValue("xfem_volfrac") : _zero)
{
}

Real
HeatConductionKernel::computeQpResidual()
{
  Real r = _diffusion_coefficient[_qp]*Diffusion::computeQpResidual();
  if (_has_xfem_volfrac)
    r*=_xfem_volfrac[_qp];
  return r;
}

Real
HeatConductionKernel::computeQpJacobian()
{
  Real jac = _diffusion_coefficient[_qp] * Diffusion::computeQpJacobian();
  if (_diffusion_coefficient_dT)
    jac += (*_diffusion_coefficient_dT)[_qp] * _phi[_j][_qp] * Diffusion::computeQpResidual();
  if (_has_xfem_volfrac)
    jac*=_xfem_volfrac[_qp];
  return jac;
}
