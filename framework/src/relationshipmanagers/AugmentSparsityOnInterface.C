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
  InputParameters params = validParams<AlgebraicRelationshipManager>();
  params.addRequiredParam<BoundaryID>("master_boundary_id",
                                      "The id of the master boundary sideset.");
  params.addRequiredParam<BoundaryID>("slave_boundary_id", "The id of the slave boundary sideset.");
  params.addRequiredParam<SubdomainID>("master_subdomain_id", "The id of the master subdomain.");
  params.addRequiredParam<SubdomainID>("slave_subdomain_id", "The id of the slave subdomain.");
  return params;
}

AugmentSparsityOnInterface::AugmentSparsityOnInterface(const InputParameters & params)
  : AlgebraicRelationshipManager(params), _amg(nullptr), _has_attached_amg(false)
{
  BoundaryID slave_boundary_id = getParam<BoundaryID>("slave_boundary_id");
  BoundaryID master_boundary_id = getParam<BoundaryID>("master_boundary_id");
  _interface = std::make_pair(master_boundary_id, slave_boundary_id);
  SubdomainID slave_subdomain_id = getParam<SubdomainID>("slave_subdomain_id");
  SubdomainID master_subdomain_id = getParam<SubdomainID>("master_subdomain_id");
  _subdomain_pair = std::make_pair(master_subdomain_id, slave_subdomain_id);

  _rm_type = Moose::RelationshipManagerType::ALGEBRAIC;
}

void
AugmentSparsityOnInterface::mesh_reinit()
{
  // This might eventually be where the mortar segment mesh and all the other data
  // structures get rebuilt?
}

void
AugmentSparsityOnInterface::attachRelationshipManagersInternal(
    Moose::RelationshipManagerType rm_type)
{
  if (rm_type == Moose::RelationshipManagerType::GEOMETRIC)
    attachGeometricFunctorHelper(*this);
  else
    attachAlgebraicFunctorHelper(*this);
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
  if (!_has_attached_amg)
  {
    if (_app.getExecutioner())
    {
      _amg = &_app.getExecutioner()->feProblem().getMortarInterface(_interface, _subdomain_pair);
      _has_attached_amg = true;
    }
    else
      return;
  }

  const CouplingMatrix * const null_mat = libmesh_nullptr;

  for (MeshBase::const_element_iterator elem_it = range_begin; elem_it != range_end; ++elem_it)
  {
    const Elem * const elem = *elem_it;

    // Look up elem in the mortar_interface_coupling data structure.
    auto bounds = _amg->mortar_interface_coupling.equal_range(elem);
    for (const auto & pr : as_range(bounds))
    {
      const Elem * cross_interface_neighbor = pr.second;

      if (cross_interface_neighbor->processor_id() != p)
      {
        // libMesh::out << "Adding Elem " << cross_interface_neighbor->id()
        //              << " as coupled Elem for " << pr.first->id()
        //              << "." << std::endl;
        coupled_elements.insert(std::make_pair(cross_interface_neighbor, null_mat));
      }

      // If the cross_interface_neighbor is a lower-dimensional element with
      // an interior parent, add the interior parent to the
      // list of Elems coupled to us.
      const Elem * cross_interface_neighbor_ip = cross_interface_neighbor->interior_parent();
      if (cross_interface_neighbor_ip && cross_interface_neighbor_ip->processor_id() != p)
      {
        // libMesh::out << "Adding Elem " << cross_interface_neighbor_ip->id()
        //              << " as coupled Elem for " << pr.first->id()
        //              << "." << std::endl;
        coupled_elements.insert(std::make_pair(cross_interface_neighbor_ip, null_mat));
      }
    }
  } // end loop over range
}
