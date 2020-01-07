//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElemUniqueSubdomainsGenerator.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"

registerMooseObject("MooseTestApp", ElemUniqueSubdomainsGenerator);

InputParameters
ElemUniqueSubdomainsGenerator::validParams()
{
  auto p = MeshGenerator::validParams();
  p.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  p.addParam<subdomain_id_type>("mod",
                                std::numeric_limits<subdomain_id_type>::max(),
                                "elems are assigned their ids as block ids mod this");
  return p;
}

ElemUniqueSubdomainsGenerator::ElemUniqueSubdomainsGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
ElemUniqueSubdomainsGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  auto mod = getParam<subdomain_id_type>("mod");

  for (auto elem : mesh->active_local_element_ptr_range())
    elem->subdomain_id() = elem->id() % mod;

  return mesh;
}
