#include "DetailedTriPinMeshGenerator.h"
#include "TriSubChannelMesh.h"
#include "libmesh/cell_prism6.h"

registerMooseObject("SubChannelApp", DetailedTriPinMeshGenerator);

InputParameters
DetailedTriPinMeshGenerator::validParams()
{
  InputParameters params = DetailedPinMeshGeneratorBase::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The corresponding subchannel mesh");
  params.addRequiredParam<unsigned int>("nrings", "Number of fuel rod rings per assembly [-]");
  return params;
}

DetailedTriPinMeshGenerator::DetailedTriPinMeshGenerator(const InputParameters & parameters)
  : DetailedPinMeshGeneratorBase(parameters),
    _input(getMesh("input")),
    _n_rings(getParam<unsigned int>("nrings"))
{
}

std::unique_ptr<MeshBase>
DetailedTriPinMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh_base = std::move(_input);
  if (!mesh_base)
    mesh_base = buildMeshBaseObject();
  mesh_base->set_mesh_dimension(3);

  std::vector<Point> pin_centers;
  TriSubChannelMesh::rodPositions(pin_centers, _n_rings, _pitch, Point(0, 0));

  _elem_id = mesh_base->n_elem();
  for (auto & ctr : pin_centers)
    generatePin(mesh_base, ctr);

  mesh_base->subdomain_name(_block_id) = name();
  mesh_base->prepare_for_use();

  return mesh_base;
}
