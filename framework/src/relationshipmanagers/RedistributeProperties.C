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

std::unique_ptr<GhostingFunctor>
RedistributeProperties::clone() const
{
  return std::make_unique<RedistributeProperties>(*this);
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
RedistributeProperties::addMaterialPropertyStorage(
    std::vector<std::shared_ptr<MaterialData>> & mat_data, MaterialPropertyStorage & mat_props)
{
  _materials.emplace_back(&mat_data, &mat_props);
}

void
RedistributeProperties::redistribute()
{
  const MeshBase & mesh = _moose_mesh->getMesh();
  const processor_id_type pid = mesh.processor_id();
  for (auto & [mat_data, mat_prop_store] : _materials)
    if (mat_prop_store->hasStatefulProperties())
    {
      MaterialData & my_mat_data = *((*mat_data)[/*_tid*/ 0]); // Not threaded

      std::array<MaterialPropertyStorage::PropsType *, 3> props_maps{
          &mat_prop_store->props(), &mat_prop_store->propsOld(), &mat_prop_store->propsOlder()};

      for (auto * props_map_ptr : props_maps)
      {
        MaterialPropertyStorage::PropsType & props_map = *props_map_ptr;
        typedef std::unordered_map<unsigned int, std::string> stored_props_type;
        typedef std::tuple<stored_props_type, dof_id_type, int> stored_elem_type;
        std::map<processor_id_type, std::vector<stored_elem_type>> props_to_push;

        // Take non-const references here for compatibility with old
        // dataStore(T&)
        for (auto & [elem, elem_props_map] : props_map)
        {
          // There better be an element here
          libmesh_assert(elem);

          // There better be a *real* element here, not a dangling
          // pointer
          libmesh_assert(elem->id() < mesh.max_elem_id());
          libmesh_assert(elem->processor_id() < mesh.n_processors());

          if (elem->processor_id() != pid)
          {
            stored_props_type stored_props;
            unsigned int n_q_points = 0;
            for (auto & [prop_id, prop_vals] : elem_props_map)
            {
              for (PropertyValue * prop : prop_vals)
              {
                n_q_points = std::max(n_q_points, prop->size());
              }

              std::ostringstream oss;
              dataStore(oss, prop_vals, nullptr);
              stored_props[prop_id] = oss.str();
            }

            props_to_push[elem->processor_id()].emplace_back(
                std::move(stored_props), elem->id(), n_q_points);
          }
        }

        auto recv_functor = [&](processor_id_type, const std::vector<stored_elem_type> & data)
        {
          for (const auto & [elem_hash, elem_id, n_q_points] : data)
          {
            const Elem * elem = mesh.elem_ptr(elem_id);
            auto it = props_map.find(elem);
            if (it == props_map.end())
              for (const auto & [prop_id, prop_str] : elem_hash)
                mat_prop_store->initProps(my_mat_data, elem, prop_id, n_q_points);

            auto & elem_props = props_map[elem];

            for (const auto & [prop_id, prop_str] : elem_hash)
            {
              std::istringstream iss(prop_str);
              dataLoad(iss, elem_props[prop_id], nullptr);
            }
          }
        };

        Parallel::push_parallel_vector_data(mesh.comm(), props_to_push, recv_functor);
      }
    }
}
