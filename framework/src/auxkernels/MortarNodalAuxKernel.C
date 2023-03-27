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
#include "MooseUtils.h"
#include "libmesh/quadrature.h"

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

  // We should probably default use_displaced_mesh to true. If no displaced mesh exists
  // FEProblemBase::addKernel will automatically correct it to false. However,
  // this will still prompt a call from AugmentSparsityOnInterface to get a displaced
  // mortar interface since object._use_displaced_mesh = true.

  return params;
}

template <typename ComputeValueType>
MortarNodalAuxKernelTempl<ComputeValueType>::MortarNodalAuxKernelTempl(
    const InputParameters & parameters)
  : AuxKernelTempl<ComputeValueType>(setBoundaryParam(parameters)),
    MortarConsumerInterface(this),
    _displaced(this->template getParam<bool>("use_displaced_mesh")),
    _fe_problem(*this->template getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _msm_volume(0),
    _incremental(this->template getParam<bool>("incremental")),
    _u_old(uOld()),
    _test_lower(_var.phiLower()),
    _coord_msm(_assembly.mortarCoordTransformation())
{
  if (!isNodal())
    paramError("variable",
               "MortarNodalAuxKernel derived classes populate nodal aux variables only.");
}

template <typename ComputeValueType>
void
MortarNodalAuxKernelTempl<ComputeValueType>::initialSetup()
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
  if (!_var.isNodalDefined())
    return;

  ComputeValueType value(0);
  Real total_volume = 0;

  const auto & its = amg().secondariesToMortarSegments(*_current_node);

  auto act_functor = [&value, &total_volume, this]()
  {
    _msm_volume = 0;
    setNormals();
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
                                        act_functor,
                                        /*reinit_mortar_user_objects=*/false);

  // We have to reinit the node for this variable in order to get the dof index set for the node
  _var.reinitNode();

  // If the node doesn't have corresponding mortar segments, force the value assigned in this step
  // to be zero. This can be useful when nodes initially do not project but will project at a
  // different stage of the simulation

  if (MooseUtils::relativeFuzzyEqual(total_volume, 0.0))
    value = 0;
  else
    value /= total_volume;

  // Allow mortar auxiliary kernels to compute quantities incrementally
  if (!_incremental)
    _var.setNodalValue(value);
  else
  {
    mooseAssert(_u_old.size() == 1,
                "Expected 1 value in MortarNodalAuxKernel, but got " << _u_old.size());
    _var.setNodalValue(value + _u_old[0]);
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
