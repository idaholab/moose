//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * Subtracts the mesh velocity from the convection term in the Navier-Stokes momentum equation
 */
class INSADMomentumMeshAdvection : public ADVectorKernelValue
{
public:
  static InputParameters validParams();
  static InputParameters displacementParams();
  template <typename T>
  static void setDisplacementParams(T & mesh_convection_obj);

  INSADMomentumMeshAdvection(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  /// The strong residual for this object, computed by material classes to eliminate computation
  /// duplication in simulations in which we have stabilization
  const ADMaterialProperty<RealVectorValue> & _convected_mesh_strong_residual;
};

template <typename T>
void
INSADMomentumMeshAdvection::setDisplacementParams(T & mesh_convection_obj)
{
  auto check_coupled = [&](const auto & var_name)
  {
    if (mesh_convection_obj.coupledComponents(var_name) > 1)
      mesh_convection_obj.paramError(
          var_name, "Only one variable should be used for '", var_name, "'");
    if (mesh_convection_obj.isCoupledConstant(var_name))
      mesh_convection_obj.paramError(var_name, "Displacement variables cannot be constants");
  };
  check_coupled("disp_x");
  check_coupled("disp_y");
  check_coupled("disp_z");

  if (mesh_convection_obj._tid == 0)
  {
    // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
    // don't need
    auto & obj_tracker = mesh_convection_obj._fe_problem.template getUserObject<INSADObjectTracker>(
        "ins_ad_object_tracker");
    for (const auto block_id : mesh_convection_obj.blockIDs())
    {
      obj_tracker.set("has_convected_mesh", true, block_id);
      obj_tracker.set("disp_x", mesh_convection_obj.coupledName("disp_x"), block_id);
      if (mesh_convection_obj.isParamValid("disp_y"))
        obj_tracker.set("disp_y", mesh_convection_obj.coupledName("disp_y"), block_id);
      if (mesh_convection_obj.isParamValid("disp_z"))
        obj_tracker.set("disp_z", mesh_convection_obj.coupledName("disp_z"), block_id);
    }
  }
}
