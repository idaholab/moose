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
    _secondary_subdomain_name(getParam<SubdomainName>("secondary_subdomain"))
{
}

void
AugmentSparsityOnInterface::mesh_reinit()
{
  // This might eventually be where the mortar segment mesh and all the other data
  // structures get rebuilt?
}

void
AugmentSparsityOnInterface::internalInit()
{
  if (_mesh.isDistributedMesh())
    mooseError(
        "We need to first be able to run MeshModifiers before remote elements are deleted before "
        "the AugmentSparsityOnInterface ghosting functor can work with DistributedMesh");
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
  // Note that as indicated by our error in internalInit this ghosting functor will not work on a
  // distributed mesh. This logic below will have to changed in order to support distributed
  // mesh. The FEProblemBase and Executioner do not get created until after the mesh has been
  // prepared and we have potentially deleted remote elements (although we do now have code that
  // illustrates delaying deletion of remote elements until after the equation systems init,
  // e.g. until after we've run ghosting functors on the DofMap
  if (!_has_attached_amg && _app.getExecutioner())
  {
    // We ask the user to pass boundary names instead of ids to our constraint object.  We are
    // unable to get the boundary ids until we've read in the mesh, which is done after we add
    // geometric relationship managers. Hence we can't do the below in our constructor. Now that
    // we're doing ghosting we've definitely read in the mesh
    auto boundary_pair = std::make_pair(_mesh.getBoundaryID(_primary_boundary_name),
                                        _mesh.getBoundaryID(_secondary_boundary_name));
    _subdomain_pair.first = _mesh.getSubdomainID(_primary_subdomain_name);
    _subdomain_pair.second = _mesh.getSubdomainID(_secondary_subdomain_name);

    _amg = &_app.getExecutioner()->feProblem().getMortarInterface(
        boundary_pair, _subdomain_pair, _use_displaced_mesh);
    _has_attached_amg = true;
  }

  const CouplingMatrix * const null_mat = libmesh_nullptr;

  // If we're on a dynamic mesh, we need to ghost the entire interface because we don't know at the
  // beginning of the non-linear solve which elements will project onto which over the course of the
  // solve
  if (_use_displaced_mesh)
  {
    for (const auto & elem : _mesh.getMesh().active_element_ptr_range())
    {
      if (elem->subdomain_id() == _subdomain_pair.first ||
          elem->subdomain_id() == _subdomain_pair.second)
      {
        if (elem->processor_id() != p)
          coupled_elements.insert(std::make_pair(elem, null_mat));
        auto ip = elem->interior_parent();
        if (ip->processor_id() != p)
          coupled_elements.insert(std::make_pair(ip, null_mat));
      }
    }
  }
  // For a static mesh we can just ghost the cross interface neighbors calculated during mortar mesh
  // generation
  else if (_amg)
  {
    for (const auto & elem : as_range(range_begin, range_end))
    {
      // Look up elem in the mortar_interface_coupling data structure.
      auto bounds = _amg->mortar_interface_coupling.equal_range(elem->id());

      for (const auto & pr : as_range(bounds))
      {
        const Elem * cross_interface_neighbor = _mesh.getMesh().elem_ptr(pr.second);

        if (cross_interface_neighbor->processor_id() != p)
          coupled_elements.insert(std::make_pair(cross_interface_neighbor, null_mat));

        // If the cross_interface_neighbor is a lower-dimensional element with
        // an interior parent, add the interior parent to the
        // list of Elems coupled to us.
        const Elem * cross_interface_neighbor_ip = cross_interface_neighbor->interior_parent();
        if (cross_interface_neighbor_ip && cross_interface_neighbor_ip->processor_id() != p)
          coupled_elements.insert(std::make_pair(cross_interface_neighbor_ip, null_mat));
      } // end loop over bounds range

      // Finally add the interior parent of this element if it's not local
      auto elem_ip = elem->interior_parent();
      if (elem_ip && elem_ip->processor_id() != p)
        coupled_elements.insert(std::make_pair(elem_ip, null_mat));

    } // end loop over active local elements range
  }   // end if (_amg)
}

bool
AugmentSparsityOnInterface::operator==(const RelationshipManager & other) const
{
  if (auto asoi = dynamic_cast<const AugmentSparsityOnInterface *>(&other))
  {
    if (_primary_boundary_name == asoi->_primary_boundary_name &&
        _secondary_boundary_name == asoi->_secondary_boundary_name &&
        _primary_subdomain_name == asoi->_primary_subdomain_name &&
        _secondary_subdomain_name == asoi->_secondary_subdomain_name)
      return true;
  }
  return false;
}
