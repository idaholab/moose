// App includes
#include "AugmentSparsityOnInterface.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseApp.h"

// libMesh includes
#include "libmesh/elem.h"

registerMooseObject("MooseApp", AugmentSparsityOnInterface);

using namespace libMesh;

template <>
InputParameters
validParams<AugmentSparsityOnInterface>()
{
  InputParameters params = validParams<RelationshipManager>();
  params.addParam<BoundaryID>("master_boundary_id", "The id of the master boundary sideset.");
  params.addParam<BoundaryID>("slave_boundary_id", "The id of the slave boundary sideset.");
  params.addParam<BoundaryName>("master_boundary_name", "The name of the master boundary sideset.");
  params.addParam<BoundaryName>("slave_boundary_name", "The name of the slave boundary sideset.");

  return params;
}

AugmentSparsityOnInterface::AugmentSparsityOnInterface(const InputParameters & params)
  : RelationshipManager(params),
    _amg(nullptr),
    _has_attached_amg(false),
    _master_boundary_name(getParam<BoundaryName>("master_boundary_name")),
    _slave_boundary_name(getParam<BoundaryName>("slave_boundary_name"))
{
  BoundaryID slave_boundary_id = getParam<BoundaryID>("slave_boundary_id");
  BoundaryID master_boundary_id = getParam<BoundaryID>("master_boundary_id");
  _interface = std::make_pair(master_boundary_id, slave_boundary_id);
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
    // The user may have passed boundary names instead of ids to our constraint object in which case
    // we are unable to get the boundary ids until we've read in the mesh, which is done after we
    // add geometric relationship managers. Hence we can't do the below in our constructor. Now that
    // we're doing ghosting we've definitely read in the mesh
    if (_interface.first == Moose::INVALID_BOUNDARY_ID)
      _interface.first = _mesh.getBoundaryID(_master_boundary_name);
    if (_interface.second == Moose::INVALID_BOUNDARY_ID)
      _interface.second = _mesh.getBoundaryID(_slave_boundary_name);

    _amg = &_app.getExecutioner()->feProblem().getMortarInterface(_interface, _subdomain_pair);
    _has_attached_amg = true;
  }

  const CouplingMatrix * const null_mat = libmesh_nullptr;

  // If we're on a dynamic mesh, we need to ghost the entire interface because we don't know at the
  // beginning of the non-linear solve which elements will project onto which over the course of the
  // solve
  if (_use_displaced_mesh)
  {
    for (const auto & elem : as_range(range_begin, range_end))
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
    }   // end loop over active local elements range
  }     // end if (_amg)
}

bool
AugmentSparsityOnInterface::operator==(const RelationshipManager & other) const
{
  if (auto asoi = dynamic_cast<const AugmentSparsityOnInterface *>(&other))
  {
    if (_interface.first == asoi->_interface.first &&
        _interface.second == asoi->_interface.second &&
        _master_boundary_name == asoi->_master_boundary_name &&
        _slave_boundary_name == asoi->_slave_boundary_name)
      return true;
  }
  return false;
}
