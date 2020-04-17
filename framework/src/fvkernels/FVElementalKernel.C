//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVElementalKernel.h"
#include "MooseVariableFV.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "NonlinearSystemBase.h"

#include "libmesh/elem.h"

#include "metaphysicl/raw_type.h"

InputParameters
FVElementalKernel::validParams()
{
  InputParameters params = FVKernel::validParams();
  params.registerSystemAttributeName("FVElementalKernel");
  return params;
}

FVElementalKernel::FVElementalKernel(const InputParameters & parameters)
  : FVKernel(parameters),
    MooseVariableInterface(this,
                           false,
                           "variable",
                           Moose::VarKindType::VAR_NONLINEAR,
                           Moose::VarFieldType::VAR_FIELD_STANDARD),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    _var(*mooseVariableFV())
{
}

void
FVElementalKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());
  _local_re(0) += MetaPhysicL::raw_value(computeQpResidual() * _assembly.elem()->volume());
  accumulateTaggedLocalResidual();
}

void
FVElementalKernel::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());
  auto dofs_per_elem = _subproblem.systemBaseNonlinear().getMaxVarNDofsPerElem();
  size_t ad_offset = _var.number() * dofs_per_elem;
  auto vol = _assembly.elem()->volume();
  const auto r = computeQpResidual() * vol;
  _local_ke(0, 0) += r.derivatives()[ad_offset];
  accumulateTaggedLocalMatrix();
}

void
FVElementalKernel::computeOffDiagJacobian()
{
  const auto r = computeQpResidual() * _assembly.elem()->volume();

  auto & ce = _assembly.couplingEntries();
  for (const auto & it : ce)
  {
    MooseVariableFieldBase & ivariable = *(it.first);
    MooseVariableFieldBase & jvariable = *(it.second);

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    if (ivar != _var.number())
      continue;

    size_t ad_offset = jvar * _subproblem.systemBaseNonlinear().getMaxVarNDofsPerElem();

    prepareMatrixTag(_assembly, ivar, jvar);

    mooseAssert(
        _local_ke.m() == 1,
        "We are currently only supporting constant monomials for finite volume calculations");
    mooseAssert(
        _local_ke.n() == 1,
        "We are currently only supporting constant monomials for finite volume calculations");
    mooseAssert(jvariable.dofIndices().size() == 1,
                "The AD derivative indexing below only makes sense for constant monomials, e.g. "
                "for a number of dof indices equal to  1");

    _local_ke(0, 0) = r.derivatives()[ad_offset];

    accumulateTaggedLocalMatrix();
  }
}
