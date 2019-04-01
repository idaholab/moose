#include "MortarData.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "MooseError.h"

MortarData::MortarData() : _has_displaced_objects(false) {}

AutomaticMortarGeneration &
MortarData::getMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                               const std::pair<SubdomainID, SubdomainID> & subdomain_key,
                               SubProblem & subproblem,
                               bool on_displaced)
{
  if (on_displaced)
    _has_displaced_objects = true;

  _mortar_subdomain_coverage.insert(subdomain_key.first);
  _mortar_subdomain_coverage.insert(subdomain_key.second);

  if (_mortar_interfaces.find(boundary_key) == _mortar_interfaces.end())
    _mortar_interfaces.emplace(boundary_key,
                               libmesh_make_unique<AutomaticMortarGeneration>(
                                   subproblem.mesh().getMesh(), boundary_key, subdomain_key));
  return *_mortar_interfaces.at(boundary_key);
}

AutomaticMortarGeneration &
MortarData::getMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                               const std::pair<SubdomainID, SubdomainID> & /*subdomain_key*/)
{
  if (_mortar_interfaces.find(boundary_key) == _mortar_interfaces.end())
    mooseError(
        "The requested mortar interface AutomaticMortarGeneration object does not yet exist!");

  return *_mortar_interfaces.at(boundary_key);
}

void
MortarData::update()
{
  for (auto & mortar_pair : _mortar_interfaces)
  {
    auto & amg = *mortar_pair.second;

    ////////////////////////////////////////////////////////////////////////////////
    // Construct maps from nodes -> lower dimensional elements on the master and slave boundaries.
    ////////////////////////////////////////////////////////////////////////////////
    amg.buildNodeToElemMaps();

    ////////////////////////////////////////////////////////////////////////////////
    // Compute nodal normals.
    ////////////////////////////////////////////////////////////////////////////////
    amg.computeNodalNormals();

    ////////////////////////////////////////////////////////////////////////////////
    // (Optional) Write nodal normals to file.
    ////////////////////////////////////////////////////////////////////////////////
    // amg.writeNodalNormalsToFile();

    ////////////////////////////////////////////////////////////////////////////////
    // Project slave nodes (find xi^(2) values).
    ////////////////////////////////////////////////////////////////////////////////
    amg.projectSlaveNodes();

    ////////////////////////////////////////////////////////////////////////////////
    // Project master nodes (find xi^(1) values).
    ////////////////////////////////////////////////////////////////////////////////
    amg.projectMasterNodes();

    ////////////////////////////////////////////////////////////////////////////////
    // Build the mortar segment mesh on the slave boundary.
    ////////////////////////////////////////////////////////////////////////////////
    amg.buildMortarSegmentMesh();
  }
}
