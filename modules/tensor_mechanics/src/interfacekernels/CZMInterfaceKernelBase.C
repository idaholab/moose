//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMInterfaceKernelBase.h"

InputParameters
CZMInterfaceKernelBase::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
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

  return params;
}

CZMInterfaceKernelBase::CZMInterfaceKernelBase(const InputParameters & parameters)
  : JvarMapKernelInterface<InterfaceKernel>(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _disp_neighbor_var(_ndisp),
    _vars(_ndisp),
    _traction_global(getMaterialPropertyByName<RealVectorValue>(
        _base_name + getParam<std::string>("traction_global_name"))),
    _dtraction_djump_global(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "dtraction_djump_global"))
{
  // Enforce consistency
  if (_ndisp != _mesh.dimension())
    paramError("displacements", "Number of displacements must match problem dimension.");

  if (_ndisp > 3 || _ndisp < 1)
    mooseError("the CZM material requires 1, 2 or 3 displacement variables");

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp_var[i] = coupled("displacements", i);
    _disp_neighbor_var[i] = coupled("displacements", i);
    _vars[i] = getVar("displacements", i);
  }
}

Real
CZMInterfaceKernelBase::computeQpResidual(Moose::DGResidualType type)
{
  Real r = _traction_global[_qp](_component);

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

Real
CZMInterfaceKernelBase::computeQpJacobian(Moose::DGJacobianType type)
{
  // retrieve the diagonal Jacobian coefficient dependning on the displacement
  // component (_component) this kernel is working on
  return computeDResidualDDisplacement(_component, type);
}

Real
CZMInterfaceKernelBase::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  // bail out if jvar is not coupled
  if (getJvarMap()[jvar] < 0)
    return 0.0;

  // Jacobian of the residul[_component] w.r.t to the coupled displacement
  // component[off_diag_component]
  for (unsigned int off_diag_component = 0; off_diag_component < _ndisp; ++off_diag_component)
  {
    if (jvar == _disp_var[off_diag_component])
      return computeDResidualDDisplacement(off_diag_component, type);
  }
  // this is the place where one should implement derivatives of the residual w.r.t. other variables
  return 0.0;
}
