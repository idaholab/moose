//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// App includes
#include "AugmentSparsityOnInterface.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseApp.h"

// libMesh includes
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/boundary_info.h"

registerMooseObject("MooseApp", AugmentSparsityOnInterface);

using namespace libMesh;

InputParameters
AugmentSparsityOnInterface::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  params.addRequiredParam<BoundaryName>("primary_boundary",
                                        "The name of the primary boundary sideset.");
  params.addRequiredParam<BoundaryName>("secondary_boundary",
                                        "The name of the secondary boundary sideset.");
  params.addRequiredParam<SubdomainName>("primary_subdomain",
                                         "The name of the primary lower dimensional subdomain.");
  params.addRequiredParam<SubdomainName>("secondary_subdomain",
                                         "The name of the secondary lower dimensional subdomain.");
  params.addParam<bool>("ghost_point_neighbors",
                        false,
                        "Whether we should ghost point neighbors of secondary face elements, and "
                        "consequently also their mortar interface couples.");
  return params;
}

AugmentSparsityOnInterface::AugmentSparsityOnInterface(const InputParameters & params)
  : RelationshipManager(params),
    _primary_boundary_name(getParam<BoundaryName>("primary_boundary")),
    _secondary_boundary_name(getParam<BoundaryName>("secondary_boundary")),
    _primary_subdomain_name(getParam<SubdomainName>("primary_subdomain")),
    _secondary_subdomain_name(getParam<SubdomainName>("secondary_subdomain")),
    _is_coupling_functor(isType(Moose::RelationshipManagerType::COUPLING)),
    _ghost_point_neighbors(getParam<bool>("ghost_point_neighbors"))
{
}

AugmentSparsityOnInterface::AugmentSparsityOnInterface(const AugmentSparsityOnInterface & other)
  : RelationshipManager(other),
    _primary_boundary_name(other._primary_boundary_name),
    _secondary_boundary_name(other._secondary_boundary_name),
    _primary_subdomain_name(other._primary_subdomain_name),
    _secondary_subdomain_name(other._secondary_subdomain_name),
    _is_coupling_functor(other._is_coupling_functor),
    _ghost_point_neighbors(other._ghost_point_neighbors)
{
}

void
AugmentSparsityOnInterface::mesh_reinit()
{
  // This might eventually be where the mortar segment mesh and all the other data
  // structures get rebuilt?

  RelationshipManager::mesh_reinit();
}

void
AugmentSparsityOnInterface::internalInitWithMesh(const MeshBase &)
{
}

std::string
AugmentSparsityOnInterface::getInfo() const
{
  std::ostringstream oss;
  oss << "AugmentSparsityOnInterface";
  return oss.str();
}

void
AugmentSparsityOnInterface::operator()(const MeshBase::const_element_iterator & range_begin,
                                       const MeshBase::const_element_iterator & range_end,
                                       const processor_id_type p,
                                       map_type & coupled_elements)
{
  // We ask the user to pass boundary names instead of ids to our constraint object.  However, We
  // are unable to get the boundary ids from boundary names until we've attached the MeshBase object
  // to the MooseMesh
  bool generating_mesh = !_moose_mesh->getMeshPtr();
  auto primary_boundary_id = generating_mesh ? Moose::INVALID_BOUNDARY_ID
                                             : _moose_mesh->getBoundaryID(_primary_boundary_name);
  auto secondary_boundary_id = generating_mesh
                                   ? Moose::INVALID_BOUNDARY_ID
                                   : _moose_mesh->getBoundaryID(_secondary_boundary_name);
  auto primary_subdomain_id = generating_mesh
                                  ? Moose::INVALID_BLOCK_ID
                                  : _moose_mesh->getSubdomainID(_primary_subdomain_name);
  auto secondary_subdomain_id = generating_mesh
                                    ? Moose::INVALID_BLOCK_ID
                                    : _moose_mesh->getSubdomainID(_secondary_subdomain_name);

  const AutomaticMortarGeneration * const amg =
      _app.getExecutioner() ? &_app.getExecutioner()->feProblem().getMortarInterface(
                                  std::make_pair(primary_boundary_id, secondary_boundary_id),
                                  std::make_pair(primary_subdomain_id, secondary_subdomain_id),
                                  _use_displaced_mesh)
                            : nullptr;

  const CouplingMatrix * const null_mat = libmesh_nullptr;

  // If we're on a dynamic mesh or we have not yet constructed the mortar mesh, we need to ghost the
  // entire interface because we don't know a priori what elements will project onto what. We *do
  // not* add the whole interface if we are a coupling functor because it is very expensive. This is
  // because when building the sparsity pattern, we call through to the ghosting functors with one
  // element at a time (and then below we do a loop over all the mesh's active elements). It's
  // perhaps faster in this case to deal with mallocs coming out of MatSetValues, especially if the
  // mesh displacements are relatively small
  if ((!amg || _use_displaced_mesh) && !_is_coupling_functor)
  {
    for (const Elem * const elem : _mesh->active_element_ptr_range())
    {
      if (generating_mesh)
      {
        // We are still generating the mesh, so it's possible we don't even have the right boundary
        // ids created yet! So we actually ghost all boundary elements and all lower dimensional
        // elements who have parents on a boundary
        if (elem->on_boundary())
          coupled_elements.insert(std::make_pair(elem, null_mat));
        else if (const Elem * const ip = elem->interior_parent())
        {
          if (ip->on_boundary())
            coupled_elements.insert(std::make_pair(elem, null_mat));
        }
      }
      else
      {
        // We've finished generating our mesh so we can be selective and only ghost elements lying
        // in our lower-dimensional subdomains and their interior parents

        mooseAssert(primary_boundary_id != Moose::INVALID_BOUNDARY_ID,
                    "Primary boundary id should exist by now. If you're using a MeshModifier "
                    "please use the corresponding MeshGenerator instead");
        mooseAssert(secondary_boundary_id != Moose::INVALID_BOUNDARY_ID,
                    "Secondary boundary id should exist by now. If you're using a MeshModifier "
                    "please use the corresponding MeshGenerator instead");
        mooseAssert(primary_subdomain_id != Moose::INVALID_BLOCK_ID,
                    "Primary subdomain id should exist by now. If you're using a MeshModifier "
                    "please use the corresponding MeshGenerator instead");
        mooseAssert(secondary_subdomain_id != Moose::INVALID_BLOCK_ID,
                    "Secondary subdomain id should exist by now. If you're using a MeshModifier "
                    "please use the corresponding MeshGenerator instead");

        // Higher-dimensional boundary elements
        const BoundaryInfo & binfo = _mesh->get_boundary_info();

        for (auto side : elem->side_index_range())
          if ((elem->processor_id() != p) &&
              (binfo.has_boundary_id(elem, side, primary_boundary_id) ||
               binfo.has_boundary_id(elem, side, secondary_boundary_id)))
            coupled_elements.insert(std::make_pair(elem, null_mat));

        // Lower dimensional subdomain elements
        if ((elem->processor_id() != p) && (elem->subdomain_id() == primary_subdomain_id ||
                                            elem->subdomain_id() == secondary_subdomain_id))
        {
          coupled_elements.insert(std::make_pair(elem, null_mat));

#ifndef NDEBUG
          // let's do some safety checks
          const Elem * const ip = elem->interior_parent();
          mooseAssert(ip,
                      "We should have set interior parents for all of our lower-dimensional mortar "
                      "subdomains");
          auto side = ip->which_side_am_i(elem);
          auto bnd_id = elem->subdomain_id() == primary_subdomain_id ? primary_boundary_id
                                                                     : secondary_boundary_id;
          mooseAssert(_mesh->get_boundary_info().has_boundary_id(ip, side, bnd_id),
                      "The interior parent for the lower-dimensional element does not lie on the "
                      "boundary");
#endif
        }
      }
    }
  }
  // For a static mesh (or for determining a sparsity pattern approximation on a displaced mesh) we
  // can just ghost the coupled elements determined during mortar mesh generation
  else if (amg)
  {
    auto ghost_mortar_interface_couplings =
        [this, p, &coupled_elements, null_mat, amg](const Elem * const elem_arg)
    {
      // Look up elem_arg in the mortar_interface_coupling data structure.
      const auto & mic = amg->mortarInterfaceCoupling();
      auto find_it = mic.find(elem_arg->id());
      if (find_it == mic.end())
        return;

      const auto & coupled_set = find_it->second;

      for (const auto coupled_elem_id : coupled_set)
      {
        const Elem * coupled_elem = _mesh->elem_ptr(coupled_elem_id);
        mooseAssert(coupled_elem,
                    "The coupled element with id " << coupled_elem_id << " doesn't exist!");

        if (coupled_elem->processor_id() != p)
          coupled_elements.insert(std::make_pair(coupled_elem, null_mat));
      }
    };

    for (const Elem * const elem : as_range(range_begin, range_end))
    {
      ghost_mortar_interface_couplings(elem);

      if (_ghost_point_neighbors)
      {
        // I hypothesize that node processor ids are tied to higher dimensional element processor
        // ids over lower dimensional element processor ids based on debugging experience.
        // Consequently we need to be checking for higher dimensional elements and whether they are
        // along our secondary boundary. From there we will query the AMG object's
        // mortar-interface-coupling container to get the secondary lower-dimensional element, and
        // then we will ghost it's point neighbors and their mortar interface couples

        // It's possible that one higher-dimensional element could have multiple lower-dimensional
        // elements from multiple sides. Morever, even if there is only one lower-d element per
        // higher-d element, the unordered_multimap that holds the coupling information can have
        // duplicate key-value pairs if there are multiple mortar segments per secondary face. So to
        // prevent attempting to insert into the coupled elements map multiple times with the same
        // element, we'll keep track of the elements we've handled. We're going to use a tree-based
        // set here since the number of lower-d elements handled should never exceed the number of
        // element sides (which is small)
        std::set<dof_id_type> secondary_lower_elems_handled;
        const BoundaryInfo & binfo = _mesh->get_boundary_info();
        for (auto side : elem->side_index_range())
        {
          if (!binfo.has_boundary_id(elem, side, secondary_boundary_id))
            // We're not a higher-dimensional element along the secondary face, or at least this
            // side isn't
            continue;

          const auto & mic = amg->mortarInterfaceCoupling();
          auto find_it = mic.find(elem->id());
          if (find_it == mic.end())
            continue;

          const auto & coupled_set = find_it->second;
          for (const auto coupled_elem_id : coupled_set)
          {
            auto * const coupled_elem = _mesh->elem_ptr(coupled_elem_id);

            if (coupled_elem->subdomain_id() != secondary_subdomain_id)
            {
              // We support higher-d-secondary to higher-d-primary coupling now, e.g.
              // if we get here, coupled_elem is not actually a secondary lower elem; it's a
              // primary higher-d elem
              mooseAssert(coupled_elem->dim() == elem->dim(), "These should be matching dim");
              continue;
            }

            auto insert_pr = secondary_lower_elems_handled.insert(coupled_elem_id);

            // If insertion didn't happen, then we've already handled this element
            if (!insert_pr.second)
              continue;

            // We've already ghosted the secondary lower-d element itself if it needed to be
            // outside of the _ghost_point_neighbors logic. But now we must make sure to ghost the
            // point neighbors of the secondary lower-d element and their mortar interface
            // couplings
            std::set<const Elem *> secondary_lower_elem_point_neighbors;
            coupled_elem->find_point_neighbors(secondary_lower_elem_point_neighbors);

            for (const Elem * const neigh : secondary_lower_elem_point_neighbors)
            {
              if (neigh->processor_id() != p)
                coupled_elements.emplace(neigh, null_mat);

              ghost_mortar_interface_couplings(neigh);
            }
          } // end iteration over multimap

          // We actually should have added all the lower-dimensional elements associated with the
          // higher-dimensional element, so we can stop iterating over sides
          break;

        } // end for side_index_range
      }   // end if ghost_point_neighbors
    }     // end for loop over input range
  }       // end if amg
}

bool
AugmentSparsityOnInterface::operator>=(const RelationshipManager & other) const
{
  if (auto asoi = dynamic_cast<const AugmentSparsityOnInterface *>(&other))
  {
    if (_primary_boundary_name == asoi->_primary_boundary_name &&
        _secondary_boundary_name == asoi->_secondary_boundary_name &&
        _primary_subdomain_name == asoi->_primary_subdomain_name &&
        _secondary_subdomain_name == asoi->_secondary_subdomain_name &&
        (_ghost_point_neighbors >= asoi->_ghost_point_neighbors) && baseGreaterEqual(*asoi))
      return true;
  }
  return false;
}
