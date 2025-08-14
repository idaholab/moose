//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MatDiffusionBase.h"
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

template <typename T>
class PolycrystalMatDiffusionBase : public MatDiffusionBase<T>
{
public:
  static InputParameters validParams();

  PolycrystalMatDiffusionBase(const InputParameters & parameters);

protected:
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  /// Number of grain order parameters for cases where there is dependence on grain OP gradients
  const unsigned int _op_num;

  /// diffusion coefficient derivatives w.r.t. variables that have explicit dependence on gradients
  const MaterialProperty<typename GradientType<T>::type> & _dDdgradc;
  std::vector<const MaterialProperty<typename GradientType<T>::type> *> _dDdgradeta;

  /// For solid-pore systems, mame of the order parameter identifies the solid-pore surface
  unsigned int _surface_op_var;

  /// Variable to allow user to control whether grain OP gradient contributions are added to Jacobian
  const bool _add_grain_op_gradients;
};

template <typename T>
InputParameters
PolycrystalMatDiffusionBase<T>::validParams()
{
  InputParameters params = MatDiffusionBase<T>::validParams();
  params.addCoupledVar(
      "surface_op_var",
      "Name of the order parameter for solid-pore surface. For use when diffusivity "
      "depends on these OP gradients, leave this parameter un-set otherwise. ");
  params.addCoupledVarWithAutoBuild(
      "grain_op_vars", "var_name_base", "op_num", "Array of grain order parameter variables");
  params.addParam<bool>(
      "add_grain_op_gradients",
      true,
      "Whether grain order parameter gradient contributions are added to Jacobian.");
  return params;
}

template <typename T>
PolycrystalMatDiffusionBase<T>::PolycrystalMatDiffusionBase(const InputParameters & parameters)
  : MatDiffusionBase<T>(parameters),
    _op_num(this->coupledComponents("grain_op_vars")),
    _dDdgradc(this->template getMaterialPropertyDerivative<typename GradientType<T>::type>(
        this->isParamValid("D_name") ? "D_name" : "diffusivity", "gradc")),
    _dDdgradeta(_op_num),
    _surface_op_var(this->isCoupled("surface_op_var") ? this->coupled("surface_op_var")
                                                      : libMesh::invalid_uint),
    _add_grain_op_gradients(this->template getParam<bool>("add_grain_op_gradients"))
{
  if (_add_grain_op_gradients)
    for (unsigned int j = 0; j < _op_num; ++j)
      _dDdgradeta[j] =
          &this->template getMaterialPropertyDerivative<typename GradientType<T>::type>(
              this->isParamValid("D_name") ? "D_name" : "diffusivity",
              ("gradgr" + Moose::stringify(j)));
}

template <typename T>
Real
PolycrystalMatDiffusionBase<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  // const unsigned int cvar = this->mapJvarToCvar(jvar);

  Real sum = MatDiffusionBase<T>::computeQpOffDiagJacobian(jvar);
  if (jvar == _surface_op_var)
    sum += _dDdgradc[this->_qp] * this->_grad_phi[this->_j][this->_qp] * this->_grad_v[this->_qp] *
           this->_grad_test[this->_i][this->_qp];

  if (_add_grain_op_gradients)
  {
    for (unsigned int k = 0; k < _op_num; ++k)
      if (jvar == this->coupled("grain_op_vars", k))
        sum += (*_dDdgradeta[k])[this->_qp] * this->_grad_phi[this->_j][this->_qp] *
               this->_grad_v[this->_qp] * this->_grad_test[this->_i][this->_qp];
  }

  return sum;
}
