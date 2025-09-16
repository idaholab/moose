#pragma once

#include "FunctorRelationshipManager.h"
#include <unordered_map>
#include "libmesh/elem.h"

class FakeNeighborRM : public FunctorRelationshipManager
{
public:
  static InputParameters validParams();
  FakeNeighborRM(const InputParameters & params);

  // Required virtual functions from base classes
  std::string getInfo() const override;
  bool operator>=(const RelationshipManager & rhs) const override;
  std::unique_ptr<libMesh::GhostingFunctor> clone() const override;

  // Public method to set the map data
  void setFakeNeighborMap(
      const std::unordered_map<std::pair<const libMesh::Elem *, unsigned int>,
                               std::pair<const libMesh::Elem *, unsigned int>> & fake_neighbor_map);

protected:
  virtual void internalInitWithMesh(const MeshBase &) override;
  void dofmap_reinit() override;
  void set_mesh(const libMesh::MeshBase * mesh) override;

private:
  // The RM owns its own copy of the map.
  std::unordered_map<std::pair<const libMesh::Elem *, unsigned int>,
                     std::pair<const libMesh::Elem *, unsigned int>>
      _elem_side_to_fake_neighbor_elem_side;
};
