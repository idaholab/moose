//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RemeshTransfer.h"

#include "AuxiliarySystem.h"
#include "FEProblemBase.h"
#include "MaterialPropertyStorage.h"
#include "MooseMesh.h"
#include "ProjectedStatefulMaterialStorageAction.h"
#include "SystemBase.h"

#include "libmesh/dof_map.h"
#include "libmesh/dof_object.h"
#include "libmesh/elem.h"
#include "libmesh/enum_fe_family.h"
#include "libmesh/fe_interface.h"
#include "libmesh/fe_type.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_base.h"
#include "libmesh/node.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"
#include "libmesh/simple_range.h"
#include "libmesh/system.h"
#include "libmesh/utility.h"

#include <algorithm>
#include <set>

using namespace libMesh;

RemeshTransfer::RemeshTransfer(FEProblemBase & problem)
  : ParallelObject(problem.comm()), _fe_problem(problem), _mesh(problem.mesh())
{
}

void
RemeshTransfer::checkStatefulCoverage() const
{
  // The engine calls FEProblemBase::eraseElementStatefulProps() on every replaced element, which
  // erases from all three storages, so a stateful property in any of them is lost at a remesh
  const std::vector<const MaterialPropertyStorage *> storages = {
      &_fe_problem.getMaterialPropertyStorage(),
      &_fe_problem.getBndMaterialPropertyStorage(),
      &_fe_problem.getNeighborMaterialPropertyStorage()};

  // The three storages share one property registry, so a property that lives in more than one of
  // them is only reported once
  std::set<unsigned int> reported;

  for (const auto storage : storages)
    for (const auto prop_id : storage->statefulProps())
    {
      if (!reported.insert(prop_id).second)
        continue;

      const auto prop_name = storage->queryStatefulPropName(prop_id);
      if (!prop_name)
        continue;

      // declarers is sorted, so the message reads the same on every rank and from run to run
      const auto & declarers = storage->getPropRecord(prop_id).declarers;
      const std::string declarer = declarers.empty() ? "(unknown)" : *declarers.begin();

      // use_interpolated_state would have rewritten the request that made this property stateful,
      // so reaching here means the raw old state is read; ProjectedStatefulMaterialStorageAction
      // leaves nothing queryable behind, so the auxiliary variable it would have added only says
      // which of the two remedies applies
      if (_fe_problem.hasVariable(
              ProjectedStatefulMaterialStorageAction::projectedVariableName(*prop_name, 0)))
        mooseError("The stateful material property '",
                   *prop_name,
                   "' declared by material '",
                   declarer,
                   "' is projected for remeshing, but its old state is still read from the raw "
                   "material property storage, which remeshing discards along with the elements it "
                   "replaces. Set use_interpolated_state = true on every object that reads the old "
                   "or older state of '",
                   *prop_name,
                   "', so that it reads the projected auxiliary variables instead.");
      else
        mooseError("The stateful material property '",
                   *prop_name,
                   "' declared by material '",
                   declarer,
                   "' is not projected for remeshing. Remeshing carries stateful material "
                   "properties across a mesh replacement through projected auxiliary variables; "
                   "add the property to the [ProjectedStatefulMaterialStorage] block.");
    }
}

std::vector<SystemBase *>
RemeshTransfer::systems() const
{
  std::vector<SystemBase *> systems;
  for (const auto i : make_range(_fe_problem.numSolverSystems()))
    systems.push_back(&_fe_problem.getSystemBase(static_cast<unsigned int>(i)));
  systems.push_back(&_fe_problem.getAuxiliarySystem());

  return systems;
}

std::vector<NumericVector<Number> *>
RemeshTransfer::systemVectors(System & system)
{
  std::vector<NumericVector<Number> *> vectors{system.solution.get()};
  for (const auto & [_, vector] : as_range(system.vectors_begin(), system.vectors_end()))
    vectors.push_back(vector.get());

  return vectors;
}

std::unique_ptr<NumericVector<Number>>
RemeshTransfer::localizedCopy(const NumericVector<Number> & vector, const DofMap & dof_map)
{
  auto copy = NumericVector<Number>::build(vector.comm());
  copy->init(dof_map.n_dofs(), dof_map.n_local_dofs(), dof_map.get_send_list(), false, GHOSTED);
  vector.localize(*copy, dof_map.get_send_list());

  return copy;
}

std::size_t
RemeshTransfer::sourceValuesPerPoint() const
{
  std::size_t n_values = 0;
  for (const auto system : systems())
    n_values += system->system().n_vars() * systemVectors(system->system()).size();

  return n_values;
}

void
RemeshTransfer::gather(const Remesher::RemeshRecord & record)
{
  mooseAssert(record.new_nodes.size() == record.new_node_sources.size(),
              "The source points of the record are not parallel to its new nodes");
  mooseAssert(record.new_elements.size() == record.new_element_sources.size(),
              "The source points of the record are not parallel to its new elements");

  const auto systems = this->systems();

  ReadVectors reads(systems.size());
  for (const auto i : index_range(systems))
  {
    auto & system = systems[i]->system();
    for (const auto vector : systemVectors(system))
      reads[i].push_back(localizedCopy(*vector, system.get_dof_map()));
  }

  snapshotSurvivingEntities(record, systems, reads);
  evaluateSourcePoints(record, systems, reads);
}

void
RemeshTransfer::snapshotSurvivingEntities(const Remesher::RemeshRecord & record,
                                          const std::vector<SystemBase *> & systems,
                                          const ReadVectors & reads)
{
  // The entities the surgery touched are left out: the replaced ones are about to be deleted, and
  // the new ones carry no degrees of freedom until meshChanged() distributes them
  std::set<const DofObject *> touched;
  for (const auto & node : record.new_nodes)
    touched.insert(node);
  for (const auto & node : record.replaced_nodes)
    touched.insert(node);
  for (const auto & elem : record.new_elements)
    touched.insert(elem);
  for (const auto & elem : record.replaced_elements)
    touched.insert(elem);

  auto & mesh = _mesh.getMesh();

  _snapshots.assign(systems.size(), SystemSnapshot());

  for (const auto s : index_range(systems))
  {
    auto & system = systems[s]->system();
    const auto & dof_map = system.get_dof_map();
    const auto sys_num = system.number();
    auto & snapshot = _snapshots[s];

    // Only the degrees of freedom this rank owns are snapshotted. scatter() writes those, and the
    // close() that follows carries them to the ranks that ghost them
    auto add_entity = [&snapshot, sys_num, &system](const DofObject & dof_object)
    {
      for (const auto var : make_range(system.n_vars()))
        for (const auto comp : make_range(dof_object.n_comp(sys_num, var)))
          snapshot.entity_dofs.push_back({&dof_object, var, comp});
    };

    for (const auto & node : mesh.local_node_ptr_range())
      if (!touched.count(node))
        add_entity(*node);
    for (const auto & elem : mesh.active_local_element_ptr_range())
      if (!touched.count(elem))
        add_entity(*elem);

    // A scalar variable hangs off no mesh entity. Its degrees of freedom sit at the end of the
    // numbering, where only the position within the block of the variable survives the
    // redistribution
    std::vector<dof_id_type> scalar_dofs;
    std::vector<dof_id_type> scalar_read_dofs;
    for (const auto var : make_range(system.n_vars()))
      if (system.variable_type(var).family == SCALAR)
      {
        dof_map.SCALAR_dof_indices(scalar_dofs, var);
        for (const auto i : index_range(scalar_dofs))
          if (dof_map.local_index(scalar_dofs[i]))
          {
            snapshot.scalar_dofs.emplace_back(var, i);
            scalar_read_dofs.push_back(scalar_dofs[i]);
          }
      }

    snapshot.entity_values.resize(reads[s].size());
    snapshot.scalar_values.resize(reads[s].size());
    for (const auto k : index_range(reads[s]))
    {
      const auto & vector = *reads[s][k];

      auto & entity_values = snapshot.entity_values[k];
      entity_values.reserve(snapshot.entity_dofs.size());
      for (const auto & [dof_object, var, comp] : snapshot.entity_dofs)
        entity_values.push_back(vector(dof_object->dof_number(sys_num, var, comp)));

      auto & scalar_values = snapshot.scalar_values[k];
      scalar_values.reserve(scalar_read_dofs.size());
      for (const auto dof : scalar_read_dofs)
        scalar_values.push_back(vector(dof));
    }
  }
}

void
RemeshTransfer::evaluateSourcePoints(const Remesher::RemeshRecord & record,
                                     const std::vector<SystemBase *> & systems,
                                     const ReadVectors & reads)
{
  _new_node_values.assign(record.new_nodes.size(), std::vector<Real>());
  _new_element_values.assign(record.new_elements.size(), std::vector<Real>());

  std::map<processor_id_type, std::vector<SourceQuery>> queries;
  // Where the answer to each query belongs, in the order the queries were built. The first member
  // says whether the answer feeds a new node, the second is its index in the record
  std::map<processor_id_type, std::vector<std::pair<bool, std::size_t>>> answers;

  auto queue = [&queries, &answers](
                   const Remesher::RemeshSourcePoint & source, const bool is_node, const auto i)
  {
    if (!source.old_elem)
      mooseError("The remesher returned a new mesh entity that has no source point on the old "
                 "mesh, so there is nothing to give it a value from.");

    const auto pid = source.old_elem->processor_id();
    queries[pid].emplace_back(source.xi, source.old_elem->id());
    answers[pid].emplace_back(is_node, i);
  };

  for (const auto i : index_range(record.new_node_sources))
    queue(record.new_node_sources[i], true, i);
  for (const auto i : index_range(record.new_element_sources))
    queue(record.new_element_sources[i], false, i);

  auto gather_functor = [this, &systems, &reads](processor_id_type,
                                                 const std::vector<SourceQuery> & incoming,
                                                 std::vector<std::vector<Real>> & outgoing)
  {
    outgoing.resize(incoming.size());
    for (const auto i : index_range(incoming))
    {
      const auto & [xi, elem_id] = incoming[i];
      const Elem * const old_elem = _mesh.getMesh().query_elem_ptr(elem_id);
      if (!old_elem)
        mooseError("The old element ",
                   elem_id,
                   ", which a remesh source point was located in, is not in the mesh of the rank "
                   "that owns it.");

      interpolate(*old_elem, xi, systems, reads, outgoing[i]);
    }
  };

  auto action_functor = [this, &answers](const processor_id_type pid,
                                         const std::vector<SourceQuery> &,
                                         const std::vector<std::vector<Real>> & incoming)
  {
    const auto & targets = libmesh_map_find(answers, pid);
    mooseAssert(targets.size() == incoming.size(), "One answer per query was expected");

    for (const auto i : index_range(incoming))
    {
      const auto & [is_node, index] = targets[i];
      if (is_node)
        _new_node_values[index] = incoming[i];
      else
        _new_element_values[index] = incoming[i];
    }
  };

  // Collective: every rank takes part, including the ranks whose surgery produced no new entity
  // and which therefore send no query
  const std::vector<Real> * example = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(
      comm(), queries, gather_functor, action_functor, example);

  mooseAssert(std::all_of(_new_node_values.begin(),
                          _new_node_values.end(),
                          [n = sourceValuesPerPoint()](const std::vector<Real> & values)
                          { return values.size() == n; }),
              "A new node was left without one value per system, variable and vector");
  mooseAssert(std::all_of(_new_element_values.begin(),
                          _new_element_values.end(),
                          [n = sourceValuesPerPoint()](const std::vector<Real> & values)
                          { return values.size() == n; }),
              "A new element was left without one value per system, variable and vector");
}

void
RemeshTransfer::interpolate(const Elem & old_elem,
                            const Point & xi,
                            const std::vector<SystemBase *> & systems,
                            const ReadVectors & reads,
                            std::vector<Real> & values) const
{
  values.clear();

  std::vector<dof_id_type> dof_indices;
  std::vector<Real> phi;

  for (const auto s : index_range(systems))
  {
    auto & system = systems[s]->system();
    const auto & dof_map = system.get_dof_map();

    for (const auto var : make_range(system.n_vars()))
    {
      const auto & fe_type = system.variable_type(var);

      dof_map.dof_indices(&old_elem, dof_indices, var);
      phi.clear();

      // A scalar variable has no shape functions, and a vector valued family has no scalar ones.
      // Neither lands on a mesh entity of the new mesh through a source point; a scalar variable
      // is carried by the snapshot of the surviving degrees of freedom instead
      const bool interpolable =
          fe_type.family != SCALAR && FEInterface::field_type(fe_type) == TYPE_SCALAR;

      // An empty index list means the variable is not defined on the subdomain of the old element,
      // in which case the new entity has no degree of freedom to fill either
      if (interpolable && !dof_indices.empty())
      {
        if (dof_indices.size() != FEInterface::n_dofs(fe_type, &old_elem))
          mooseError("Remeshing cannot interpolate the variable '",
                     system.variable_name(var),
                     "', which carries more than one value per shape function of an element.");

        for (const auto j : index_range(dof_indices))
          phi.push_back(FEInterface::shape(fe_type, &old_elem, j, xi));
      }

      for (const auto k : index_range(reads[s]))
      {
        const auto & vector = *reads[s][k];

        Real value = 0;
        for (const auto j : index_range(phi))
          value += phi[j] * vector(dof_indices[j]);

        values.push_back(value);
      }
    }
  }
}

void
RemeshTransfer::scatter(const Remesher::RemeshRecord & record)
{
  const auto systems = this->systems();
  mooseAssert(systems.size() == _snapshots.size(),
              "gather() ran on a different set of systems than scatter()");

  restoreSurvivingEntities(systems);
  fillNewEntities(record, systems);

  for (const auto system : systems)
  {
    for (const auto vector : systemVectors(system->system()))
      // close() also refreshes the ghosted entries of a ghosted vector, which is how the ranks
      // that do not own a degree of freedom see the value written here
      vector->close();

    // Refresh the localized copy of the solution that the rest of MOOSE reads through
    system->system().update();
  }

  // The gathered state is a full copy of the local solution, which there is no reason to hold on
  // to between two remesh events
  _snapshots = {};
  _new_node_values = {};
  _new_element_values = {};
}

void
RemeshTransfer::restoreSurvivingEntities(const std::vector<SystemBase *> & systems)
{
  for (const auto s : index_range(systems))
  {
    auto & system = systems[s]->system();
    const auto & dof_map = system.get_dof_map();
    const auto sys_num = system.number();
    const auto & snapshot = _snapshots[s];
    const auto vectors = systemVectors(system);
    mooseAssert(vectors.size() == snapshot.entity_values.size(),
                "The system gained or lost a vector across the mesh change");

    std::vector<dof_id_type> scalar_dofs;

    for (const auto k : index_range(vectors))
    {
      auto & vector = *vectors[k];

      for (const auto i : index_range(snapshot.entity_dofs))
      {
        const auto & [dof_object, var, comp] = snapshot.entity_dofs[i];

        // A surviving entity loses a variable when the surgery changes which subdomains touch it,
        // and then the degree of freedom that held the value is gone
        if (comp < dof_object->n_comp(sys_num, var))
          vector.set(dof_object->dof_number(sys_num, var, comp), snapshot.entity_values[k][i]);
      }

      for (const auto i : index_range(snapshot.scalar_dofs))
      {
        const auto & [var, index] = snapshot.scalar_dofs[i];
        dof_map.SCALAR_dof_indices(scalar_dofs, var);
        vector.set(scalar_dofs[index], snapshot.scalar_values[k][i]);
      }
    }
  }
}

void
RemeshTransfer::fillNewEntities(const Remesher::RemeshRecord & record,
                                const std::vector<SystemBase *> & systems)
{
  // The values of a source point are laid out by system, then variable, then vector, which is the
  // order interpolate() produced them in. Both orders are global metadata, so the rank that
  // evaluated a point and the rank that consumes it agree on the layout without communicating
  std::size_t slot = 0;

  for (const auto system_base : systems)
  {
    auto & system = system_base->system();
    const auto vectors = systemVectors(system);

    for (const auto var : make_range(system.n_vars()))
      for (const auto k : index_range(vectors))
      {
        auto & vector = *vectors[k];

        for (const auto i : index_range(record.new_nodes))
          setNewEntityDof(*record.new_nodes[i], system, var, _new_node_values[i][slot], vector);

        for (const auto i : index_range(record.new_elements))
          setNewEntityDof(
              *record.new_elements[i], system, var, _new_element_values[i][slot], vector);

        ++slot;
      }
  }

  mooseAssert(slot == sourceValuesPerPoint(), "The source point layout changed across the surgery");
}

void
RemeshTransfer::setNewEntityDof(const DofObject & dof_object,
                                const System & system,
                                const unsigned int var,
                                const Real value,
                                NumericVector<Number> & vector) const
{
  const auto n_comp = dof_object.n_comp(system.number(), var);

  // The variable is not defined on the subdomain of the entity, so there is nothing to fill
  if (n_comp == 0)
    return;

  if (n_comp > 1)
    mooseError("Remeshing cannot carry the variable '",
               system.variable_name(var),
               "' across a mesh replacement, because it holds ",
               n_comp,
               " degrees of freedom on a single mesh entity. The remesh record locates one value "
               "per new node and one per new element, which covers the variables that hold a "
               "single degree of freedom per entity, such as first order LAGRANGE and CONSTANT "
               "MONOMIAL.");

  vector.set(dof_object.dof_number(system.number(), var, 0), value);
}
