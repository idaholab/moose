//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"
#include "RankThreeTensor.h"

template <typename T>
struct GradientType;

template <>
struct GradientType<Real>
{
  typedef RealVectorValue type;
};

/**
 * In the long term the GradientType struct can be eliminated by using the specialization of
 * libmesh::TensorTools::IncrementRank for RankTwoTensor, this will require changing the derived
 * class MatAnisoDiffusion and several other classes from RealTensorValue to RankTwoTensor
 */
template <>
struct GradientType<RealTensorValue>
{
  typedef RankThreeTensor type;
};

/**
 * This class template implements a diffusion kernel with a mobility that can vary
 * spatially and can depend on variables in the simulation. Two classes are derived from
 * this template, MatDiffusion for isotropic diffusion and MatAnisoDiffusion for
 * anisotropic diffusion.
 *
 * \tparam T Type of the diffusion coefficient parameter. This can be Real for
 *           isotropic diffusion or RealTensorValue for the general anisotropic case.
 */
template <typename T>
class MatDiffusionBase : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  MatDiffusionBase(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  virtual Real computeQpCJacobian();

  /// diffusion coefficient
  const MaterialProperty<T> & _D;

  /// diffusion coefficient derivative w.r.t. the kernel variable
  const MaterialProperty<T> & _dDdc;

  /// diffusion coefficient derivatives w.r.t. coupled variables
  std::vector<const MaterialProperty<T> *> _dDdarg;

  /// Number of grain order parameters for cases where there is dependence on grain OP gradients
  const unsigned int _op_num;

  /// diffusion coefficient derivatives w.r.t. variables that have explicit dependence on gradients
  const MaterialProperty<typename GradientType<T>::type> & _dDdgradc;
  std::vector<const MaterialProperty<typename GradientType<T>::type> *> _dDdgradeta;

  /// is the kernel used in a coupled form?
  const bool _is_coupled;

  /// int label for the Concentration
  unsigned int _v_var;

  /// Gradient of the concentration
  const VariableGradient & _grad_v;

  /// For solid-pore systems, mame of the order parameter identifies the solid-pore surface
  unsigned int _surface_op_var;

  /// Variable to allow user to control whether grain OP gradient contributions are added to Jacobian
  const bool _add_grain_op_gradients;
};

template <typename T>
InputParameters
MatDiffusionBase<T>::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addDeprecatedParam<MaterialPropertyName>(
      "D_name",
      "The name of the diffusivity",
      "This parameter has been renamed to 'diffusivity', which is more mnemonic and more conducive "
      "to passing a number literal");
  params.addParam<MaterialPropertyName>(
      "diffusivity", "D", "The diffusivity value or material property");
  params.addCoupledVar("args",
                       "Optional vector of arguments for the diffusivity. If provided and "
                       "diffusivity is a derivative parsed material, Jacobian contributions from "
                       "the diffusivity will be automatically computed");
  params.addCoupledVar("conc", "Deprecated! Use 'v' instead");
  params.addCoupledVar("v",
                       "Coupled concentration variable for kernel to operate on; if this "
                       "is not specified, the kernel's nonlinear variable will be used as "
                       "usual");
  params.addCoupledVar(
      "surface_op_var",
      "Name of the order parameter for solid-pore surface. For use when diffusivity "
      "depends on these OP gradients, leave this parameter un-set otherwise. ");
  params.addCoupledVarWithAutoBuild(
      "grain_op_vars", "var_name_base", "op_num", "Array of grain order parameter variables");
  params.addParam<bool>(
      "add_grain_op_gradients",
      false,
      "Whether grain order parameter gradient contributions are added to Jacobian. Set to false "
      "unless diffusivity explicitly depends on grain OP gradients.");

  return params;
}

template <typename T>
MatDiffusionBase<T>::MatDiffusionBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _D(isParamValid("D_name") ? getMaterialProperty<T>("D_name")
                              : getMaterialProperty<T>("diffusivity")),
    _dDdc(getMaterialPropertyDerivative<T>(isParamValid("D_name") ? "D_name" : "diffusivity",
                                           _var.name())),
    _dDdarg(_coupled_moose_vars.size()),
    _op_num(coupledComponents("grain_op_vars")),
    _dDdgradc(getMaterialPropertyDerivative<typename GradientType<T>::type>(
        isParamValid("D_name") ? "D_name" : "diffusivity", "gradc")),
    _dDdgradeta(_op_num),
    _is_coupled(isCoupled("v")),
    _v_var(_is_coupled ? coupled("v") : (isCoupled("conc") ? coupled("conc") : _var.number())),
    _grad_v(_is_coupled ? coupledGradient("v")
                        : (isCoupled("conc") ? coupledGradient("conc") : _grad_u)),
    _surface_op_var(isCoupled("surface_op_var") ? coupled("surface_op_var")
                                                : libMesh::invalid_uint),
    _add_grain_op_gradients(getParam<bool>("add_grain_op_gradients"))
{
  // deprecated variable parameter conc
  if (isCoupled("conc"))
    mooseDeprecated("In '", name(), "' the parameter 'conc' is deprecated, please use 'v' instead");

  // fetch derivatives
  for (unsigned int i = 0; i < _dDdarg.size(); ++i)
    _dDdarg[i] = &getMaterialPropertyDerivative<T>(
        isParamValid("D_name") ? "D_name" : "diffusivity", _coupled_moose_vars[i]->name());
  if (_add_grain_op_gradients)
    for (unsigned int j = 0; j < _op_num; ++j)
      _dDdgradeta[j] = &getMaterialPropertyDerivative<typename GradientType<T>::type>(
          isParamValid("D_name") ? "D_name" : "diffusivity", ("gradgr" + Moose::stringify(j)));
}

template <typename T>
void
MatDiffusionBase<T>::initialSetup()
{
  validateNonlinearCoupling<Real>(parameters().isParamSetByUser("D_name") ? "D_name"
                                                                          : "diffusivity");
}

template <typename T>
Real
MatDiffusionBase<T>::computeQpResidual()
{
  return _D[_qp] * _grad_v[_qp] * _grad_test[_i][_qp];
}

template <typename T>
Real
MatDiffusionBase<T>::computeQpJacobian()
{
  Real sum = _phi[_j][_qp] * _dDdc[_qp] * _grad_v[_qp] * _grad_test[_i][_qp];
  if (!_is_coupled)
    sum += computeQpCJacobian();

  return sum;
}

template <typename T>
Real
MatDiffusionBase<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  Real sum = (*_dDdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_v[_qp] * _grad_test[_i][_qp];
  if (jvar == _surface_op_var)
    sum += _dDdgradc[_qp] * _grad_phi[_j][_qp] * _grad_v[_qp] * _grad_test[_i][_qp];

  if (_add_grain_op_gradients)
  {
    for (unsigned int k = 0; k < _op_num; ++k)
      if (jvar == coupled("grain_op_vars", k))
        sum += (*_dDdgradeta[k])[_qp] * _grad_phi[_j][_qp] * _grad_v[_qp] * _grad_test[_i][_qp];
  }

  if (_v_var == jvar)
    sum += computeQpCJacobian();

  return sum;
}

template <typename T>
Real
MatDiffusionBase<T>::computeQpCJacobian()
{
  return _D[_qp] * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
