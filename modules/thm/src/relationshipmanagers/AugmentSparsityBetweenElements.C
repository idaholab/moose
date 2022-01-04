#include "AugmentSparsityBetweenElements.h"
#include "libmesh/elem.h"

registerMooseObject("THMApp", AugmentSparsityBetweenElements);

using namespace libMesh;

InputParameters
AugmentSparsityBetweenElements::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  params.addRequiredParam<std::map<dof_id_type, std::vector<dof_id_type>> *>(
      "_elem_map", "Element to element augmentation map");
  return params;
}

AugmentSparsityBetweenElements::AugmentSparsityBetweenElements(const InputParameters & params)
  : RelationshipManager(params),
    _elem_map(*getParam<std::map<dof_id_type, std::vector<dof_id_type>> *>("_elem_map"))
{
}

void
AugmentSparsityBetweenElements::mesh_reinit()
{
}

void
AugmentSparsityBetweenElements::internalInit()
{
}

std::string
AugmentSparsityBetweenElements::getInfo() const
{
  std::ostringstream oss;
  oss << "AugmentSparsityBetweenElements";
  return oss.str();
}

void
AugmentSparsityBetweenElements::operator()(const MeshBase::const_element_iterator & range_begin,
                                           const MeshBase::const_element_iterator & range_end,
                                           processor_id_type /*p*/,
                                           map_type & coupled_elements)
{
  const CouplingMatrix * const null_mat = libmesh_nullptr;
  for (const auto & elem : as_range(range_begin, range_end))
  {
    auto it = _elem_map.find(elem->id());
    if (it != _elem_map.end())
    {
      for (auto & coupled_elem_id : it->second)
        coupled_elements.insert(std::make_pair(_moose_mesh->elemPtr(coupled_elem_id), null_mat));
    }
  }
}

bool
AugmentSparsityBetweenElements::operator>=(const RelationshipManager & rhs) const
{
  const auto * const rm = dynamic_cast<const AugmentSparsityBetweenElements *>(&rhs);
  if (!rm)
    return false;

  return (_elem_map == rm->_elem_map) && baseGreaterEqual(rhs);
}
