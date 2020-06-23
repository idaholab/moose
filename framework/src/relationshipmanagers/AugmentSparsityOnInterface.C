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

AugmentSparsityOnInterface::AugmentSparsityOnInterface(const AugmentSparsityOnInterface & others)
  : RelationshipManager(others),
    _amg(others._amg),
    _has_attached_amg(others._has_attached_amg),
    _primary_boundary_name(others._primary_boundary_name),
    _secondary_boundary_name(others._secondary_boundary_name),
    _primary_subdomain_name(others._primary_boundary_name),
    _secondary_subdomain_name(others._secondary_boundary_name)
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
  // geometric relationship managers. Hence we can't do the below in our constructor. Now that
  // we're doing ghosting we've definitely read in the mesh
  auto primary_boundary_id = _mesh.getBoundaryID(_primary_boundary_name);
  auto secondary_boundary_id = _mesh.getBoundaryID(_secondary_boundary_name);
  auto primary_subdomain_id = _mesh.getSubdomainID(_primary_subdomain_name);
  auto secondary_subdomain_id = _mesh.getSubdomainID(_secondary_subdomain_name);

  // Note that as indicated by our error in internalInit this ghosting functor will not work on a
  // distributed mesh. This logic below will have to changed in order to support distributed
  // mesh. The FEProblemBase and Executioner do not get created until after the mesh has been
  // prepared and we have potentially deleted remote elements (although we do now have code that
  // illustrates delaying deletion of remote elements until after the equation systems init,
  // e.g. until after we've run ghosting functors on the DofMap
  if (!_has_attached_amg && _app.getExecutioner())
  {
    _amg = &_app.getExecutioner()->feProblem().getMortarInterface(
        std::make_pair(primary_boundary_id, secondary_boundary_id),
        std::make_pair(primary_subdomain_id, secondary_subdomain_id),
        _use_displaced_mesh);
    _has_attached_amg = true;
  }

  const CouplingMatrix * const null_mat = libmesh_nullptr;

  // If we're on a dynamic mesh or we have not yet constructed the mortar mesh, we need to ghost the
  // entire interface because we don't know a priori what elements will project onto what. We *do
  // not* add the whole interface if we are a coupling functor because based on profiling the cost
  // is very expensive. It's perhaps better in that case to deal with mallocs coming out of
  // MatSetValues, especially if the mesh displacements are relatively small
  if ((!_amg || _use_displaced_mesh) && !_is_coupling_functor)
  {
    for (const auto & elem : _moose_mesh->getMesh().active_element_ptr_range())
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
              _mesh.getMesh().get_boundary_info().has_boundary_id(ip, side, bnd_id),
              "The interior parent for the lower-dimensional element does not lie on the boundary");
#endif
        }
      }
      else
      {
        // If we do not have an AutomaticMortarGeneration object then we may not have added our
        // lower-dimensional elements yet. Consequently to be safe we need to query based on the
        // boundary ids

        const BoundaryInfo & binfo = _mesh.getMesh().get_boundary_info();

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
        const Elem * coupled_elem = _mesh.getMesh().elem_ptr(coupled_elem_id);
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
