//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MetaDataGenerator.h"

#include "MooseMesh.h"

#include "libmesh/elem.h"

registerMooseObject("MooseTestApp", MetaDataGenerator);

InputParameters
MetaDataGenerator::validParams()
{
  auto p = GeneratedMeshGenerator::validParams();
  p.addParam<unsigned int>("mod",
                           std::numeric_limits<subdomain_id_type>::max(),
                           "Element ID will be reinterpreted as element ids mod this");
  return p;
}

MetaDataGenerator::MetaDataGenerator(const InputParameters & parameters)
  : GeneratedMeshGenerator(parameters), _data(declareMeshProperty<MeshData>("modded_element_id"))
{
}

std::unique_ptr<MeshBase>
MetaDataGenerator::generate()
{
  if (!isFinal())
    mooseError("MetaDataGenerator must be final mesh generator");

  std::unique_ptr<MeshBase> mesh = GeneratedMeshGenerator::generate();

  auto mod = getParam<unsigned int>("mod");

  // constructing the data from the generated mesh
  _data.construct(*mesh, mod);

  return mesh;
}

void
MetaDataGenerator::MeshData::construct(const MeshBase & mesh, unsigned int d)
{
  Moose::out << "Meta data construction for mesh is called!" << std::endl;
  mod = d;
  for (const auto & elem : mesh.element_ptr_range())
  {
    _modded_id.insert(std::pair<const Elem *, unsigned int>(elem, elem->id() % mod));
    Moose::out << elem->id() << " " << elem->centroid() << " " << elem->id() % mod << std::endl;
  }
}

template <>
void
dataStore(std::ostream & stream, MetaDataGenerator::MeshData & d, void * /*context*/)
{
  stream.write((char *)&d.mod, sizeof(d.mod));
}

template <>
void
dataLoad(std::istream & stream, MetaDataGenerator::MeshData & d, void * context)
{
  unsigned int mod;
  stream.read((char *)&mod, sizeof(mod));

  MooseMesh & moose_mesh = *static_cast<MooseMesh *>(context);

  // reconstructing the data from the recovered global mesh
  d.construct(moose_mesh.getMesh(), mod);
}
