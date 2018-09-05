// App includes
#include "AugmentSparsityOnInterface.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseApp.h"

// libMesh includes
#include "libmesh/elem.h"

using namespace libMesh;

template <>
InputParameters
validParams<AugmentSparsityOnInterface>()
{
  InputParameters params = validParams<AlgebraicRelationshipManager>();
  params.addRequiredParam<unsigned>("slave_id", "The id of the slave sideset");
  params.addRequiredParam<unsigned>("master_id", "The id of the master sideset");
  return params;
}

AugmentSparsityOnInterface::AugmentSparsityOnInterface(const InputParameters & params)
  : AlgebraicRelationshipManager(params), _amg(nullptr), _has_attached_amg(false)
{
  unsigned slave_id = getParam<unsigned>("slave_id");
  unsigned master_id = getParam<unsigned>("master_id");
  _interface = std::make_pair(slave_id, master_id);
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
  if (_app.isSplitMesh() || _mesh.isDistributedMesh())
  {
    if (rm_type == Moose::RelationshipManagerType::Geometric)
      attachGeometricFunctorHelper(*this);
    else
      attachAlgebraicFunctorHelper(*this);
  }
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
    _amg = &_app.getExecutioner()->feProblem().getMortarInterface(_interface);
    _has_attached_amg = true;
  }

  const CouplingMatrix * const null_mat = libmesh_nullptr;

  for (MeshBase::const_element_iterator elem_it = range_begin; elem_it != range_end; ++elem_it)
  {
    const Elem * const elem = *elem_it;

    // This code is present in both the PointNeighborCoupling class that lives
    // in libmesh and the misc_ex9 code, so I think it's required but I don't really
    // understand it.
    if (elem->processor_id() != p)
      coupled_elements.insert(std::make_pair(elem, null_mat));

    // If this Elem has an interior_parent, add it to the list.
    const Elem * elem_ip = elem->interior_parent();
    if (elem_ip && elem_ip->processor_id() != p)
    {
      // libMesh::out << "Adding Elem " << elem_ip->id()
      //              << " as coupled Elem for " << elem->id()
      //              << "." << std::endl;
      coupled_elements.insert(std::make_pair(elem_ip, null_mat));
    }

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
