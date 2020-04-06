//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVTimeKernel.h"

InputParameters
FVElementalKernel::validParams()
{
  InputParameters params = FVKernel::validParams();

  // TODO: this will need to eventually override/register a "FVElementalKernel"
  // system string with the params instead of just using the parent class's
  // FVKernel - this is a new feature added to attributes and params by andrew
  // recently where the system attribute is grabbed from params instead of
  // registered manually when objercts are added to the warehouse.  It is okay
  // for FVTimeKernels to have a different system because they don't need to
  // be retrieved with all FVKernels - they only need to ever be retrieved
  // with volumetric fv kernels (i.e. source and time kernels).
  // FV source kernels will also need to register their base as
  // "FVElementalKernel".

  return params;
}

FVElementalKernel::FVElementalKernel(const InputParameters & parameters)
  : FVKernel(parameters),
    MooseVariableInterface(this, false, "variable", Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    _var(*mooseVariableFV())
{
}

registerADMooseObject("MooseApp", FVTimeKernel);

template <ComputeStage compute_stage>
InputParameters
FVTimeKernel<compute_stage>::validParams()
{
  InputParameters params = FVElementalKernel::validParams();

  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  params.registerSystemAttributeName("FVElementalKernel");
  return params;
}

template <ComputeStage compute_stage>
FVTimeKernel<compute_stage>::FVTimeKernel(const InputParameters & parameters)
  : FVElementalKernel(parameters), _u_dot(_var.adUDot<compute_stage>())
{
}

template <ComputeStage compute_stage>
void
FVTimeKernel<compute_stage>::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());
  _local_re(0) += MetaPhysicL::raw_value(computeQpResidual() * _assembly.elem()->volume());
  accumulateTaggedLocalResidual();
}

template <ComputeStage compute_stage>
ADReal
FVTimeKernel<compute_stage>::computeQpResidual()
{
  return _u_dot[_qp];
}

template <>
void
FVTimeKernel<RESIDUAL>::computeJacobian()
{
}

template <ComputeStage compute_stage>
void
FVTimeKernel<compute_stage>::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());
  auto dofs_per_elem = _subproblem.systemBaseNonlinear().getMaxVarNDofsPerElem();
  size_t ad_offset = _var.number() * dofs_per_elem;
  auto vol = _assembly.elem()->volume();
  const auto r = computeQpResidual() * vol;
  _local_ke(0, 0) += r.derivatives()[ad_offset];
  accumulateTaggedLocalMatrix();
}

adBaseClass(FVTimeKernel);
