//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCZMInterfaceKernelBase.h"

template <bool is_ad>
InputParameters
SCZMInterfaceKernelBaseTempl<is_ad>::validParams()
{
  InputParameters params = SBMInterfaceBase<is_ad>::validParams();

  params.addRequiredParam<unsigned int>("component",
                                        "The component of the displacement vector this kernel is "
                                        "working on: component == 0 => X, 1 => Y, 2 => Z");
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addRequiredCoupledVar("displacements", "The string containing displacement variables");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<std::string>("traction_global_name",
                               "traction_global",
                               "Name of the traction material property (global frame)");
  params.addParam<bool>(
      "no_shifted",
      false,
      "Disable the shifted integration corrections while retaining the same interface kernel and "
      "cohesive-zone model. Shifted integration is enabled by default and is the intended SBM "
      "mode; disable it only to isolate the effect of the correction terms for verification and "
      "comparison.");

  return params;
}

template <bool is_ad>
SCZMInterfaceKernelBaseTempl<is_ad>::SCZMInterfaceKernelBaseTempl(
    const InputParameters & parameters)
  : JvarMapKernelInterface<SBMInterfaceBase<is_ad>>(parameters),
    _base_name(this->isParamValid("base_name") &&
                       !this->template getParam<std::string>("base_name").empty()
                   ? this->template getParam<std::string>("base_name") + "_"
                   : ""),
    _component(this->template getParam<unsigned int>("component")),
    _ndisp(this->coupledComponents("displacements")),
    _traction_global(this->template getGenericMaterialPropertyByName<RealVectorValue, is_ad>(
        _base_name + this->template getParam<std::string>("traction_global_name"))),
    _shifted(!this->template getParam<bool>("no_shifted"))
{
  if (_ndisp != this->_mesh.dimension())
    this->paramError("displacements", "Number of displacements must match problem dimension.");

  if (_ndisp > 3 || _ndisp < 1)
    this->paramError("displacements",
                     "The SCZM material requires 1, 2, or 3 displacement variables.");
}

template <bool is_ad>
GenericReal<is_ad>
SCZMInterfaceKernelBaseTempl<is_ad>::computeQpResidual(Moose::DGResidualType type)
{
  auto residual = _traction_global[this->_qp](_component);

  switch (type)
  {
    case Moose::Element:
      residual *= -this->_test[this->_i][this->_qp];
      break;

    case Moose::Neighbor:
      residual *= this->_test_neighbor[this->_i][this->_qp];
      break;
  }

  return residual;
}

template class SCZMInterfaceKernelBaseTempl<false>;
template class SCZMInterfaceKernelBaseTempl<true>;
