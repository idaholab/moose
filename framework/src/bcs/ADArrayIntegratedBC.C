//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADArrayIntegratedBC.h"

#include "Assembly.h"
#include "SubProblem.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

InputParameters
ADArrayIntegratedBC::validParams()
{
  InputParameters params = IntegratedBCBase::validParams();
  params += ADFunctorInterface::validParams();
  return params;
}

ADArrayIntegratedBC::ADArrayIntegratedBC(const InputParameters & parameters)
  : IntegratedBCBase(parameters),
    MooseVariableInterface<RealEigenVector>(this,
                                            false,
                                            "variable",
                                            Moose::VarKindType::VAR_SOLVER,
                                            Moose::VarFieldType::VAR_FIELD_ARRAY),
    ADFunctorInterface(this),
    _var(*mooseVariable()),
    _ad_JxW(_assembly.adJxWFace()),
    _ad_coord(_assembly.adCoordTransformation()),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh")),
    _normals(_assembly.adNormals()),
    _test(_var.phiFace()),
    _grad_test(_var.gradPhiFace()),
    _u(_var.adSlnFace()),
    _grad_u(_var.adGradSlnFace()),
    _count(_var.count()),
    _work_vector(_count)
{
  _subproblem.haveADObjects(true);
  addMooseVariableDependency(mooseVariable());

  if (!_save_in_strings.empty() || !_diag_save_in_strings.empty())
    paramError("save_in", "Save-in is not supported by AD array boundary conditions");
}

void
ADArrayIntegratedBC::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  // computeQpResidual() reads the current quadrature point and test function from _qp and _i
  for (const auto qp : make_range(_qrule->n_points()))
  {
    _qp = qp;
    initQpResidual();
    for (const auto i : index_range(_test))
    {
      _i = i;
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
ADArrayIntegratedBC::computeResidualsForJacobian()
{
  _local_ad_re.resize(_test.size() * _count);
  for (auto & residual : _local_ad_re)
    residual = 0;

  precalculateJacobian();
  for (const auto qp : make_range(_qrule->n_points()))
  {
    _qp = qp;
    // on a displaced mesh the surface measure depends on the displacements, so its derivatives
    // enter the Jacobian
    const ADReal jxw_c = _use_displaced_mesh ? ADReal(_ad_JxW[_qp] * _ad_coord[_qp])
                                             : ADReal(_JxW[_qp] * _coord[_qp]);
    initQpResidual();
    for (const auto i : index_range(_test))
    {
      _i = i;
      _work_vector.setZero();
      computeQpResidual(_work_vector);
      _work_vector *= jxw_c;
      _assembly.saveLocalADArray(_local_ad_re, _i, _test.size(), _work_vector);
    }
  }
}

void
ADArrayIntegratedBC::computeJacobian()
{
  computeResidualsForJacobian();
  addJacobian(_assembly, _local_ad_re, _var.dofIndices(), _var.arrayScalingFactor());
}

void
ADArrayIntegratedBC::computeResidualAndJacobian()
{
  computeResidualsForJacobian();
  addResiduals(_assembly, _local_ad_re, _var.dofIndices(), _var.arrayScalingFactor());
  addJacobian(_assembly, _local_ad_re, _var.dofIndices(), _var.arrayScalingFactor());
}

void
ADArrayIntegratedBC::computeOffDiagJacobian(const unsigned int jvar)
{
  // AD computes the derivatives with respect to all variables at once
  if (jvar == _var.number())
    computeJacobian();
}

void
ADArrayIntegratedBC::computeOffDiagJacobianScalar(unsigned int)
{
}
