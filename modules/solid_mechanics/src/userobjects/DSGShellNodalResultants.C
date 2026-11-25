//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DSGShellNodalResultants.h"
#include "ComputeDSGShellStrain.h"

registerMooseObject("SolidMechanicsApp", DSGShellNodalResultants);

InputParameters
DSGShellNodalResultants::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(
      "Recovers the stress resultants of the DSG shell elements at the nodes: every element "
      "evaluates its resultants at its own nodes, and this object averages them over the "
      "elements sharing each node. Needs compute_nodal_strains = true on the strain material; "
      "read by DSGShellNodalResultantsAux.");
  return params;
}

DSGShellNodalResultants::DSGShellNodalResultants(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _nodal_resultants(getMaterialProperty<DenseMatrix<Real>>("dsg_shell_nodal_resultants"))
{
}

void
DSGShellNodalResultants::initialize()
{
  _sums.clear();
  _counts.clear();
}

void
DSGShellNodalResultants::execute()
{
  // The material fills the matrix only when the strain material was asked for the nodal
  // strains; an empty matrix is a set-up error worth naming.
  const DenseMatrix<Real> & resultants = _nodal_resultants[0];
  if (resultants.m() != _current_elem->n_nodes())
    mooseError("Element ",
               _current_elem->id(),
               " carries no nodal resultants. Set 'compute_nodal_strains = true' on the "
               "ComputeDSGShellStrain material.");

  for (unsigned int node = 0; node < _current_elem->n_nodes(); ++node)
  {
    const dof_id_type node_id = _current_elem->node_id(node);
    std::vector<Real> & sum = _sums[node_id];
    if (sum.empty())
      sum.assign(DSGShell::n_strains, 0.0);
    for (unsigned int component = 0; component < DSGShell::n_strains; ++component)
      sum[component] += resultants(node, component);
    _counts[node_id] += 1;
  }
}

void
DSGShellNodalResultants::threadJoin(const UserObject & other)
{
  const auto & other_object = static_cast<const DSGShellNodalResultants &>(other);
  for (const auto & [node_id, other_sum] : other_object._sums)
  {
    std::vector<Real> & sum = _sums[node_id];
    if (sum.empty())
      sum.assign(DSGShell::n_strains, 0.0);
    for (unsigned int component = 0; component < DSGShell::n_strains; ++component)
      sum[component] += other_sum[component];
  }
  for (const auto & [node_id, other_count] : other_object._counts)
    _counts[node_id] += other_count;
}

void
DSGShellNodalResultants::finalize()
{
  // A node on a processor boundary receives contributions from the elements of more than one
  // processor. Each processor sends the sums of the nodes it does not own; the owner, whose
  // local nodes are the ones the auxiliary kernel asks for, adds them to its own. Only the
  // nodes on processor boundaries travel.
  if (_communicator.size() == 1)
    return;

  const processor_id_type my_rank = _communicator.rank();
  std::vector<dof_id_type> node_ids;
  std::vector<Real> sums;
  std::vector<unsigned int> counts;
  for (const auto & [node_id, sum] : _sums)
    if (_mesh.nodeRef(node_id).processor_id() != my_rank)
    {
      node_ids.push_back(node_id);
      sums.insert(sums.end(), sum.begin(), sum.end());
      counts.push_back(_counts[node_id]);
    }
  _communicator.allgather(node_ids);
  _communicator.allgather(sums);
  _communicator.allgather(counts);

  for (const auto i : index_range(node_ids))
  {
    const dof_id_type node_id = node_ids[i];
    if (_mesh.nodeRef(node_id).processor_id() != my_rank)
      continue;
    std::vector<Real> & sum = _sums[node_id];
    if (sum.empty())
      sum.assign(DSGShell::n_strains, 0.0);
    for (unsigned int component = 0; component < DSGShell::n_strains; ++component)
      sum[component] += sums[DSGShell::n_strains * i + component];
    _counts[node_id] += counts[i];
  }
}

Real
DSGShellNodalResultants::nodalValue(const dof_id_type node_id,
                                    const unsigned int resultant_index) const
{
  const auto sum = _sums.find(node_id);
  if (sum == _sums.end())
    mooseError("Node ", node_id, " belongs to no DSG shell element.");
  return sum->second[resultant_index] / _counts.at(node_id);
}
