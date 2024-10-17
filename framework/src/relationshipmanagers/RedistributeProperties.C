//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RedistributeProperties.h"

#include "MaterialProperty.h"
#include "MaterialPropertyStorage.h"

#include <timpi/parallel_sync.h>

registerMooseObject("MooseApp", RedistributeProperties);

InputParameters
RedistributeProperties::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  return params;
}

RedistributeProperties::RedistributeProperties(const InputParameters & parameters)
  : RelationshipManager(parameters)
{
}

void
RedistributeProperties::operator()(const MeshBase::const_element_iterator &,
                                   const MeshBase::const_element_iterator &,
                                   processor_id_type,
                                   map_type &)
{
}

std::unique_ptr<libMesh::GhostingFunctor>
RedistributeProperties::clone() const
{
  return _app.getFactory().copyConstruct(*this);
}

std::string
RedistributeProperties::getInfo() const
{
  return "RedistributeProperties";
}

// the LHS ("this" object) in MooseApp::addRelationshipManager is the existing RelationshipManager
// object to which we are comparing the rhs to determine whether it should get added
bool
RedistributeProperties::operator>=(const RelationshipManager & rhs) const
{
  const auto * rm = dynamic_cast<const RedistributeProperties *>(&rhs);

  // All RedistributeProperties objects are effectively equivalent
  return rm;
}

void
RedistributeProperties::addMaterialPropertyStorage(MaterialPropertyStorage & mat_props)
{
  _materials.emplace_back(&mat_props);
}

void
RedistributeProperties::redistribute()
{
  const MeshBase & mesh = _moose_mesh->getMesh();
  const processor_id_type pid = mesh.processor_id();

  for (auto & mat_prop_store : _materials)
  {
    // Once we've redistributed data elsewhere we'll delete it here.
    //
    // We don't need stored properties on non-local elements, and so
    // we don't compute them and we don't *bother deleting them when
    // non-local elements are coarsened away*, so this may be our last
    // chance to avoid dangling pointers in future redistribute()
    // calls.
    std::set<const Elem *> elems_being_migrated;

    if (mat_prop_store->hasStatefulProperties())
    {
      mooseAssert(
          !Threads::in_threads,
          "This routine has not been implemented for threads. Please query this routine before "
          "a threaded region or contact a MOOSE developer to discuss.");

      for (const auto state : mat_prop_store->stateIndexRange())
      {
        MaterialPropertyStorage::PropsType & props_map = mat_prop_store->setProps(state);
        typedef std::unordered_map<unsigned int, std::string> stored_props_type;
        typedef std::tuple<stored_props_type, dof_id_type, int> stored_elem_type;
        std::map<processor_id_type, std::vector<stored_elem_type>> props_to_push;

        // Take non-const references here for compatibility with dataStore(T&)
        for (auto & [elem, elem_props_map] : props_map)
        {
          // There better be an element here
          mooseAssert(elem, "Null element found in material property map?");

          // It had better be a *real* element here, not a dangling
          // pointer
          mooseAssert(elem->id() < mesh.max_elem_id(),
                      "Invalid (dangling? corrupted?) element in material property map?");
          mooseAssert(elem->processor_id() < mesh.n_processors(),
                      "Invalid (corrupted?) element in material property map?");

          // If it's not in the particular mesh we're responsible for,
          // we can skip it.  MOOSE mixes non-displaced with displaced
          // mesh properties in their storages.
          if (elem != mesh.query_elem_ptr(elem->id()))
            continue;

          std::set<processor_id_type> target_pids;

          if (elem->active())
            target_pids.insert(elem->processor_id());
          else if (elem->subactive())
          {
            mooseAssert(elem->parent(),
                        "Invalid (corrupted?) subactive element in material property map");
            mooseAssert(elem->parent()->refinement_flag() == Elem::JUST_COARSENED,
                        "Invalid (subactive child of not-just-coarsened) element in material "
                        "property map");
            target_pids.insert(elem->parent()->processor_id());
          }
          else
          {
            mooseAssert(elem->ancestor(),
                        "Unexpected relationship for element in material property map?");
            mooseAssert(elem->has_children(),
                        "Ancestor element with no children in material property map?");
            for (const Elem & child : elem->child_ref_range())
              target_pids.insert(child.processor_id());
          }

          // If we're being migrated elsewhere and we're not needed
          // for AMR/C locally then we should erase the local entry
          bool remote_pid = false, local_pid = false;
          for (processor_id_type target_pid : target_pids)
            if (target_pid == pid)
              local_pid = true;
            else
            {
              remote_pid = true;

              stored_props_type stored_props;
              unsigned int n_q_points = 0;
              for (auto & [prop_id, prop_vals] : elem_props_map)
              {
                mooseAssert(!prop_vals.empty(),
                            "Empty MaterialProperties in stateful properties map?");

                for (const auto & prop : prop_vals)
                  n_q_points = std::max(n_q_points, prop.size());

                std::ostringstream oss;
                dataStore(oss, prop_vals, nullptr);
                stored_props[prop_id] = oss.str();
              }

              // Get the stored data ready to push to the element's new
              // processor
              props_to_push[target_pid].emplace_back(
                  std::move(stored_props), elem->id(), n_q_points);
            }

          if (remote_pid && !local_pid)
            elems_being_migrated.insert(elem);
        }

        auto recv_functor = [&, mat_prop_store_ptr = mat_prop_store](
                                processor_id_type, const std::vector<stored_elem_type> & data)
        {
          for (const auto & [elem_hash, elem_id, n_q_points] : data)
          {
            const Elem * elem = mesh.elem_ptr(elem_id);
            auto & elem_props = props_map[elem];

            for (const auto & [prop_id, prop_str] : elem_hash)
            {
              // This should be called "initPropsIfNecessary"... which
              // is confusing but convenient.
              //
              // We need to *only* initProps for the map we're working
              // on, otherwise we might see an
              // initialized-but-not-filled entry in the next map and
              // foolishly try to send it places.
              mat_prop_store_ptr->initProps(0, state, elem, prop_id, n_q_points);

              mooseAssert(elem_props.contains(prop_id),
                          "Trying to load into a nonexistant property id?");
              MaterialProperties & mat_props = elem_props[prop_id];
              std::istringstream iss(prop_str);
              dataLoad(iss, mat_props, nullptr);
            }
          }
        };

        Parallel::push_parallel_vector_data(mesh.comm(), props_to_push, recv_functor);
      }
    }

    for (const Elem * elem : elems_being_migrated)
      mat_prop_store->eraseProperty(elem);
  }
}
