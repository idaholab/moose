//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCZMInterfaceKernelBase.h"

InputParameters
SCZMInterfaceKernelBase::validParams()
{
  InputParameters params = SBMInterfaceBase::validParams();

  params.addRequiredParam<unsigned int>("component",
                                        "The component of the displacement vector this kernel is "
                                        "working on: component == 0 => X, 1 => Y, 2 => Z");
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addRequiredCoupledVar("displacements", "The string containing displacement variables");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<std::string>("traction_global_name",
                               "traction_global",
                               "Name of the traction material property (global frame)");

  params.addParam<bool>("no_shifted", false, "Applying Shifted.");

  return params;
}

SCZMInterfaceKernelBase::SCZMInterfaceKernelBase(const InputParameters & parameters)
  : JvarMapKernelInterface<SBMInterfaceBase>(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _vars(_ndisp),
    _traction_global(getMaterialPropertyByName<RealVectorValue>(
        _base_name + getParam<std::string>("traction_global_name"))),
    _dtraction_djump_global(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "dtraction_djump_global")),
    _shifted(!getParam<bool>("no_shifted"))
{
  // Enforce consistency
  if (_ndisp != _mesh.dimension())
    paramError("displacements", "Number of displacements must match problem dimension.");

  if (_ndisp > 3 || _ndisp < 1)
    mooseError("the SCZM material requires 1, 2 or 3 displacement variables");

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp_var[i] = coupled("displacements", i);
    _vars[i] = getVar("displacements", i);
  }
}

Real
SCZMInterfaceKernelBase::computeQpResidual(Moose::DGResidualType type)
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
SCZMInterfaceKernelBase::computeQpJacobian(Moose::DGJacobianType type)
{
  // diagonal Jacobian coefficient for the displacement component this kernel works on
  return computeDResidualDDisplacement(_component, type);
}

Real
SCZMInterfaceKernelBase::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  // bail out if jvar is not coupled
  if (getJvarMap()[jvar] < 0)
    return 0.0;

  // Jacobian of residual[_component] w.r.t. coupled displacement component
  for (unsigned int off_diag_component = 0; off_diag_component < _ndisp; ++off_diag_component)
    if (jvar == _disp_var[off_diag_component])
      return computeDResidualDDisplacement(off_diag_component, type);

  return 0.0;
}
