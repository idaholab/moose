//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeAdvectionBC.h"
#include "Function.h"

registerMooseObject("MooseApp", ConservativeAdvectionBC);
registerMooseObject("MooseApp", ADConservativeAdvectionBC);

template <bool is_ad>
InputParameters
ConservativeAdvectionBCTempl<is_ad>::validParams()
{
  InputParameters params = GenericIntegratedBC<is_ad>::validParams();
  params.addParam<MaterialPropertyName>(
      "velocity_mat_prop",
      "Velocity vector as a material property. Should be provided when we want the velocity value "
      "to be determined implicitly (e.g. we don't have a Dirichlet condition)");
  params.addParam<FunctionName>("velocity_function",
                                "Function describing the values of velocity on the boundary.");
  params.addClassDescription(
      "Boundary condition for advection when it is integrated by parts. Supports Dirichlet "
      "(inlet-like) and implicit (outlet-like) conditions.");
  params.addParam<MaterialPropertyName>("advected_quantity",
                                        "An optional material property to be advected. If not "
                                        "supplied, then the variable will be used.");
  params.addParam<FunctionName>("primal_dirichlet_value",
                                "The value of the primal variable on the boundary.");
  params.addParam<MaterialPropertyName>(
      "primal_coefficient",
      1.0,
      "If a primal Dirichlet value is supplied, then a coefficient may be optionally multiplied "
      "that multiples the Dirichlet value");
  return params;
}

template <bool is_ad>
ConservativeAdvectionBCTempl<is_ad>::ConservativeAdvectionBCTempl(
    const InputParameters & parameters)
  : GenericIntegratedBC<is_ad>(parameters),
    _velocity_mat_prop(isParamValid("velocity_mat_prop")
                           ? &this->template getGenericMaterialProperty<RealVectorValue, is_ad>(
                                 "velocity_mat_prop")
                           : nullptr),
    _velocity_function(isParamValid("velocity_function") ? &getFunction("velocity_function")
                                                         : nullptr),
    _adv_quant(
        isParamValid("advected_quantity")
            ? this->template getGenericMaterialProperty<Real, is_ad>("advected_quantity").get()
            : _u),
    _primal_dirichlet(
        isParamValid("primal_dirichlet_value") ? &getFunction("primal_dirichlet_value") : nullptr),
    _primal_coeff(this->template getGenericMaterialProperty<Real, is_ad>("primal_coefficient"))
{
  if (isParamSetByUser("primal_coefficient") && !_primal_dirichlet)
    paramError("primal_coefficient",
               "This parameter should only be provided when 'primal_dirichlet_value' is provided");
  if (static_cast<bool>(_primal_dirichlet) + isParamValid("advected_quantity") > 1)
    mooseError("Only one of 'primal_dirichlet_value' or 'advected_quantity' should be provided");
  if (static_cast<bool>(_velocity_mat_prop) + static_cast<bool>(_velocity_function) != 1)
    mooseError("Exactly one of 'velocity_mat_prop' or 'velocity_function' should be provided");
}

template <bool is_ad>
GenericReal<is_ad>
ConservativeAdvectionBCTempl<is_ad>::computeQpResidual()
{
  const auto vdotn =
      (_velocity_mat_prop
           ? (*_velocity_mat_prop)[_qp]
           : GenericRealVectorValue<is_ad>(_velocity_function->vectorValue(_t, _q_point[_qp]))) *
      this->_normals[_qp];

  if (_primal_dirichlet)
    return _test[_i][_qp] * vdotn * _primal_dirichlet->value(_t, _q_point[_qp]) *
           _primal_coeff[_qp];
  else
    return _test[_i][_qp] * vdotn * _adv_quant[_qp];
}
