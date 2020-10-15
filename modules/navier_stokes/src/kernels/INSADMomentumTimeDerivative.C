//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumTimeDerivative.h"
#include "INSADObjectTracker.h"
#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", INSADMomentumTimeDerivative);

InputParameters
INSADMomentumTimeDerivative::validParams()
{
  InputParameters params = ADTimeKernelValue::validParams();
  params.addClassDescription("This class computes the time derivative for the incompressible "
                             "Navier-Stokes momentum equation.");
  params.addCoupledVar("temperature",
                       "The temperature on which material properties may depend. If properties "
                       "do depend on temperature, this variable must be coupled in in order to "
                       "correctly resize the element matrix");
  return params;
}

INSADMomentumTimeDerivative::INSADMomentumTimeDerivative(const InputParameters & parameters)
  : ADVectorTimeKernelValue(parameters),
    _td_strong_residual(getADMaterialProperty<RealVectorValue>("td_strong_residual"))
{
  // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
  // don't need
  auto & obj_tracker = const_cast<INSADObjectTracker &>(
      _fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker"));
  for (const auto block_id : blockIDs())
    obj_tracker.set("has_transient", true, block_id);
}

ADRealVectorValue
INSADMomentumTimeDerivative::precomputeQpResidual()
{
  return _td_strong_residual[_qp];
}
