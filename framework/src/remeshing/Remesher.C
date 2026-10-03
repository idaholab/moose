//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Remesher.h"

#include "DisplacedProblem.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MooseVariableFieldBase.h"
#include "SystemBase.h"

#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_fe_family.h"
#include "libmesh/enum_order.h"
#include "libmesh/enum_to_string.h"
#include "libmesh/fe_map.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_base.h"
#include "libmesh/node.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/system.h"

#include <algorithm>
#include <limits>
#include <memory>

using namespace libMesh;

InputParameters
Remesher::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("Remesher");
  return params;
}

Remesher::Remesher(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _mesh(_fe_problem.mesh())
{
}

MeshBase *
Remesher::displacedMesh() const
{
  const auto displaced_problem = _fe_problem.getDisplacedProblem();
  return displaced_problem ? &displaced_problem->mesh().getMesh() : nullptr;
}

void
Remesher::checkElementalSizingVariable(const std::string & param,
                                       const MooseVariableFieldBase & variable) const
{
  const FEType & fe_type = variable.feType();
  if (fe_type.family != MONOMIAL || fe_type.order != CONSTANT)
    paramError(param,
               "The target element size is read as the single degree of freedom an element carries "
               "for it, which requires a CONSTANT MONOMIAL variable, but '",
               variable.name(),
               "' is ",
               Utility::enum_to_string(variable.order()),
               " ",
               Utility::enum_to_string(fe_type.family),
               ".");
}

std::optional<Real>
Remesher::readTargetSize(const MooseVariableFieldBase & variable,
                         const Elem & elem,
                         const std::optional<Real> size_floor) const
{
  mooseAssert(
      elem.processor_id() == _mesh.getMesh().processor_id(),
      "Element " << elem.id()
                 << " is not owned by this rank. current_local_solution is readable at the owned "
                    "degrees of freedom plus the send list, and the degrees of freedom partition "
                    "by processor id even where the elements do not, so the caller has to narrow "
                    "to the owned elements before reading a target off them.");

  const libMesh::System & system = variable.sys().system();
  const unsigned int sys_num = system.number();
  const unsigned int var_num = variable.number();

  // The variable is not defined on the subdomain of this element, which leaves it no target
  if (!elem.n_dofs(sys_num, var_num))
    return std::nullopt;

  Real target = (*system.current_local_solution)(elem.dof_number(sys_num, var_num, 0));
  if (size_floor)
    target = std::max(target, *size_floor);
  else if (target <= 0)
    mooseError("The sizing variable '",
               variable.name(),
               "' is ",
               target,
               " on element ",
               elem.id(),
               ", but a target element size has to be positive. Clamp the variable that carries it "
               "to a positive lower bound",
               // A remesher without the parameter cannot be told to set it
               parameters().have_parameter<Real>("min_element_size")
                   ? ", or set 'min_element_size' to hold the target at a floor."
                   : ".");

  return target;
}

std::map<dof_id_type, Real>
Remesher::gatherTargetSizes(std::vector<dof_id_type> target_ids, std::vector<Real> targets) const
{
  _communicator.allgather(target_ids);
  _communicator.allgather(targets);
  mooseAssert(target_ids.size() == targets.size(),
              "Every rank contributed one id per target, so the two gathers came back the same "
              "length.");

  std::map<dof_id_type, Real> table;
  for (const auto i : index_range(target_ids))
    table.emplace(target_ids[i], targets[i]);
  return table;
}

Remesher::RemeshSourcePoint
Remesher::locateSourcePoint(const std::vector<dof_id_type> & candidate_ids, const Point & p) const
{
  mooseAssert(!candidate_ids.empty(),
              "A new entity has at least one old element to be sourced from.");
  const MeshBase & mesh = _mesh.getMesh();

  const Elem * containing = nullptr;
  const Elem * closest = nullptr;
  Real closest_distance = std::numeric_limits<Real>::max();
  for (const auto elem_id : candidate_ids)
  {
    const Elem * elem = mesh.elem_ptr(elem_id);
    if (elem->contains_point(p))
    {
      containing = elem;
      break;
    }

    const Real distance = (elem->vertex_average() - p).norm();
    if (distance < closest_distance)
    {
      closest_distance = distance;
      closest = elem;
    }
  }

  RemeshSourcePoint source;
  // A point on the side of a candidate can miss every containment test by round-off, so the closest
  // candidate supplies it rather than leaving it without a source
  source.old_elem = containing ? containing : closest;
  source.xi =
      FEMap::inverse_map(source.old_elem->dim(), source.old_elem, p, TOLERANCE, /*secure=*/false);
  return source;
}

void
Remesher::reserveNewEntityIds(const dof_id_type n_new_nodes,
                              const dof_id_type n_new_elements,
                              dof_id_type & first_node_id,
                              dof_id_type & first_elem_id) const
{
  MeshBase & mesh = _mesh.getMesh();

  if (!_mesh.isDistributedMesh())
  {
    // Every rank holds the whole mesh and performs the same surgery in the same order, so these are
    // the same first free ids everywhere and the ranks stay in step without communicating
    first_node_id = mesh.max_node_id();
    first_elem_id = mesh.max_elem_id();
    return;
  }

  // max_node_id() and max_elem_id() are the global maxima only once the parallel counters have been
  // resynchronized, and every rank has to carve its block out of the same maximum
  mesh.update_parallel_id_counts();

  // Every rank participates, including one that creates nothing at all, because these are
  // collective
  std::vector<dof_id_type> new_node_counts;
  std::vector<dof_id_type> new_element_counts;
  _communicator.allgather(n_new_nodes, new_node_counts);
  _communicator.allgather(n_new_elements, new_element_counts);

  // The block of this rank sits after the blocks of the ranks below it, so the blocks are disjoint
  // and every rank could predict any other rank's block from the same two counts
  first_node_id = mesh.max_node_id();
  first_elem_id = mesh.max_elem_id();
  for (const auto pid : make_range(processor_id()))
  {
    first_node_id += new_node_counts[pid];
    first_elem_id += new_element_counts[pid];
  }
}

Node *
Remesher::addNode(const Point & point,
                  const RemeshSourcePoint & source,
                  dof_id_type & next_node_id,
                  RemeshRecord & record) const
{
  MeshBase & mesh = _mesh.getMesh();
  MeshBase * const displaced_mesh = displacedMesh();

  mooseAssert(source.old_elem, "A new node needs an old element to take its values from");
  const processor_id_type owner = source.old_elem->processor_id();
  const unsigned int n_systems = source.old_elem->node_ptr(0)->n_systems();

  Node * const new_node = mesh.add_point(point, next_node_id, owner);
  new_node->set_n_systems(n_systems);
  record.new_nodes.push_back(new_node);
  record.new_node_sources.push_back(source);

  if (displaced_mesh)
  {
    // Where the source element maps the same reference coordinate on the displaced mesh, which is
    // where the displacement variables will put the node once they are interpolated onto it
    const Elem * displaced_source = displaced_mesh->elem_ptr(source.old_elem->id());
    Node * const displaced_node = displaced_mesh->add_point(
        FEMap::map(displaced_source->dim(), displaced_source, source.xi), next_node_id, owner);
    displaced_node->set_n_systems(n_systems);
  }

  ++next_node_id;
  return new_node;
}

Elem *
Remesher::addTriangle(MeshBase & target,
                      const std::array<Node *, 3> & nodes,
                      const dof_id_type id,
                      const Elem & source,
                      const SideBoundaryIds & side_boundary_ids) const
{
  std::unique_ptr<Elem> triangle = Elem::build_with_id(TRI3, id);
  for (const auto i : index_range(nodes))
    triangle->set_node(i, nodes[i]);
  triangle->subdomain_id() = source.subdomain_id();
  // add_elem() reads the partition off the element, so it has to be set before the element is added
  triangle->processor_id() = source.processor_id();

  Elem * added = target.add_elem(std::move(triangle));
  added->set_n_systems(source.n_systems());

  for (const auto side : added->side_index_range())
  {
    const auto side_nodes = added->nodes_on_side(side);
    const auto it = side_boundary_ids.find(
        sortedNodePair(added->node_id(side_nodes[0]), added->node_id(side_nodes[1])));
    if (it != side_boundary_ids.end())
      target.get_boundary_info().add_side(added, side, it->second);
  }

  return added;
}

Elem *
Remesher::addMirroredTriangle(const std::array<Node *, 3> & nodes,
                              const RemeshSourcePoint & source,
                              const SideBoundaryIds & side_boundary_ids,
                              dof_id_type & next_elem_id,
                              RemeshRecord & record) const
{
  MeshBase & mesh = _mesh.getMesh();
  MeshBase * const displaced_mesh = displacedMesh();

  mooseAssert(source.old_elem, "A new triangle needs an old element to take its values from");
  Elem * const added = addTriangle(mesh, nodes, next_elem_id, *source.old_elem, side_boundary_ids);
  record.new_elements.push_back(added);
  record.new_element_sources.push_back(source);

  if (displaced_mesh)
  {
    // The displaced copies of the same nodes, which addNode gave the same ids
    const std::array<Node *, 3> displaced_nodes{displaced_mesh->node_ptr(nodes[0]->id()),
                                                displaced_mesh->node_ptr(nodes[1]->id()),
                                                displaced_mesh->node_ptr(nodes[2]->id())};
    addTriangle(*displaced_mesh,
                displaced_nodes,
                next_elem_id,
                *displaced_mesh->elem_ptr(source.old_elem->id()),
                side_boundary_ids);
  }

  ++next_elem_id;
  return added;
}
