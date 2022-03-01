//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainIDGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", SubdomainIDGenerator);

InputParameters
SubdomainIDGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<SubdomainID>("subdomain_id", "New subdomain IDs of all elements");
  params.addClassDescription("Sets all the elements of the input mesh to a unique subdomain ID.");

  return params;
}

SubdomainIDGenerator::SubdomainIDGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _subdomain_id(getParam<SubdomainID>("subdomain_id"))
{
}

std::unique_ptr<MeshBase>
SubdomainIDGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  for (auto & elem : mesh->element_ptr_range())
    elem->subdomain_id() = _subdomain_id;

  return dynamic_pointer_cast<MeshBase>(mesh);
}
