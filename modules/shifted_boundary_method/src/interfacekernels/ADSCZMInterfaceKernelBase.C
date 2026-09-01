//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSCZMInterfaceKernelBase.h"

InputParameters
ADSCZMInterfaceKernelBase::validParams()
{
  InputParameters params = ADSBMInterfaceBase::validParams();
  params.addRequiredParam<unsigned int>("component",
                                        "the component of the "
                                        "displacement vector this kernel is working on:"
                                        " component == 0, ==> X"
                                        " component == 1, ==> Y"
                                        " component == 2, ==> Z");
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addRequiredCoupledVar("displacements", "the string containing displacement variables");
  params.addParam<std::string>("base_name", "Material property base name");
  params.set<std::string>("traction_global_name") = "traction_global";

  params.addParam<bool>("no_shifted", false, "Applying Shifted.");

  return params;
}

ADSCZMInterfaceKernelBase::ADSCZMInterfaceKernelBase(const InputParameters & parameters)
  : JvarMapKernelInterface<ADSBMInterfaceBase>(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _traction_global(getADMaterialPropertyByName<RealVectorValue>(
        _base_name + getParam<std::string>("traction_global_name"))),
    _shifted(!getParam<bool>("no_shifted"))
{
  // Enforce consistency
  if (_ndisp != _mesh.dimension())
    paramError("displacements", "Number of displacements must match problem dimension.");

  if (_ndisp > 3 || _ndisp < 1)
    mooseError("the CZM material requires 1, 2 or 3 displacement variables");
}

ADReal
ADSCZMInterfaceKernelBase::computeQpResidual(Moose::DGResidualType type)
{
  auto r = _traction_global[_qp](_component);

  switch (type)
  {
    // [test_secondary-test_primary]*T where T represents the traction.
    case Moose::Element:
      r *= -_test[_i][_qp];
      break;
    case Moose::Neighbor:
      r *= _test_neighbor[_i][_qp];
      break;
  }
  return r;
}
