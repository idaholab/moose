#include "FakeNeighborRM.h"
#include "MooseApp.h"
#include "FEProblemBase.h"
#include "libmesh/dof_map.h"
#include "libmesh/coupling_matrix.h"

using namespace libMesh;

registerMooseObject("MooseApp", FakeNeighborRM);

namespace
{
class FakeNeighborFunctorImpl : public GhostingFunctor
{
public:
  explicit FakeNeighborFunctorImpl(
      const std::unordered_map<std::pair<const Elem *, unsigned int>,
                               std::pair<const Elem *, unsigned int>> & map)
    : _map(map), _dof_coupling(nullptr)
  {
  }

  void operator()(const MeshBase::const_element_iterator & range_begin,
                  const MeshBase::const_element_iterator & range_end,
                  processor_id_type p,
                  map_type & coupled_elements) override
  {
    for (const auto & elem : as_range(range_begin, range_end))
    {
      if (elem->processor_id() != p)
        coupled_elements.emplace(elem, nullptr);

      for (unsigned int side = 0; side < elem->n_sides(); ++side)
      {
        auto it = _map.find(std::make_pair(elem, side));
        if (it == _map.end())
          continue;

        const Elem * neigh = it->second.first;
        if (neigh && neigh->processor_id() != p)
          coupled_elements.emplace(neigh, _dof_coupling);
      }
    }
  }

  void set_dof_coupling(const CouplingMatrix * dof_coupling) { _dof_coupling = dof_coupling; }

  bool map_empty() const { return _map.empty(); }

private:
  const std::unordered_map<std::pair<const Elem *, unsigned int>,
                           std::pair<const Elem *, unsigned int>> & _map;
  const CouplingMatrix * _dof_coupling;
};
}

InputParameters
FakeNeighborRM::validParams()
{
  // No custom parameters needed, we set the data with a method call.
  return FunctorRelationshipManager::validParams();
}

FakeNeighborRM::FakeNeighborRM(const InputParameters & params) : FunctorRelationshipManager(params)
{
}

void
FakeNeighborRM::setFakeNeighborMap(
    const std::unordered_map<std::pair<const libMesh::Elem *, unsigned int>,
                             std::pair<const libMesh::Elem *, unsigned int>> & fake_neighbor_map)
{
  // Make our own safe copy of the map data.
  _elem_side_to_fake_neighbor_elem_side = fake_neighbor_map;
}

void
FakeNeighborRM::internalInitWithMesh(const MeshBase &)
{
  // Create the functor, passing a const reference to our OWN copy of the map.
  if (!_functor)
    _functor = std::make_unique<FakeNeighborFunctorImpl>(_elem_side_to_fake_neighbor_elem_side);
  else if (auto fn = dynamic_cast<FakeNeighborFunctorImpl *>(_functor.get()))
  {
    if (fn->map_empty() && !_elem_side_to_fake_neighbor_elem_side.empty())
      _functor = std::make_unique<FakeNeighborFunctorImpl>(_elem_side_to_fake_neighbor_elem_side);
  }
}

void
FakeNeighborRM::dofmap_reinit()
{
  if (_dof_map)
    static_cast<FakeNeighborFunctorImpl *>(_functor.get())
        ->set_dof_coupling(_dof_map->_dof_coupling);
}

// Implementation of required pure virtual functions
std::string
FakeNeighborRM::getInfo() const
{
  return "FakeNeighborRM (ghosts across artificial neighbors)";
}

bool
FakeNeighborRM::operator>=(const RelationshipManager & rhs) const
{
  const auto * rm = dynamic_cast<const FakeNeighborRM *>(&rhs);
  if (!rm)
    return false;
  return baseGreaterEqual(*rm);
}

std::unique_ptr<GhostingFunctor>
FakeNeighborRM::clone() const
{
  return _app.getFactory().copyConstruct(*this);
}

void
FakeNeighborRM::set_mesh(const libMesh::MeshBase * mesh)
{
  // 1. Just-in-time creation
  if (!_functor)
    _functor = std::make_unique<FakeNeighborFunctorImpl>(_elem_side_to_fake_neighbor_elem_side);

  // 2. Now that we guarantee _functor exists, safely call the base class version.
  FunctorRelationshipManager::set_mesh(mesh);
}
