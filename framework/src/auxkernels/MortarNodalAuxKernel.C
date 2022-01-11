//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarNodalAuxKernel.h"
#include "MooseVariableField.h"
#include "MortarUtils.h"

#include "libmesh/quadrature.h"

template <typename ComputeValueType>
InputParameters
MortarNodalAuxKernelTempl<ComputeValueType>::validParams()
{
  InputParameters params = AuxKernelTempl<ComputeValueType>::validParams();
  params += MortarInterface::validParams();
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

template <typename ComputeValueType>
MortarNodalAuxKernelTempl<ComputeValueType>::MortarNodalAuxKernelTempl(
    const InputParameters & parameters)
  : AuxKernelTempl<ComputeValueType>(parameters),
    MortarInterface(this),
    _displaced(this->template getParam<bool>("use_displaced_mesh")),
    _fe_problem(*this->template getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _msm_volume(0)
{
  if (!isNodal())
    paramError("variable", "MortarNodalAuxKernel derivatives populate nodal aux variables only.");
}

template <typename ComputeValueType>
void
MortarNodalAuxKernelTempl<ComputeValueType>::compute()
{
  ComputeValueType value(0);
  Real total_volume = 0;

  const auto & its = amg().secondariesToMortarSegments(*_current_node);
  std::array<MortarNodalAuxKernelTempl<ComputeValueType> *, 1> consumers = {{this}};

  auto act_functor = [&value, &total_volume, this]() {
    _msm_volume = 0;
    value += computeValue();
    total_volume += _msm_volume;
  };

  Moose::Mortar::loopOverMortarSegments(
      its, _assembly, _subproblem, _fe_problem, amg(), _displaced, consumers, act_functor);

  // We have to reinit the node for this variable in order to get the dof index set for the node
  _var.reinitNode();
  _var.setNodalValue(value / total_volume);
}

template <typename ComputeValueType>
void
MortarNodalAuxKernelTempl<ComputeValueType>::precalculateValue()
{
  mooseError(
      "not clear where this should be implemented in the compute loop. If you want to implement "
      "this function, please contact a MOOSE developer and tell them your use case");
}

// Explicitly instantiates the two versions of the MortarNodalAuxKernelTempl class
template class MortarNodalAuxKernelTempl<Real>;
template class MortarNodalAuxKernelTempl<RealVectorValue>;
