//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRayKernel.h"

// Local includes
#include "RayTracingStudy.h"

// MOOSE includes
#include "Assembly.h"
#include "NonlinearSystemBase.h"
#include "ADUtils.h"

template <typename T>
InputParameters
ADRayKernelTempl<T>::validParams()
{
  auto params = IntegralRayKernelBase::validParams();
  params += TaggingInterface::validParams();

  params.template addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this ADRayKernel operates on");

  return params;
}

template <typename T>
ADRayKernelTempl<T>::ADRayKernelTempl(const InputParameters & params)
  : IntegralRayKernelBase(params),
    MooseVariableInterface<T>(this,
                              false,
                              "variable",
                              Moose::VarKindType::VAR_NONLINEAR,
                              std::is_same<T, Real>::value ? Moose::VarFieldType::VAR_FIELD_STANDARD
                                                           : Moose::VarFieldType::VAR_FIELD_VECTOR),
    TaggingInterface(this),
    _var(this->mooseVariableField()),
    _test(_var.phi()),
    _u(_var.adSln()),
    _grad_u(_var.adGradSln()),
    _phi(_assembly.phi(_var))
{
  // We do not allow RZ/RSPHERICAL because in the context of these coord
  // systems there is no way to represent a line source - we would end up
  // with a plane/surface source or a volumetric source, respectively.
  // This is also why we do not multiply by _coord[_qp] in any of the
  // integrations that follow.
  for (const auto & subdomain_id : _mesh.meshSubdomains())
    if (_fe_problem.getCoordSystem(subdomain_id) != Moose::COORD_XYZ)
      mooseError("Not valid on coordinate systems other than XYZ");

  _subproblem.haveADObjects(true);

  addMooseVariableDependency(&variable());

  if (!isImplicit())
    mooseError("ADRayKernels do not currently support explicit solves.");
}

template <typename T>
void
ADRayKernelTempl<T>::onSegment()
{
  mooseAssert(_current_subdomain_id == _assembly.currentSubdomainID(), "Subdomain IDs not in sync");
  mooseAssert(_fe_problem.currentlyComputingJacobian() || _fe_problem.currentlyComputingResidual(),
              "Not computing residual or Jacobian");

  if (_fe_problem.currentlyComputingJacobian())
    computeJacobian();
  else if (_fe_problem.currentlyComputingResidual())
    computeResidual();
}

template <typename T>
void
ADRayKernelTempl<T>::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  for (_qp = 0; _qp < _JxW.size(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      _local_re(_i) += raw_value(_JxW[_qp] * computeQpResidual());

  accumulateTaggedLocalResidual();
}

template <typename T>
void
ADRayKernelTempl<T>::computeJacobian()
{
  _subproblem.prepareShapes(_var.number(), _tid);

  for (auto & r : _residuals)
    r = 0;
  _residuals.resize(_test.size(), 0);

  precalculateResidual();
  for (_qp = 0; _qp < _JxW.size(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      _residuals[_i] += _JxW[_qp] * computeQpResidual();

  addJacobian(_assembly, _residuals, _var.dofIndices(), _var.scalingFactor());
}

template class ADRayKernelTempl<Real>;

// Not implementing this until there is a use case and tests for it!
// template class ADRayKernelTempl<RealVectorValue>;
