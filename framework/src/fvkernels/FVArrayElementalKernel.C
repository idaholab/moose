//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVArrayElementalKernel.h"
#include "MooseVariableFV.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "NonlinearSystemBase.h"
#include "ADUtils.h"

#include "libmesh/elem.h"

InputParameters
FVArrayElementalKernel::validParams()
{
  InputParameters params = FVKernel::validParams();
  params.registerSystemAttributeName("FVArrayElementalKernel");
  params += MaterialPropertyInterface::validParams();
  return params;
}

FVArrayElementalKernel::FVArrayElementalKernel(const InputParameters & parameters)
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
FVArrayElementalKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());
  const auto r = computeQpResidual() * _assembly.elemVolume();
  _assembly.saveLocalArrayResidual(_local_re, 0, 1, r);
  accumulateTaggedLocalResidual();
}

void
FVArrayElementalKernel::computeJacobian()
{
  const auto r = computeQpResidual() * _assembly.elemVolume();

  mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");

  auto local_functor = [&](const ADReal &, dof_id_type, const std::set<TagID> &) {
    mooseError("FV Array variables do not support non-global AD DOF indexing");
  };

  for (unsigned int v = 0; v < _var.count(); v++)
  {
    auto dof = _var.dofIndices()[0];
    const unsigned int ndofs = 1;
    _assembly.processDerivatives(
        r(v), Moose::globalADArrayOffset(dof, ndofs, v), _matrix_tags, local_functor);
  }
}
