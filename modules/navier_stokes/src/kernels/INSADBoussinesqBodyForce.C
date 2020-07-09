//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADBoussinesqBodyForce.h"
#include "INSADObjectTracker.h"
#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", INSADBoussinesqBodyForce);

InputParameters
INSADBoussinesqBodyForce::validParams()
{
  InputParameters params = ADVectorKernelValue::validParams();
  params.addClassDescription("Computes a body force for natural convection buoyancy.");
  params.addRequiredCoupledVar("temperature", "temperature variable, for off diagonal jacobian");
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  params.addParam<MaterialPropertyName>("alpha_name",
                                        "alpha",
                                        "The name of the thermal expansion coefficient"
                                        "this is of the form rho = rho*(1-alpha (T-T_ref))");
  params.addParam<MaterialPropertyName>(
      "ref_temp", "temp_ref", "The name of the reference temperature");
  return params;
}

INSADBoussinesqBodyForce::INSADBoussinesqBodyForce(const InputParameters & parameters)
  : ADVectorKernelValue(parameters),
    _boussinesq_strong_residual(
        getADMaterialProperty<RealVectorValue>("boussinesq_strong_residual"))
{
  // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
  // don't need
  auto & obj_tracker = const_cast<INSADObjectTracker &>(
      _fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker"));
  obj_tracker.set("has_boussinesq", true);
  obj_tracker.set("alpha", &getADMaterialProperty<Real>("alpha_name"));
  obj_tracker.set("ref_temp", &getMaterialProperty<Real>("ref_temp"));
  obj_tracker.set("temperature", &adCoupledValue("temperature"));
  obj_tracker.set("gravity", getParam<RealVectorValue>("gravity"));
}

ADRealVectorValue
INSADBoussinesqBodyForce::precomputeQpResidual()
{
  return _boussinesq_strong_residual[_qp];
}
