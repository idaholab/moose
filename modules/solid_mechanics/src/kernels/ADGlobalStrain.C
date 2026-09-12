//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "ADGlobalStrain.h"
#include "GlobalStrainPeriodicDirUserObject.h"
#include "RankTwoTensor.h"

registerMooseObject("SolidMechanicsApp", ADGlobalStrain);

InputParameters
ADGlobalStrain::validParams()
{
  InputParameters params = ADKernelScalarBase::validParams();
  params.addClassDescription(
      "Assembles either the diagonal or off-diagonal volume-integrated global-stress equations "
      "and obtains all Jacobian blocks using automatic differentiation.");

  params.renameCoupledVar("scalar_variable",
                          "scalar_global_strain",
                          "Diagonal or off-diagonal scalar global-strain variable");
  params.makeParamRequired<std::vector<VariableName>>("scalar_global_strain");

  MooseEnum component_type("diagonal off_diagonal");
  params.addRequiredParam<MooseEnum>(
      "component_type", component_type, "Select the stress-component group assembled here");

  params.addRequiredParam<UserObjectName>(
      "global_strain_uo", "UserObject that supplies the translated-periodic directions");
  params.addParam<std::vector<Real>>(
      "applied_stress_tensor", "Constant applied stress in MOOSE order 11, 22, 33, 23, 13, 12");
  params.addParam<std::string>("base_name", "Optional material-property base name");

  // This Kernel exists only to execute a QP loop for the scalar residual.
  params.set<bool>("compute_field_residuals") = false;
  params.suppressParameter<bool>("compute_field_residuals");

  return params;
}

ADGlobalStrain::ADGlobalStrain(const InputParameters & parameters)
  : ADKernelScalarBase(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(getADMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _periodicity_uo(getUserObject<GlobalStrainPeriodicDirUserObject>("global_strain_uo")),
    _periodic_dir(_periodicity_uo.getPeriodicDirections()),
    _use_diagonal_components(getParam<MooseEnum>("component_type") == "diagonal"),
    _components(),
    _dim(_mesh.dimension()),
    _applied_stress_tensor()
{
  assignComponentIndices();

  if (_components.empty())
    paramError("component_type",
               "The selected component group contains no active global-strain component.");

  if (_components.size() != _k_order)
    paramError("scalar_global_strain",
               "The scalar variable contains ",
               _k_order,
               " component(s), but component_type=",
               _use_diagonal_components ? "diagonal" : "off_diagonal",
               " requires ",
               _components.size(),
               " component(s) for the detected periodic directions.");

  if (isParamValid("applied_stress_tensor"))
    _applied_stress_tensor.fillFromInputVector(
        getParam<std::vector<Real>>("applied_stress_tensor"));
  else
    _applied_stress_tensor.zero();
}

void
ADGlobalStrain::assignComponentIndices()
{
  _components.clear();

  if (_use_diagonal_components)
  {
    // Diagonal ordering: xx, yy, zz, with nonperiodic entries removed.
    for (unsigned int i = 0; i < _dim; ++i)
      if (_periodic_dir(i))
        _components.emplace_back(i, i);
  }
  else
  {
    // Off-diagonal ordering: xy, xz, yz, with inactive entries removed.
    if (_dim >= 2 && (_periodic_dir(0) || _periodic_dir(1)))
      _components.emplace_back(0, 1);

    if (_dim >= 3 && (_periodic_dir(0) || _periodic_dir(2)))
      _components.emplace_back(0, 2);

    if (_dim >= 3 && (_periodic_dir(1) || _periodic_dir(2)))
      _components.emplace_back(1, 2);
  }
}

ADReal
ADGlobalStrain::computeQpResidual()
{
  return 0.0;
}

ADReal
ADGlobalStrain::computeScalarQpResidual()
{
  mooseAssert(_h < _components.size(),
              "ADKernelScalarBase supplied an invalid scalar component index");

  const auto & indices = _components[_h];
  return _stress[_qp](indices.first, indices.second) -
         _applied_stress_tensor(indices.first, indices.second);
}
