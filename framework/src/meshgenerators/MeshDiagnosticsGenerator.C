//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshDiagnosticsGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", MeshDiagnosticsGenerator);

InputParameters
MeshDiagnosticsGenerator::validParams()
{

  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to          ");
  params.addClassDescription("");

  params.addParam<bool>(
      "examine_element_volumes", true, "whether to examine volume of the elements");
  params.addParam<Real>("minimum_element_volumes", 1e-16, "minimum size for element volume");
  params.addParam<Real>("maximum_element_volumes", 1e16, "Maximum size for element volume");

  params.addParam<bool>("examine_element_types",
                        true,
                        "whether to look for multiple element types in the same sub-domain");

  return params;
}

MeshDiagnosticsGenerator::MeshDiagnosticsGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _check_element_volumes(getParam<bool>("examine_element_volumes")),
    _min_volume(getParam<Real>("minimum_element_volumes")),
    _max_volume(getParam<Real>("maximum_element_volumes")),
    _check_element_types(getParam<bool>("examine_element_types")),
    _check_element_intersect(getParam<bool>("examine_element_intersect"))
{
}

std::unique_ptr<MeshBase>
MeshDiagnosticsGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  if (_check_element_volumes)
  {
    for (auto & elem : mesh->active_element_ptr_range())
    {
      if (elem->volume() <= _min_volume)
      {
        _num_tiny_elems++;
      }
      if (elem->volume() >= _max_volume)
      {
        _num_big_elems++;
      }
    }
    _console << "Number of elements below volume size : " << _num_tiny_elems << std::endl;
    _console << "Number of elements above volume size : " << _num_big_elems << std::endl;
  }

  if (_check_element_types)
  {
    std::set<subdomain_id_type> ids;
    mesh->subdomain_ids(ids);
    // loop on sub-domain
    for (auto & id : ids)
    {
      std::set<ElemType> types;
      // for loop on elements within this sub-domain
      for (auto & elem : mesh->active_subdomain_elements_ptr_range(id))
      {
        types.insert(elem->type());
      }
      std::string elem_type_names;
      for (auto & elem_type : types)
        elem_type_names += " " + Moose::stringify(elem_type);

      _console << "Element type in subdomain " + mesh->subdomain_name(id) + " (" +
                      std::to_string(id) + ") :" + elem_type_names
               << std::endl;
      if (types.size() > 1)
        mooseWarning("Two different element types in subdomain " + std::to_string(id));
    }
  }
  if (_check_element_intersect)
  {
    for (auto & elem : mesh->active_element_ptr_range())
    {
      if (elem->volume() <= _min_volume)
      {
        _num_tiny_elems++;
      }
      if (elem->volume() >= _max_volume)
      {
        _num_big_elems++;
      }
    }
    _console << "Number of elements below volume size : " << _num_tiny_elems << std::endl;
    _console << "Number of elements above volume size : " << _num_big_elems << std::endl;
  }
  return dynamic_pointer_cast<MeshBase>(mesh);
}
