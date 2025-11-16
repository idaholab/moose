//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADArrayKernel.h"

#include "Assembly.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"

#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

InputParameters
ADArrayKernel::validParams()
{
  InputParameters params = KernelBase::validParams();
  params.registerBase("ADArrayKernel");
  return params;
}

ADArrayKernel::ADArrayKernel(const InputParameters & parameters)
  : KernelBase(parameters),
    MooseVariableInterface<RealEigenVector>(this,
                                            false,
                                            "variable",
                                            Moose::VarKindType::VAR_SOLVER,
                                            Moose::VarFieldType::VAR_FIELD_ARRAY),
    ADFunctorInterface(this),
    _var(*mooseVariable()),
    _test(_var.phi()),
    _grad_test(_var.gradPhi()),
    _array_grad_test(_var.arrayGradPhi()),
    _phi(_assembly.phi(_var)),
    _grad_phi(_assembly.gradPhi(_var)),
    _u(_var.adSln()),
    _grad_u(_var.adGradSln()),
    _count(_var.count()),
    _work_vector(_count)
{
  _subproblem.haveADObjects(true);
  addMooseVariableDependency(mooseVariable());
}

void
ADArrayKernel::computeResidual()
{

  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initQpResidual();
    for (_i = 0; _i < _test.size(); _i++)
    {
      _work_vector.setZero();
      computeQpResidual(_work_vector);
      auto raw_work_vector = MetaPhysicL::raw_value(_work_vector);
      raw_work_vector *= _JxW[_qp] * _coord[_qp];
      _assembly.saveLocalArrayResidual(_local_re, _i, _test.size(), raw_work_vector);
    }
  }

  accumulateTaggedLocalResidual();
}

void
ADArrayKernel::computeJacobian()
{
  _local_ad_re.resize(_test.size() * _count);
  for (auto & residual : _local_ad_re)
    residual = 0;

  precalculateJacobian();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initQpResidual();
    for (_i = 0; _i < _test.size(); _i++)
    {
      computeQpResidual(_work_vector);
      _work_vector *= _JxW[_qp] * _coord[_qp];
      _assembly.saveLocalADArray(_local_ad_re, _i, _test.size(), _work_vector);
    }
  }

  addJacobian(_assembly, _local_ad_re, _var.dofIndices(), _var.arrayScalingFactor());
}

void
ADArrayKernel::jacobianSetup()
{
  _my_elem = nullptr;
}

void
ADArrayKernel::computeOffDiagJacobian(const unsigned int)
{
  if (_my_elem != _current_elem)
  {
    computeJacobian();
    _my_elem = _current_elem;
  }
}
