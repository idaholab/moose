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

defineLegacyParams(AugmentSparsityOnInterface);

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
  return params;
}

AugmentSparsityOnInterface::AugmentSparsityOnInterface(const InputParameters & params)
  : RelationshipManager(params),
    _amg(nullptr),
    _has_attached_amg(false),
    _primary_boundary_name(getParam<BoundaryName>("primary_boundary")),
    _secondary_boundary_name(getParam<BoundaryName>("secondary_boundary")),
    _primary_subdomain_name(getParam<SubdomainName>("primary_subdomain")),
    _secondary_subdomain_name(getParam<SubdomainName>("secondary_subdomain")),
    _is_coupling_functor(isType(Moose::RelationshipManagerType::COUPLING))
{
}

AugmentSparsityOnInterface::AugmentSparsityOnInterface(const AugmentSparsityOnInterface & other)
  : RelationshipManager(other),
    _amg(other._amg),
    _has_attached_amg(other._has_attached_amg),
    _primary_boundary_name(other._primary_boundary_name),
    _secondary_boundary_name(other._secondary_boundary_name),
    _primary_subdomain_name(other._primary_subdomain_name),
    _secondary_subdomain_name(other._secondary_subdomain_name),
    _is_coupling_functor(other._is_coupling_functor)
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
AugmentSparsityOnInterface::internalInit()
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
                                       processor_id_type p,
                                       map_type & coupled_elements)
{
  // We ask the user to pass boundary names instead of ids to our constraint object.  We are
  // unable to get the boundary ids until we've read in the mesh, which is done after we add
  // geometric relationship managers. Hence we can't do the below in our constructor.
  bool generating_mesh = !_moose_mesh->getMeshPtr();
  auto primary_boundary_id =
      generating_mesh ? Moose::ANY_BOUNDARY_ID : _moose_mesh->getBoundaryID(_primary_boundary_name);
  auto secondary_boundary_id = generating_mesh
                                   ? Moose::ANY_BOUNDARY_ID
                                   : _moose_mesh->getBoundaryID(_secondary_boundary_name);
  auto primary_subdomain_id = generating_mesh
                                  ? Moose::INVALID_BLOCK_ID
                                  : _moose_mesh->getSubdomainID(_primary_subdomain_name);
  auto secondary_subdomain_id = generating_mesh
                                    ? Moose::INVALID_BLOCK_ID
                                    : _moose_mesh->getSubdomainID(_secondary_subdomain_name);

  if (!_has_attached_amg && _app.getExecutioner())
  {
    _amg = &_app.getExecutioner()->feProblem().getMortarInterface(
        std::make_pair(primary_boundary_id, secondary_boundary_id),
        std::make_pair(primary_subdomain_id, secondary_subdomain_id),
        _use_displaced_mesh);
    _has_attached_amg = true;

    mooseAssert(
        !generating_mesh,
        "If we have an executioner, then we should definitely not still be generating the mesh.");
  }

  const CouplingMatrix * const null_mat = libmesh_nullptr;

  // If we're on a dynamic mesh or we have not yet constructed the mortar mesh, we need to ghost the
  // entire interface because we don't know a priori what elements will project onto what. We *do
  // not* add the whole interface if we are a coupling functor because based on profiling the cost
  // is very expensive. It's perhaps better in that case to deal with mallocs coming out of
  // MatSetValues, especially if the mesh displacements are relatively small
  if ((!_amg || _use_displaced_mesh) && !_is_coupling_functor)
  {
    for (const auto & elem : _mesh->active_element_ptr_range())
    {
      if (_amg)
      {
        // If we have an AutomaticMortarGeneration object then we've definitely finished generating
        // our mesh, e.g. we've already added lower-dimensional elements. It is safe then to query
        // only based on the lower-dimensional subdomain ids
        if (elem->subdomain_id() == primary_subdomain_id ||
            elem->subdomain_id() == secondary_subdomain_id)
        {
          if (elem->processor_id() != p)
            coupled_elements.insert(std::make_pair(elem, null_mat));
          auto ip = elem->interior_parent();
          if (ip->processor_id() != p)
            coupled_elements.insert(std::make_pair(ip, null_mat));

#ifndef NDEBUG
          // let's do some safety checks
          auto side = ip->which_side_am_i(elem);

          auto bnd_id = elem->subdomain_id() == primary_subdomain_id ? primary_boundary_id
                                                                     : secondary_boundary_id;

          mooseAssert(
              _mesh->get_boundary_info().has_boundary_id(ip, side, bnd_id),
              "The interior parent for the lower-dimensional element does not lie on the boundary");
#endif
        }
      }
      else
      {
        // If we do not have an AutomaticMortarGeneration object then we may not have added our
        // lower-dimensional elements yet. Consequently to be safe we need to query based on the
        // boundary ids. Moreover, we may not even have the right boundary ids available yet! So we
        // actually ghost all boundary elements and all lower dimensional elements who have parents
        // on a boundary if we're still generating the mesh

        if (generating_mesh)
        {
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
          const BoundaryInfo & binfo = _mesh->get_boundary_info();

          for (auto side : elem->side_index_range())
            if ((elem->processor_id() != p) &&
                (binfo.has_boundary_id(elem, side, primary_boundary_id) ||
                 binfo.has_boundary_id(elem, side, secondary_boundary_id)))
              coupled_elements.insert(std::make_pair(elem, null_mat));

          // We still to need to add the lower-dimensional elements if they exist
          if ((elem->processor_id() != p) && (elem->subdomain_id() == primary_subdomain_id ||
                                              elem->subdomain_id() == secondary_subdomain_id))
            coupled_elements.insert(std::make_pair(elem, null_mat));
        }
      }
    }
  }
  // For a static mesh (or for determining a sparsity pattern approximation on a displaced mesh) we
  // can just ghost the coupled elements determined during mortar mesh generation
  else if (_amg)
  {
    for (const auto & elem : as_range(range_begin, range_end))
    {
      // Look up elem in the mortar_interface_coupling data structure.
      auto bounds = _amg->mortar_interface_coupling.equal_range(elem->id());

      for (const auto & pr : as_range(bounds))
      {
        auto coupled_elem_id = pr.second;
        const Elem * coupled_elem = _mesh->elem_ptr(coupled_elem_id);
        mooseAssert(coupled_elem,
                    "The coupled element with id " << coupled_elem_id << " doesn't exist!");

        if (coupled_elem->processor_id() != p)
          coupled_elements.insert(std::make_pair(coupled_elem, null_mat));
      }
    }
  }
}

bool
AugmentSparsityOnInterface::operator==(const RelationshipManager & other) const
{
  if (auto asoi = dynamic_cast<const AugmentSparsityOnInterface *>(&other))
  {
    if (_primary_boundary_name == asoi->_primary_boundary_name &&
        _secondary_boundary_name == asoi->_secondary_boundary_name &&
        _primary_subdomain_name == asoi->_primary_subdomain_name &&
        _secondary_subdomain_name == asoi->_secondary_subdomain_name && isType(asoi->_rm_type))
      return true;
  }
  return false;
}
