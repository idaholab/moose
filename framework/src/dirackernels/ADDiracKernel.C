//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDiracKernel.h"
#include "DiracKernelInfo.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "Problem.h"
#include "MooseMesh.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/quadrature.h"

InputParameters
ADDiracKernel::validParams()
{
  InputParameters params = DiracKernelBase::validParams();
  params += ADFunctorInterface::validParams();
  params.registerBase("DiracKernel");
  return params;
}

ADDiracKernel::ADDiracKernel(const InputParameters & parameters)
  : DiracKernelBase(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_SOLVER,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    ADFunctorInterface(this),
    _var(this->mooseVariableField()),
    _phi(_assembly.phi(_var)),
    _test(_var.phi()),
    _u(_var.adSln()),
    _grad_u(_var.adGradSln())
{
  _subproblem.haveADObjects(true);

  addMooseVariableDependency(&this->mooseVariableField());

  // Stateful material properties are not allowed on DiracKernels
  statefulPropertiesAllowed(false);
}

void
ADDiracKernel::jacobianSetup()
{
  _last_jacobian_elem = nullptr;
}

void
ADDiracKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  computeADResiduals();

  for (const auto i : index_range(_ad_residuals))
    _local_re(i) = raw_value(_ad_residuals[i]);

  accumulateTaggedLocalResidual();
}

void
ADDiracKernel::computeJacobian()
{
  computeFullJacobian();
}

void
ADDiracKernel::computeOffDiagJacobian(const unsigned int /*jvar*/)
{
  if (_last_jacobian_elem != _current_elem)
  {
    computeFullJacobian();
    _last_jacobian_elem = _current_elem;
  }
}

void
ADDiracKernel::computeFullJacobian()
{
  computeADResiduals();
  addJacobian(_assembly, _ad_residuals, _var.dofIndices(), _var.scalingFactor());
}

void
ADDiracKernel::computeResidualAndJacobian()
{
  computeADResiduals();
  addResidualsAndJacobian(_assembly, _ad_residuals, _var.dofIndices(), _var.scalingFactor());
}

void
ADDiracKernel::computeADResiduals()
{
  _ad_residuals.resize(_test.size());
  std::fill(_ad_residuals.begin(), _ad_residuals.end(), 0);

  precalculateResidual();

  const std::vector<unsigned int> * multiplicities =
      _drop_duplicate_points ? NULL : &_local_dirac_kernel_info.getPoints()[_current_elem].second;
  unsigned int local_qp = 0;
  Real multiplicity = 1.0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _current_point = _physical_point[_qp];
    if (isActiveAtPoint(_current_elem, _current_point))
    {
      if (!_drop_duplicate_points)
        multiplicity = (*multiplicities)[local_qp++];

      for (_i = 0; _i < _test.size(); _i++)
        _ad_residuals[_i] += multiplicity * computeQpResidual();
    }
  }
}
