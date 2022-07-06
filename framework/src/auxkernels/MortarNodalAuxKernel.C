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

#include <limits>

namespace
{
const InputParameters &
setBoundaryParam(const InputParameters & params_in)
{
  InputParameters & ret = const_cast<InputParameters &>(params_in);
  ret.set<std::vector<BoundaryName>>("boundary") = {
      params_in.get<BoundaryName>("secondary_boundary")};
  return ret;
}
}

template <typename ComputeValueType>
InputParameters
MortarNodalAuxKernelTempl<ComputeValueType>::validParams()
{
  InputParameters params = AuxKernelTempl<ComputeValueType>::validParams();
  params += MortarConsumerInterface::validParams();
  params.set<bool>("ghost_point_neighbors") = true;
  params.suppressParameter<std::vector<BoundaryName>>("boundary");
  params.suppressParameter<std::vector<SubdomainName>>("block");
  params.addParam<bool>(
      "incremental", false, "Whether to accumulate mortar auxiliary kernel value");
  return params;
}

template <typename ComputeValueType>
MortarNodalAuxKernelTempl<ComputeValueType>::MortarNodalAuxKernelTempl(
    const InputParameters & parameters)
  : AuxKernelTempl<ComputeValueType>(setBoundaryParam(parameters)),
    MortarExecutorInterface(
        *this->template getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    MortarConsumerInterface(this),
    _displaced(parameters.isParamSetByUser("use_displaced_mesh")
                   ? this->template getParam<bool>("use_displaced_mesh")
                   : true),
    _fe_problem(*this->template getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _msm_volume(0),
    _incremental(this->template getParam<bool>("incremental")),
    _u_old(uOld())
{
  if (!isNodal())
    paramError("variable", "MortarNodalAuxKernel derivatives populate nodal aux variables only.");
}

template <typename ComputeValueType>
void
MortarNodalAuxKernelTempl<ComputeValueType>::mortarSetup()
{
  std::array<const MortarNodalAuxKernelTempl<ComputeValueType> *, 1> consumers = {{this}};

  Moose::Mortar::setupMortarMaterials(consumers,
                                      _fe_problem,
                                      amg(),
                                      _tid,
                                      _secondary_ip_sub_to_mats,
                                      _primary_ip_sub_to_mats,
                                      _secondary_boundary_mats);
}

template <typename ComputeValueType>
void
MortarNodalAuxKernelTempl<ComputeValueType>::compute()
{
  ComputeValueType value(0);
  Real total_volume = 0;

  const auto & its = amg().secondariesToMortarSegments(*_current_node);

  auto act_functor = [&value, &total_volume, this]()
  {
    _msm_volume = 0;
    value += computeValue();
    total_volume += _msm_volume;
  };

  std::array<MortarNodalAuxKernelTempl<ComputeValueType> *, 1> consumers = {{this}};

  Moose::Mortar::loopOverMortarSegments(its,
                                        _assembly,
                                        _subproblem,
                                        _fe_problem,
                                        amg(),
                                        _displaced,
                                        consumers,
                                        _tid,
                                        _secondary_ip_sub_to_mats,
                                        _primary_ip_sub_to_mats,
                                        _secondary_boundary_mats,
                                        act_functor);

  // We have to reinit the node for this variable in order to get the dof index set for the node
  _var.reinitNode();

  // If the node doesn't have corresponding mortar segments, force the value assigned in this step
  // to be zero. This can be useful when nodes initially do not project but will project at a
  // different stage of the simulation
  if (total_volume == 0)
    total_volume = std::numeric_limits<Real>::max();

  // Allow mortar auxiliary kernels to compute quantities incrementally
  if (!_incremental)
    _var.setNodalValue(value / total_volume);
  else
  {
    mooseAssert(_u_old.size() == 1,
                "Expected 1 value in MortarNodalAuxKernel, but got " << _u_old.size());
    _var.setNodalValue(value / total_volume + _u_old[0]);
  }
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
