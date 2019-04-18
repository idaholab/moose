#include "MortarData.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "MooseError.h"

MortarData::MortarData() {}

AutomaticMortarGeneration &
MortarData::getMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                               const std::pair<SubdomainID, SubdomainID> & subdomain_key,
                               SubProblem & subproblem,
                               bool on_displaced)
{
  _mortar_subdomain_coverage.insert(subdomain_key.first);
  _mortar_subdomain_coverage.insert(subdomain_key.second);

  _mortar_boundary_coverage.insert(boundary_key.first);
  _mortar_boundary_coverage.insert(boundary_key.second);

  if (on_displaced)
  {
    if (_displaced_mortar_interfaces.find(boundary_key) == _displaced_mortar_interfaces.end())
      _displaced_mortar_interfaces.emplace(
          boundary_key,
          AutomaticMortarGeneration(
              subproblem.mesh().getMesh(), boundary_key, subdomain_key, on_displaced));
    return _displaced_mortar_interfaces.at(boundary_key);
  }
  else
  {
    if (_mortar_interfaces.find(boundary_key) == _mortar_interfaces.end())
      _mortar_interfaces.emplace(
          boundary_key,
          AutomaticMortarGeneration(
              subproblem.mesh().getMesh(), boundary_key, subdomain_key, on_displaced));
    return _mortar_interfaces.at(boundary_key);
  }
}

AutomaticMortarGeneration &
MortarData::getMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                               const std::pair<SubdomainID, SubdomainID> & /*subdomain_key*/,
                               bool on_displaced)
{
  if (on_displaced)
  {
    if (_displaced_mortar_interfaces.find(boundary_key) == _displaced_mortar_interfaces.end())
      mooseError(
          "The requested mortar interface AutomaticMortarGeneration object does not yet exist!");

    return _displaced_mortar_interfaces.at(boundary_key);
  }
  else
  {
    if (_mortar_interfaces.find(boundary_key) == _mortar_interfaces.end())
      mooseError(
          "The requested mortar interface AutomaticMortarGeneration object does not yet exist!");

    return _mortar_interfaces.at(boundary_key);
  }
}

void
MortarData::update()
{
  for (auto & mortar_pair : _mortar_interfaces)
  {
    auto & amg = mortar_pair.second;

    // Clear exiting data
    amg.clear();

    // Construct maps from nodes -> lower dimensional elements on the master and slave boundaries.
    amg.buildNodeToElemMaps();

    // Compute nodal normals.
    amg.computeNodalNormals();

    // (Optional) Write nodal normals to file.
    // amg.writeNodalNormalsToFile();

    // Project slave nodes (find xi^(2) values).
    amg.projectSlaveNodes();

    // Project master nodes (find xi^(1) values).
    amg.projectMasterNodes();

    // Build the mortar segment mesh on the slave boundary.
    amg.buildMortarSegmentMesh();
  }
}
