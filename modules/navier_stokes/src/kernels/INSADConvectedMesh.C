//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADConvectedMesh.h"
#include "INSADObjectTracker.h"

registerMooseObject("NavierStokesApp", INSADConvectedMesh);

InputParameters
INSADConvectedMesh::validParams()
{
  InputParameters params = ADVectorKernelValue::validParams();
  params.addClassDescription(
      "Corrects the convective derivative for situations in which the fluid mesh is dynamic.");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  return params;
}

INSADConvectedMesh::INSADConvectedMesh(const InputParameters & parameters)
  : ADVectorKernelValue(parameters),
    _convected_mesh_strong_residual(
        getADMaterialProperty<RealVectorValue>("convected_mesh_strong_residual"))
{
  auto check_coupled = [&](const auto & var_name)
  {
    if (coupledComponents(var_name) > 1)
      paramError(var_name, "Only one variable should be used for '", var_name, "'");
    if (isCoupledConstant(var_name))
      paramError(var_name, "Displacement variables cannot be constants");
  };
  check_coupled("disp_x");
  check_coupled("disp_y");
  check_coupled("disp_z");

  if (_tid == 0)
  {
    // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
    // don't need
    auto & obj_tracker = const_cast<INSADObjectTracker &>(
        _fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker"));
    for (const auto block_id : blockIDs())
    {
      obj_tracker.set("has_convected_mesh", true, block_id);
      obj_tracker.set("disp_x", getParam<std::vector<VariableName>>("disp_x").front(), block_id);
      if (isParamValid("disp_y"))
        obj_tracker.set("disp_y", getParam<std::vector<VariableName>>("disp_y").front(), block_id);
      if (isParamValid("disp_z"))
        obj_tracker.set("disp_z", getParam<std::vector<VariableName>>("disp_z").front(), block_id);
    }
  }
}

ADRealVectorValue
INSADConvectedMesh::precomputeQpResidual()
{
  return _convected_mesh_strong_residual[_qp];
}
