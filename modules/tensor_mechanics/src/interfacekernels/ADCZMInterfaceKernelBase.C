//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCZMInterfaceKernelBase.h"
#include "CZMInterfaceKernelBase.h"

InputParameters
ADCZMInterfaceKernelBase::validParams()
{
  InputParameters params = CZMInterfaceKernelBase::validParams();
  return params;
}

ADCZMInterfaceKernelBase::ADCZMInterfaceKernelBase(const InputParameters & parameters)
  : ADInterfaceKernel(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _traction_global(getADMaterialPropertyByName<RealVectorValue>(
        _base_name + getParam<std::string>("traction_global_name")))
{
  // Enforce consistency
  if (_ndisp != _mesh.dimension())
    paramError("displacements", "Number of displacements must match problem dimension.");

  if (_ndisp > 3 || _ndisp < 1)
    mooseError("the CZM material requires 1, 2 or 3 displacement variables");
}

ADReal
ADCZMInterfaceKernelBase::computeQpResidual(Moose::DGResidualType type)
{
  ADReal r = _traction_global[_qp](_component);

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
