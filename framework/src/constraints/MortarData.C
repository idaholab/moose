#include "MortarData.h"
#include "SubProblem.h"
#include "MooseMesh.h"

MortarData::MortarData(SubProblem & subproblem) : _subproblem(subproblem) {}

AutomaticMortarGeneration &
MortarData::getMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                               const std::pair<SubdomainID, SubdomainID> & subdomain_key)
{
  _mortar_subdomain_coverage.insert(subdomain_key.first);
  _mortar_subdomain_coverage.insert(subdomain_key.second);

  if (_mortar_interfaces.find(boundary_key) == _mortar_interfaces.end())
    _mortar_interfaces.emplace(boundary_key,
                               libmesh_make_unique<AutomaticMortarGeneration>(
                                   _subproblem.mesh().getMesh(), boundary_key, subdomain_key));
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
    amg.build_node_to_elem_maps();

    ////////////////////////////////////////////////////////////////////////////////
    // Compute nodal normals.
    ////////////////////////////////////////////////////////////////////////////////
    amg.compute_nodal_normals();

    ////////////////////////////////////////////////////////////////////////////////
    // (Optional) Write nodal normals to file.
    ////////////////////////////////////////////////////////////////////////////////
    // amg.write_nodal_normals_to_file();

    ////////////////////////////////////////////////////////////////////////////////
    // Project slave nodes (find xi^(2) values).
    ////////////////////////////////////////////////////////////////////////////////
    amg.project_slave_nodes();

    ////////////////////////////////////////////////////////////////////////////////
    // Project master nodes (find xi^(1) values).
    ////////////////////////////////////////////////////////////////////////////////
    amg.project_master_nodes();

    ////////////////////////////////////////////////////////////////////////////////
    // Build the mortar segment mesh on the slave boundary.
    ////////////////////////////////////////////////////////////////////////////////
    amg.build_mortar_segment_mesh();
  }
}
