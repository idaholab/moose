//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumCoupledForce.h"
#include "INSADObjectTracker.h"
#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", INSADMomentumCoupledForce);

InputParameters
INSADMomentumCoupledForce::validParams()
{
  InputParameters params = ADVectorKernelValue::validParams();
  params.addClassDescription(
      "Computes a body force due to a coupled vector variable or a vector function");
  params.addCoupledVar(
      "coupled_vector_var",
      "The coupled vector variable applying the force. Positive variable components represent "
      "momentum sources in that component direction, e.g. if the x-component is positive then this "
      "object imposes a momentum source in the +x direction. Multiple variable names can be "
      "provided; the result will be a summed force.");
  params.addParam<std::vector<FunctionName>>(
      "vector_function",
      "A vector function which can be used to stand-in for the 'coupled_vector_var' param. "
      "Multiple function names can be provided; the result will be a summed force");
  return params;
}

INSADMomentumCoupledForce::INSADMomentumCoupledForce(const InputParameters & parameters)
  : ADVectorKernelValue(parameters),
    _coupled_force_strong_residual(
        getADMaterialProperty<RealVectorValue>("coupled_force_strong_residual"))
{
  bool has_coupled = isCoupled("coupled_vector_var");
  bool has_function = isParamValid("vector_function");
  if (!has_coupled && !has_function)
    mooseError("Either the 'coupled_vector_var' or 'vector_function' param must be set for the "
               "'INSADMomentumCoupledForce' object");

  // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
  // don't need
  auto & obj_tracker = const_cast<INSADObjectTracker &>(
      _fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker"));
  for (const auto block_id : blockIDs())
  {
    obj_tracker.set("has_coupled_force", true, block_id);
    if (has_coupled)
      obj_tracker.set(
          "coupled_force_var", getParam<std::vector<VariableName>>("coupled_vector_var"), block_id);
    if (has_function)
      obj_tracker.set("coupled_force_vector_function",
                      getParam<std::vector<FunctionName>>("vector_function"),
                      block_id);
  }
}

ADRealVectorValue
INSADMomentumCoupledForce::precomputeQpResidual()
{
  return _coupled_force_strong_residual[_qp];
}
