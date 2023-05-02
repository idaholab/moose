/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "DetailedQuadPinMeshGenerator.h"
#include "QuadSubChannelMesh.h"
#include "libmesh/cell_prism6.h"

registerMooseObject("SubChannelApp", DetailedQuadPinMeshGenerator);

InputParameters
DetailedQuadPinMeshGenerator::validParams()
{
  InputParameters params = DetailedPinMeshGeneratorBase::validParams();
  params.addClassDescription("Creates detailed mesh of fuel pins in a square lattice arrangement");
  params.addRequiredParam<MeshGeneratorName>("input", "The corresponding subchannel mesh");
  params.addRequiredParam<unsigned int>("nx", "Number of channels in the x direction [-]");
  params.addRequiredParam<unsigned int>("ny", "Number of channels in the y direction [-]");
  return params;
}

DetailedQuadPinMeshGenerator::DetailedQuadPinMeshGenerator(const InputParameters & parameters)
  : DetailedPinMeshGeneratorBase(parameters),
    _input(getMesh("input")),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny"))
{
}

std::unique_ptr<MeshBase>
DetailedQuadPinMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh_base = std::move(_input);
  if (!mesh_base)
    mesh_base = buildMeshBaseObject();
  mesh_base->set_mesh_dimension(3);

  std::vector<Point> pin_centers;
  QuadSubChannelMesh::generatePinCenters(_nx, _ny, _pitch, 0, pin_centers);

  _elem_id = mesh_base->n_elem();
  for (auto & ctr : pin_centers)
    generatePin(mesh_base, ctr);

  mesh_base->subdomain_name(_block_id) = name();
  mesh_base->prepare_for_use();

  return mesh_base;
}
