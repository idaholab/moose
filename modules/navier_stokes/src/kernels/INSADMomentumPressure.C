//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumPressure.h"
#include "Assembly.h"
#include "INSADObjectTracker.h"
#include "FEProblemBase.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSADMomentumPressure);

InputParameters
INSADMomentumPressure::validParams()
{
  InputParameters params = ADVectorKernel::validParams();
  params.addClassDescription("Adds the pressure term to the INS momentum equation");
  params.addRequiredCoupledVar(NS::pressure, "The pressure");
  params.addParam<bool>(
      "integrate_p_by_parts", true, "Whether to integrate the pressure term by parts");
  return params;
}

INSADMomentumPressure::INSADMomentumPressure(const InputParameters & parameters)
  : ADVectorKernel(parameters),
    _integrate_p_by_parts(getParam<bool>("integrate_p_by_parts")),
    _p(adCoupledValue(NS::pressure)),
    _grad_p(adCoupledGradient(NS::pressure)),
    _coord_sys(_assembly.coordSystem()),
    _rz_radial_coord(_mesh.getAxisymmetricRadialCoord())
{
  // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
  // don't need
  auto & obj_tracker = const_cast<INSADObjectTracker &>(
      _fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker"));
  for (const auto block_id : blockIDs())
    obj_tracker.set("integrate_p_by_parts", _integrate_p_by_parts, block_id);
}

ADReal
INSADMomentumPressure::computeQpResidual()
{
  if (_integrate_p_by_parts)
  {
    ADReal residual = -_p[_qp] * _grad_test[_i][_qp].tr();
    if (_coord_sys == Moose::COORD_RZ)
    {
      const auto r_component_residual = -_p[_qp] / _ad_q_point[_qp](_rz_radial_coord);
      ADRealVectorValue rz_residual;
      rz_residual(_rz_radial_coord) = r_component_residual;
      residual += rz_residual * _test[_i][_qp];
    }
    return residual;
  }
  else
    return _test[_i][_qp] * _grad_p[_qp];
}
