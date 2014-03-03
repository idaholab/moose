/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "StressDivergence.h"

#include "Material.h"
#include "SymmElasticityTensor.h"

template<>
InputParameters validParams<StressDivergence>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature");
  params.addParam<Real>("zeta", 0.0, "Stiffness dependent Rayleigh damping coefficient");
  params.addParam<Real>("alpha", 0.0, "alpha parameter required for HHT time integration");
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");
  params.addCoupledVar("xfem_volfrac", "Coupled XFEM Volume Fraction");

  params.set<bool>("use_displaced_mesh") = true;

  return params;
}


StressDivergence::StressDivergence(const InputParameters & parameters) :
    Kernel(parameters),
    _stress_older(getMaterialPropertyOlder<SymmTensor>("stress" + getParam<std::string>("appended_property_name"))),
    _stress_old(getMaterialPropertyOld<SymmTensor>("stress" + getParam<std::string>("appended_property_name"))),
    _stress(getMaterialProperty<SymmTensor>("stress" + getParam<std::string>("appended_property_name"))),
    _Jacobian_mult(getMaterialProperty<SymmElasticityTensor>("Jacobian_mult" + getParam<std::string>("appended_property_name"))),
    _d_stress_dT(getMaterialProperty<SymmTensor>("d_stress_dT"+ getParam<std::string>("appended_property_name"))),
    _component(getParam<unsigned int>("component")),
    _xdisp_coupled(isCoupled("disp_x")),
    _ydisp_coupled(isCoupled("disp_y")),
    _zdisp_coupled(isCoupled("disp_z")),
    _temp_coupled(isCoupled("temp")),
    _xdisp_var(_xdisp_coupled ? coupled("disp_x") : 0),
    _ydisp_var(_ydisp_coupled ? coupled("disp_y") : 0),
    _zdisp_var(_zdisp_coupled ? coupled("disp_z") : 0),
    _temp_var(_temp_coupled ? coupled("temp") : 0),
    _zeta(getParam<Real>("zeta")),
    _alpha(getParam<Real>("alpha")),
    _has_xfem_volfrac(isCoupled("xfem_volfrac")),
    _xfem_volfrac(_has_xfem_volfrac ? coupledValue("xfem_volfrac") : _zero)
{}

Real
StressDivergence::computeQpResidual()
{
  Real r;
  if ((_dt > 0) && ((_zeta != 0) || (_alpha != 0)))
    r =  _stress[_qp].rowDot(_component, _grad_test[_i][_qp])
      * (1 + _alpha+(1+_alpha) * _zeta/_dt)
      - (_alpha + (1+2*_alpha)*_zeta/_dt)*_stress_old[_qp].rowDot(_component, _grad_test[_i][_qp])
      + (_alpha * _zeta/_dt)*_stress_older[_qp].rowDot(_component,_grad_test[_i][_qp]);
  else
    r =  _stress[_qp].rowDot(_component, _grad_test[_i][_qp]);
  if (_has_xfem_volfrac)
  {
    r*=_xfem_volfrac[_qp];
  }
  return r;
}

Real
StressDivergence::computeQpJacobian()
{
  Real jac;
  if (_dt > 0)
    jac = _Jacobian_mult[_qp].stiffness(_component, _component, _grad_test[_i][_qp], _grad_phi[_j][_qp]) * (1 + _alpha + _zeta/_dt);
  else
    jac = _Jacobian_mult[_qp].stiffness(_component, _component, _grad_test[_i][_qp], _grad_phi[_j][_qp]);
  if (_has_xfem_volfrac)
  {
    jac*=_xfem_volfrac[_qp];
  }
  return jac;
}

Real
StressDivergence::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int coupled_component = 0;

  bool active = false;

  if (_xdisp_coupled && jvar == _xdisp_var)
  {
    coupled_component = 0;
    active = true;
  }
  else if (_ydisp_coupled && jvar == _ydisp_var)
  {
    coupled_component = 1;
    active = true;
  }
  else if (_zdisp_coupled && jvar == _zdisp_var)
  {
    coupled_component = 2;
    active = true;
  }

  if (active)
  {
    if (_dt > 0)
      return _Jacobian_mult[_qp].stiffness(_component, coupled_component, _grad_test[_i][_qp], _grad_phi[_j][_qp]) * (1 + _alpha + _zeta / _dt);
    else
      return _Jacobian_mult[_qp].stiffness(_component, coupled_component, _grad_test[_i][_qp], _grad_phi[_j][_qp]);
  }

  if (_temp_coupled && jvar == _temp_var)
    return _d_stress_dT[_qp].rowDot(_component, _grad_test[_i][_qp]) * _phi[_j][_qp];

  return 0;
}
