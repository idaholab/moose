//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestWeightedGapUserObject.h"
#include "MooseVariableField.h"
#include "SubProblem.h"
#include "MortarUtils.h"
#include "MooseUtils.h"
#include "libmesh/quadrature.h"
#include "timpi/parallel_sync.h"

using namespace libMesh;

registerMooseObject("MooseTestApp", TestWeightedGapUserObject);

InputParameters
TestWeightedGapUserObject::validParams()
{
  InputParameters params = MortarUserObject::validParams();
  params += MortarConsumerInterface::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params.addRequiredCoupledVar("weighted_gap_aux_var", "The weighted gap auxiliary variable.");
  params.set<bool>("use_displaced_mesh") = true;
  params.set<bool>("interpolate_normals") = false;
  params.set<bool>("force_preaux") = true;
  return params;
}

TestWeightedGapUserObject::TestWeightedGapUserObject(const InputParameters & parameters)
  : MortarUserObject(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _var(*getVar("weighted_gap_aux_var", 0)),
    _nodal(_var.isNodal()),
    _coord(_assembly.mortarCoordTransformation()),
    _test(_var.phiLower())
{
  if (!_var.isNodal())
    paramError("weighted_gap_aux_var",
               "The gap variable must have its degrees of freedom exclusively on "
               "nodes, e.g. it should probably be of finite element type 'Lagrange'.");
}

void
TestWeightedGapUserObject::computeQpProperties()
{
  // Compute gap vector
  const auto gap_vec = _phys_points_primary[_qp] - _phys_points_secondary[_qp];

  // Compute integration point quantities: Normals (geometry) is averaged at the node, but not
  // interpolated within the weak integration.
  _qp_gap_nodal = gap_vec * (_JxW_msm[_qp] * _coord[_qp]);
}

void
TestWeightedGapUserObject::computeQpIProperties()
{
  mooseAssert(_normals.size() == _lower_secondary_elem->n_nodes(),
              "Making sure that _normals is the expected size");

  // Get the _dof_to_weighted_gap map
  const auto * const dof = static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i));

  auto & [weighted_gap, volume] = _dof_to_weighted_gap[dof];
  weighted_gap += _test[_i][_qp] * _qp_gap_nodal * _normals[_i];
  volume += _test[_i][_qp] * _JxW_msm[_qp] * _coord[_qp];
}

void
TestWeightedGapUserObject::initialize()
{
  _dof_to_weighted_gap.clear();
}

namespace
{
void
communicateGaps(std::unordered_map<const DofObject *, std::pair<Real, Real>> & dof_to_weighted_gap,
                const MooseMesh & mesh,
                const Parallel::Communicator & communicator)
{
  libmesh_parallel_only(communicator);
  const auto our_proc_id = communicator.rank();

  // We may have weighted gap information that should go to other processes that own the dofs
  using Datum = std::tuple<dof_id_type, Real, Real>;
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data;

  for (auto & [dof_object, weighted_gap_volume_pr] : dof_to_weighted_gap)
  {
    const auto proc_id = dof_object->processor_id();
    if (proc_id == our_proc_id)
      continue;

    push_data[proc_id].push_back(std::make_tuple(
        dof_object->id(), weighted_gap_volume_pr.first, weighted_gap_volume_pr.second));
  }

  const auto & lm_mesh = mesh.getMesh();

  auto action_functor =
      [our_proc_id, &lm_mesh, &dof_to_weighted_gap](const processor_id_type libmesh_dbg_var(pid),
                                                    const std::vector<Datum> & sent_data)
  {
    mooseAssert(pid != our_proc_id, "We do not send messages to ourself here");
    libmesh_ignore(our_proc_id);

    for (auto & [dof_id, weighted_gap, volume] : sent_data)
    {
      const auto * const dof_object = static_cast<const DofObject *>(lm_mesh.node_ptr(dof_id));
      mooseAssert(dof_object, "This should be non-null");
      auto & [our_weighted_gap, our_volume] = dof_to_weighted_gap[dof_object];
      our_weighted_gap += weighted_gap;
      our_volume += volume;
    }
  };

  TIMPI::push_parallel_vector_data(communicator, push_data, action_functor);
}
}

void
TestWeightedGapUserObject::finalize()
{
  communicateGaps(_dof_to_weighted_gap, _subproblem.mesh(), _communicator);
}

void
TestWeightedGapUserObject::execute()
{
  mooseAssert(_test.size() <= _normals.size(), "These should be less than or equal to each other.");

  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test.size(); ++_i)
      computeQpIProperties();
  }
}
