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
  addResidualsAndJacobian(
      _assembly, std::array<ADReal, 1>{{r}}, _var.dofIndices(), _var.scalingFactor());
}

void
FVElementalKernel::computeJacobian()
{
  const auto r = computeQpResidual() * _assembly.elemVolume();

  mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");

  addJacobian(_assembly, std::array<ADReal, 1>{{r}}, _var.dofIndices(), _var.scalingFactor());
}

void
FVElementalKernel::computeOffDiagJacobian()
{
  computeJacobian();
}

void
FVElementalKernel::computeOffDiagJacobian(unsigned int)
{
  mooseError("FVElementalKernel::computeOffDiagJacobian should be called with no arguments");
}
