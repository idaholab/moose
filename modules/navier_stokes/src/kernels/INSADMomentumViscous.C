//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumViscous.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "INSADObjectTracker.h"

#include "metaphysicl/raw_type.h"

using MetaPhysicL::raw_value;

registerMooseObject("NavierStokesApp", INSADMomentumViscous);

InputParameters
INSADMomentumViscous::validParams()
{
  InputParameters params = ADVectorKernel::validParams();
  params.addClassDescription("Adds the viscous term to the INS momentum equation");
  params.addParam<MaterialPropertyName>(
      "mu_name", "mu", "The name of the viscosity material property");
  MooseEnum viscous_form("traction laplace", "laplace");
  params.addParam<MooseEnum>("viscous_form",
                             viscous_form,
                             "The form of the viscous term. Options are 'traction' or 'laplace'");
  return params;
}

INSADMomentumViscous::INSADMomentumViscous(const InputParameters & parameters)
  : ADVectorKernel(parameters),
    _mu(getADMaterialProperty<Real>("mu_name")),
    _coord_sys(_assembly.coordSystem()),
    _form(getParam<MooseEnum>("viscous_form"))
{
  auto & obj_tracker = const_cast<INSADObjectTracker &>(
      _fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker"));
  for (const auto block_id : blockIDs())
    obj_tracker.set("viscous_form", _form, block_id);
}

ADRealTensorValue
INSADMomentumViscous::qpViscousTerm()
{
  if (_form == "laplace")
    return _mu[_qp] * _grad_u[_qp];
  else
    return _mu[_qp] * (_grad_u[_qp] + _grad_u[_qp].transpose());
}

ADRealVectorValue
INSADMomentumViscous::qpAdditionalRZTerm()
{
  // Add the u_r / r^2 term. There is an extra factor of 2 for the traction form
  ADReal resid = _mu[_qp] * _u[_qp](0);
  if (_form == "traction")
    resid *= 2.;

  if (_use_displaced_mesh)
    return resid / (_ad_q_point[_qp](0) * _ad_q_point[_qp](0));
  else
    return resid / (_q_point[_qp](0) * _q_point[_qp](0));
}

void
INSADMomentumViscous::computeResidual()
{
  // When computing the residual we use the regular version of all quantities, e.g. JxW, coord,
  // test, grad_test, because we do not need to track the derivatives

  prepareVectorTag(_assembly, _var.number());

  const unsigned int n_test = _grad_test.size();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    const RealTensorValue value = raw_value(qpViscousTerm()) * _JxW[_qp] * _coord[_qp];
    for (_i = 0; _i < n_test; _i++) // target for auto vectorization
      _local_re(_i) += MathUtils::dotProduct(value, _regular_grad_test[_i][_qp]);

    // If we're in RZ, then we need to add an additional term
    if (_coord_sys == Moose::COORD_RZ)
    {
      const RealVectorValue rz_value = raw_value(qpAdditionalRZTerm()) * _JxW[_qp] * _coord[_qp];

      for (_i = 0; _i < n_test; _i++) // target for auto vectorization
        _local_re(_i) += rz_value * _test[_i][_qp];
    }
  }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
INSADMomentumViscous::computeResidualsForJacobian()
{
  mooseAssert(_test.size() == _grad_test.size(),
              "How are the there are different number of test and grad_test objects?");
  const auto n_test = _test.size();

  if (_residuals.size() != n_test)
    _residuals.resize(n_test, 0);
  for (auto & r : _residuals)
    r = 0;

  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      // This will also compute the derivative with respect to all dofs
      const ADRealTensorValue value = qpViscousTerm() * _ad_JxW[_qp] * _ad_coord[_qp];
      for (_i = 0; _i < _grad_test.size(); _i++)
        _residuals[_i] += MathUtils::dotProduct(value, _grad_test[_i][_qp]);

      // If we're in RZ, then we need to add an additional term
      if (_coord_sys == Moose::COORD_RZ)
      {
        const ADRealVectorValue rz_value = qpAdditionalRZTerm() * _ad_JxW[_qp] * _ad_coord[_qp];

        for (_i = 0; _i < n_test; _i++)
          _residuals[_i] += rz_value * _test[_i][_qp];
      }
    }
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      // Compute scalar quanitities up front to reduce DualNumber operations
      const ADRealTensorValue value = _JxW[_qp] * _coord[_qp] * qpViscousTerm();
      for (_i = 0; _i < _grad_test.size(); _i++)
        _residuals[_i] += MathUtils::dotProduct(value, _regular_grad_test[_i][_qp]);

      // If we're in RZ, then we need to add an additional term
      if (_coord_sys == Moose::COORD_RZ)
      {
        // Compute scalar quanitities up front to reduce DualNumber operations
        const ADRealVectorValue rz_value = _JxW[_qp] * _coord[_qp] * qpAdditionalRZTerm();
        for (_i = 0; _i < n_test; _i++)
          _residuals[_i] += rz_value * _test[_i][_qp];
      }
    }
}

ADReal
INSADMomentumViscous::computeQpResidual()
{
  mooseError("computeQpResidual is not used in the INSADMomentumViscous class");
}
