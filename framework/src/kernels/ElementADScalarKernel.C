//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementADScalarKernel.h"

#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "ADUtils.h"
#include "SubProblem.h"

InputParameters
ElementADScalarKernel::validParams()
{
  InputParameters params = ScalarKernelBase::validParams();
  params += BlockRestrictable::validParams();
  params += MaterialPropertyInterface::validParams();
  params += ADFunctorInterface::validParams();
  return params;
}

ElementADScalarKernel::ElementADScalarKernel(const InputParameters & parameters)
  : ScalarKernelBase(parameters),
    BlockRestrictable(this),
    Coupleable(this, false),
    MooseVariableDependencyInterface(this),
    MaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS),
    ADFunctorInterface(this),
    _current_elem(_assembly.elem()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation()),
    _qrule(_assembly.qRule()),
    _qp(0),
    _h(0),
    _k_order(_var.order())
{
  _subproblem.haveADObjects(true);
}

void
ElementADScalarKernel::computeResidualOnElement()
{
  std::vector<Real> scalar_res(_k_order, 0);
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpResidual();
    for (_h = 0; _h < _k_order; _h++)
      scalar_res[_h] += _JxW[_qp] * _coord[_qp] * raw_value(computeScalarQpResidual());
  }
  addResiduals(_assembly, scalar_res, _var.dofIndices(), _var.scalingFactor());
}

void
ElementADScalarKernel::computeScalarResidualsForJacobian()
{
  if (_scalar_residuals.size() != _k_order)
    _scalar_residuals.resize(_k_order, 0);
  for (auto & r : _scalar_residuals)
    r = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpResidual();
    for (_h = 0; _h < _k_order; _h++)
      _scalar_residuals[_h] += _JxW[_qp] * _coord[_qp] * computeScalarQpResidual();
  }
}

void
ElementADScalarKernel::computeJacobianOnElement()
{
  computeScalarResidualsForJacobian();
  addJacobian(_assembly, _scalar_residuals, _var.dofIndices(), _var.scalingFactor());
}

void
ElementADScalarKernel::computeResidualAndJacobianOnElement()
{
  computeScalarResidualsForJacobian();
  addResidualsAndJacobian(_assembly, _scalar_residuals, _var.dofIndices(), _var.scalingFactor());
}
