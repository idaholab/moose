//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsFromNodeSetsGenerator.h"

#include "CastUniquePointer.h"

registerMooseObject("MooseApp", SideSetsFromNodeSetsGenerator);

InputParameters
SideSetsFromNodeSetsGenerator::validParams()
{
  InputParameters params = SideSetsGeneratorBase::validParams();

  params.suppressParameter<Point>("normal");
  params.suppressParameter<Real>("normal_tol");
  params.suppressParameter<bool>("fixed_normal");
  params.suppressParameter<bool>("replace");
  params.suppressParameter<bool>("include_only_external_sides");
  params.suppressParameter<std::vector<BoundaryName>>("included_boundaries");
  params.suppressParameter<std::vector<SubdomainName>>("included_subdomains");
  params.suppressParameter<std::vector<SubdomainName>>("included_neighbors");

  params.addClassDescription("Mesh generator which constructs side sets from node sets");
  return params;
}

SideSetsFromNodeSetsGenerator::SideSetsFromNodeSetsGenerator(const InputParameters & parameters)
  : SideSetsGeneratorBase(parameters)
{
}

std::unique_ptr<MeshBase>
SideSetsFromNodeSetsGenerator::generate()
{
  _input->get_boundary_info().build_side_list_from_node_list();

  return dynamic_pointer_cast<MeshBase>(_input);
}
