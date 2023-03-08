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
#include "ADUtils.h"

#include "libmesh/elem.h"

#include "metaphysicl/raw_type.h"

InputParameters
FVElementalKernel::validParams()
{
  InputParameters params = FVKernel::validParams();
  params.registerSystemAttributeName("FVElementalKernel");
  params += MaterialPropertyInterface::validParams();
  return params;
}

FVElementalKernel::FVElementalKernel(const InputParameters & parameters)
  : FVKernel(parameters),
    MooseVariableInterface(this,
                           false,
                           "variable",
                           Moose::VarKindType::VAR_NONLINEAR,
                           Moose::VarFieldType::VAR_FIELD_STANDARD),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false, /*is_fv=*/true),
    MaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS),
    _var(*mooseVariableFV()),
    _u(_var.adSln()),
    _u_functor(getFunctor<ADReal>(_var.name())),
    _current_elem(_assembly.elem()),
    _q_point(_assembly.qPoints())
{
  addMooseVariableDependency(&_var);
}

// Note the lack of quadrature point loops in the residual/jacobian compute
// functions. This is because finite volumes currently only works with
// constant monomial elements. We only have one quadrature point regardless of
// problem dimension and just multiply by the element volume.

void
FVElementalKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());
  _local_re(0) += MetaPhysicL::raw_value(computeQpResidual() * _assembly.elemVolume());
  accumulateTaggedLocalResidual();
}

void
FVElementalKernel::computeResidualAndJacobian()
{
  const auto r = computeQpResidual() * _assembly.elemVolume();
  const auto dof_index = _var.dofIndices()[0];
  _assembly.processJacobian(r, dof_index, _matrix_tags);
  _assembly.processResidual(r.value(), dof_index, _vector_tags);
}

void
FVElementalKernel::computeJacobian()
{
  const auto r = computeQpResidual() * _assembly.elemVolume();

  mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");

  auto local_functor = [&](const ADReal & residual, dof_id_type, const std::set<TagID> &)
  {
    prepareMatrixTag(_assembly, _var.number(), _var.number());
    auto dofs_per_elem = _sys.getMaxVarNDofsPerElem();
    auto ad_offset = Moose::adOffset(_var.number(), dofs_per_elem);
    _local_ke(0, 0) += residual.derivatives()[ad_offset];
    accumulateTaggedLocalMatrix();
  };

  _assembly.processJacobian(r, _var.dofIndices()[0], _matrix_tags, local_functor);
}

void
FVElementalKernel::computeOffDiagJacobian()
{
  const auto r = computeQpResidual() * _assembly.elemVolume();

  mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");

  auto local_functor = [&](const ADReal & residual, dof_id_type, const std::set<TagID> &)
  {
    auto & ce = _assembly.couplingEntries();
    for (const auto & it : ce)
    {
      MooseVariableFieldBase & ivariable = *(it.first);
      MooseVariableFieldBase & jvariable = *(it.second);

      // We currently only support coupling to other FV variables
      if (!jvariable.isFV() || !jvariable.activeOnSubdomain(_current_elem->subdomain_id()))
        continue;

      unsigned int ivar = ivariable.number();
      unsigned int jvar = jvariable.number();

      if (ivar != _var.number())
        continue;

      auto ad_offset = Moose::adOffset(jvar, _sys.getMaxVarNDofsPerElem());

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

      _local_ke(0, 0) = residual.derivatives()[ad_offset];

      accumulateTaggedLocalMatrix();
    }
  };

  _assembly.processJacobian(r, _var.dofIndices()[0], _matrix_tags, local_functor);
}

void
FVElementalKernel::computeOffDiagJacobian(unsigned int)
{
  mooseError("FVElementalKernel::computeOffDiagJacobian should be called with no arguments");
}
