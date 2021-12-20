//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoreMeshGenerator.h"

#include "MooseApp.h"
#include "Factory.h"
#include "libmesh/elem.h"

registerMooseObject("ReactorApp", CoreMeshGenerator);

InputParameters
CoreMeshGenerator::validParams()
{
  auto params = MeshGenerator::validParams();

  params.addRequiredParam<std::vector<MeshGeneratorName>>(
      "inputs", "The PinMeshGenerators that form the components of the assembly.");

  params.addRequiredParam<MeshGeneratorName>(
      "empty_position_name",
      "The place holder name in \"inputs\" that indicates an empty position.");

  params.addRequiredParam<std::vector<std::vector<unsigned int>>>(
      "pattern", "A double-indexed array starting with the upper-left corner");

  params.addParam<bool>("extrude",
                        false,
                        "Determines if this is the final step in the geometry construction"
                        " and extrudes the 2D geometry to 3D. If this is true then this mesh "
                        "cannot be used in further mesh building");

  params.addClassDescription("This CoreMeshGenerator object is designed to generate a core-like "
                             "structure, with IDs, from a reactor geometry. "
                             "The core-like structure consists of a pattern of assembly-like "
                             "structures generated with AssemblyMeshGenerator "
                             "and is permitted to have \"empty\" locations. The size and spacing "
                             "of the assembly-like structures is defined, and "
                             "enforced by declaration in the ReactorMeshParams.");

  return params;
}

CoreMeshGenerator::CoreMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _inputs(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _empty_key(getParam<MeshGeneratorName>("empty_position_name")),
    _pattern(getParam<std::vector<std::vector<unsigned int>>>("pattern")),
    _extrude(getParam<bool>("extrude"))
{
  /* Below, we get the ReactorMeshParams info from one of the pin inputs.
     The reactor params gives the axial info if we are to extrude as well as the assembly pitch
     that the pin pattern is to be checked against.
     We then create the PatternedMesh subgenerators and pass the parameter info needed for the
     inputs. Finally, if the the mesh is to be extruded we create the FancyExtruderGenerator
     subgenerator and the PlaneIDMeshGenerator subgenerator to handle this.
  */
  MeshGeneratorName _reactor_params =
      MeshGeneratorName(getMeshProperty<std::string>("reactor_params_name", _inputs[0]));

  // std::unique_ptr<MeshBase> & reactor_params_mesh = getMeshByName(_reactor_params);
  // Ensure that the user has supplied the correct info for conformal mesh generation
  if (!hasMeshProperty("mesh_dimensions", _reactor_params) ||
      !hasMeshProperty("mesh_geometry", _reactor_params))
  {
    mooseError("The reactor_params input must be a ReactorMeshParams type MeshGenerator\n");
  }
  else
  {
    _geom_type = getMeshProperty<std::string>("mesh_geometry", _reactor_params);
  }
  if (_extrude && getMeshProperty<unsigned int>("mesh_dimensions", _reactor_params) != 3)
  {
    mooseError("This is a 2 dimensional mesh, you cannot extrude it. Check you ReactorMeshParams "
               "inputs\n");
  }

  size_t empty_pattern_loc = 0;
  bool make_empty = false;
  for (auto assembly : _inputs)
  {
    if (assembly != _empty_key)
    {
      ++empty_pattern_loc;
      if (getMeshProperty<bool>("extruded", assembly))
      {
        mooseError("Assemblies that have already been extruded cannot be used in CoreMeshGenerator "
                   "definition.\n");
      }
    }
    else
    {
      make_empty = true;
      for (size_t i = 0; i < _pattern.size(); ++i)
      {
        for (size_t j = 0; j < _pattern[i].size(); ++j)
        {
          if (_pattern[i][j] == empty_pattern_loc)
          {
            empty_pos = true;
          }
        }
      }
    }
  }
  //***Add checks for pins to be the same size and pin sizes to add to Assembly pitch

  if (_geom_type == "Square")
  {
    // create a dummy assembly that is 1 assembly sized element that will get deleted later
    if (make_empty)
    {
      auto params = _app.getFactory().getValidParams("CartesianMeshGenerator");

      params.set<MooseEnum>("dim") = 2;
      std::vector<Real> pitch{getMeshProperty<Real>("assembly_pitch", _reactor_params)};
      params.set<std::vector<Real>>("dx") = pitch;
      params.set<std::vector<Real>>("dy") = pitch;
      params.set<std::vector<unsigned int>>("subdomain_id") =
          std::vector<unsigned int>{UINT16_MAX - 1};

      addMeshSubgenerator("CartesianMeshGenerator", std::string(_empty_key), params);
    }
    {
      auto params = _app.getFactory().getValidParams("CartesianIDPatternedMeshGenerator");

      params.set<std::string>("id_name") = "assembly_id";
      params.set<MooseEnum>("assign_type") =
          "cell"; // give elems IDs relative to position in assembly
      params.set<std::vector<MeshGeneratorName>>("inputs") = _inputs;
      params.set<std::vector<std::vector<unsigned int>>>("pattern") = _pattern;
      if (make_empty)
      {
        params.set<std::vector<MeshGeneratorName>>("exclude_id") =
            std::vector<MeshGeneratorName>{_empty_key};
      }
      params.set<BoundaryName>("top_boundary") = "10001";
      params.set<BoundaryName>("left_boundary") = "10002";
      params.set<BoundaryName>("bottom_boundary") = "10003";
      params.set<BoundaryName>("right_boundary") = "10004";

      addMeshSubgenerator("CartesianIDPatternedMeshGenerator", name() + "_pattern", params);
    }
  }
  else
  {
    // Hex geometry
    // create a dummy assembly that is a renamed version of one of the inputs
    if (make_empty)
    {
      {
        auto params = _app.getFactory().getValidParams(
            "HexagonConcentricCircleAdaptiveBoundaryMeshGenerator");

        params.set<Real>("hexagon_size") =
            getMeshProperty<Real>("assembly_pitch", _reactor_params) / 2.0;
        params.set<std::vector<unsigned int>>("num_sectors_per_side") =
            std::vector<unsigned int>(6, 2);
        params.set<std::vector<unsigned int>>("sides_to_adapt") = std::vector<unsigned int>{0};
        params.set<std::vector<MeshGeneratorName>>("inputs") =
            std::vector<MeshGeneratorName>{_inputs[0]};
        params.set<std::vector<subdomain_id_type>>("background_block_ids") =
            std::vector<subdomain_id_type>{UINT16_MAX - 1};

        addMeshSubgenerator("HexagonConcentricCircleAdaptiveBoundaryMeshGenerator",
                            std::string(_empty_key),
                            params);
      }
    }
    {
      auto params = _app.getFactory().getValidParams("HexIDPatternedMeshGenerator");

      params.set<std::string>("id_name") = "assembly_id";
      params.set<MooseEnum>("assign_type") =
          "cell"; // give elems IDs relative to position in assembly
      params.set<std::vector<MeshGeneratorName>>("inputs") = _inputs;
      params.set<std::vector<std::vector<unsigned int>>>("pattern") = _pattern;
      params.set<MooseEnum>("pattern_boundary") = "none";
      params.set<bool>("generate_core_metadata") = true;
      if (make_empty)
      {
        params.set<std::vector<MeshGeneratorName>>("exclude_id") =
            std::vector<MeshGeneratorName>{_empty_key};
      }

      params.set<boundary_id_type>("external_boundary_id") = 200;
      params.set<std::string>("external_boundary_name") = "outer_core";

      _build_mesh =
          &addMeshSubgenerator("HexIDPatternedMeshGenerator", name() + "_pattern", params);
    }
  }
  if (empty_pos)
  {
    auto params = _app.getFactory().getValidParams("BlockDeletionGenerator");

    params.set<SubdomainID>("block_id") = UINT16_MAX - 1;
    params.set<MeshGeneratorName>("input") = name() + "_pattern";

    _build_mesh = &addMeshSubgenerator("BlockDeletionGenerator", name() + "_deleted", params);
  }

  // transferring reactor parameters to core mesh
  declareMeshProperty("reactor_params_name", std::string(_reactor_params));

  for (auto assembly : _inputs)
  {
    if (assembly != _empty_key)
    {
      std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>> pin_map =
          getMeshProperty<std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>>(
              "pin_id_map", assembly);
      for (auto pin = pin_map.begin(); pin != pin_map.end(); ++pin)
      {
        if (_id_map.find(pin->first) == _id_map.end())
        {
          _id_map.insert(std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
              pin->first, pin->second));
        }
        else if (pin->second != _id_map.find(pin->first)->second)
        {
          mooseError("Multiple region definitions for the same pin type. Check pin_type ids.\n");
        }
      }

      if (_geom_type == "Hex")
      {
        subdomain_id_type assembly_type =
            getMeshProperty<subdomain_id_type>("assembly_type_id", assembly);
        if (_background_id_map.find(assembly_type) == _background_id_map.end())
        {
          std::vector<subdomain_id_type> background_regions =
              getMeshProperty<std::vector<subdomain_id_type>>("background_region_ids", assembly);
          std::vector<std::vector<subdomain_id_type>> duct_regions =
              getMeshProperty<std::vector<std::vector<subdomain_id_type>>>("duct_region_ids",
                                                                           assembly);
          _background_id_map.insert(std::pair<subdomain_id_type, std::vector<subdomain_id_type>>(
              assembly_type, background_regions));
          _duct_id_map.insert(
              std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
                  assembly_type, duct_regions));
        }
      }
    }
  }
  declareMeshProperty("pin_id_map", _id_map);

  declareMeshProperty("assembly_pitch", getMeshProperty<Real>("assembly_pitch", _reactor_params));

  if (_extrude && getMeshProperty<int>("mesh_dimensions", _reactor_params) == 3)
  {
    std::vector<Real> axial_boundaries =
        getMeshProperty<std::vector<Real>>("axial_boundaries", _reactor_params);
    {
      declareMeshProperty("extruded", true);
      auto params = _app.getFactory().getValidParams("FancyExtruderGenerator");

      params.set<MeshGeneratorName>("input") =
          empty_pos ? name() + "_deleted" : name() + "_pattern";
      params.set<Point>("direction") = Point(0, 0, 1);
      params.set<std::vector<unsigned int>>("num_layers") =
          getMeshProperty<std::vector<unsigned int>>("axial_mesh_intervals", _reactor_params);
      params.set<std::vector<Real>>("heights") = axial_boundaries;
      params.set<boundary_id_type>("bottom_boundary") = 202;
      params.set<boundary_id_type>("top_boundary") = 201;

      addMeshSubgenerator("FancyExtruderGenerator", name() + "_extruded", params);
    }

    {
      auto params = _app.getFactory().getValidParams("RenameBoundaryGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_extruded";
      params.set<std::vector<BoundaryName>>("old_boundary") = {
          "201", "202"}; // hard coded boundary IDs in patterned mesh generator
      params.set<std::vector<BoundaryName>>("new_boundary") = {"top", "bottom"};

      addMeshSubgenerator("RenameBoundaryGenerator", name() + "_change_plane_name", params);
    }

    {
      auto params = _app.getFactory().getValidParams("PlaneIDMeshGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_change_plane_name";

      std::vector<Real> plane_heights{0};
      for (Real z : axial_boundaries)
      {
        plane_heights.push_back(z + plane_heights.back());
      }
      params.set<std::vector<Real>>("plane_coordinates") = plane_heights;

      std::string plane_id_name = "plane_id";
      params.set<std::string>("id_name") = "plane_id";

      _build_mesh = &addMeshSubgenerator("PlaneIDMeshGenerator", name() + "_extrudedIDs", params);
    }
  }
  else
    declareMeshProperty("extruded", false);
}

std::unique_ptr<MeshBase>
CoreMeshGenerator::generate()
{
  // This generate() method will be called once the subgenerators that we depend on are
  // called. This is where we set the region ids and element integers.

  if (_extrude)
  {
    // swap the region ids on the subdomain ids to the correct ones
    // for their axial layer
    mesh_name = name() + "_extrudedIDs";
    std::string plane_id_name = "plane_id";
    // std::string region_id_name = "region_id";
    std::string pin_type_id_name = "pin_type_id";
    std::string assembly_type_id_name = "assembly_type_id";

    unsigned int ptid_int = (*_build_mesh)->get_elem_integer_index(pin_type_id_name);
    unsigned int atid_int = (*_build_mesh)->get_elem_integer_index(assembly_type_id_name);
    unsigned int pid_int = (*_build_mesh)->get_elem_integer_index(plane_id_name);
    // unsigned int rid_int = (*_build_mesh)->get_elem_integer_index(region_id_name);

    for (auto & elem : (*_build_mesh)->active_element_ptr_range())
    {
      dof_id_type z_id = elem->get_extra_integer(pid_int);
      dof_id_type pt_id = elem->get_extra_integer(ptid_int);
      subdomain_id_type r_id = elem->subdomain_id();

      if (_id_map.find(pt_id) == _id_map.end())
      {
        // element is in a duct or background region
        dof_id_type at_id = elem->get_extra_integer(atid_int);
        unsigned int peripheral_index = (UINT16_MAX - 1) - pt_id;
        if (peripheral_index == 0)
        {
          // background region
          elem->subdomain_id() = _background_id_map.find(at_id)->second[z_id];
        }
        else
        {
          // duct region
          elem->subdomain_id() = _duct_id_map.find(at_id)->second[z_id][peripheral_index - 1];
        }
      }
      else
      {
        for (size_t i = 0; i < (_id_map.at(pt_id))[0].size(); ++i)
        {
          // swap subdomain region ids if they are different
          if (r_id == _id_map.at(pt_id)[0][i] &&
              _id_map.at(pt_id)[0][i] != _id_map.at(pt_id)[z_id][i])
          {
            elem->subdomain_id() = _id_map.at(pt_id)[z_id][i];
          }
        }
      }
    }
  }
  else
  {
    mesh_name = empty_pos ? name() + "_deleted" : name() + "_pattern";
  }

  std::string region_id_name = "region_id";
  if (!(*_build_mesh)->has_elem_integer(region_id_name))
  {
    mooseError("Expected mesh inputs to have extra integer ids.\n");
  }

  unsigned int rid = (*_build_mesh)->get_elem_integer_index(region_id_name);
  ;

  std::map<subdomain_id_type, std::string> subdomain_name_map;
  for (auto & elem : (*_build_mesh)->active_element_ptr_range())
  {
    elem->set_extra_integer(rid, elem->subdomain_id());

    if (subdomain_name_map.find(elem->subdomain_id()) == subdomain_name_map.end())
    {
      subdomain_name_map.insert(std::pair<subdomain_id_type, std::string>(
          elem->subdomain_id(), std::to_string(elem->subdomain_id())));
    }
  }

  if (_geom_type == "Square")
  {

    MooseMesh::changeBoundaryId(*(_build_mesh->get()), 10001, 200, false);
    MooseMesh::changeBoundaryId(*(_build_mesh->get()), 10002, 200, false);
    MooseMesh::changeBoundaryId(*(_build_mesh->get()), 10003, 200, false);
    MooseMesh::changeBoundaryId(*(_build_mesh->get()), 10004, 200, false);

    (*_build_mesh)->get_boundary_info().sideset_name(200) = "outer_core";
    (*_build_mesh)->get_boundary_info().nodeset_name(200) = "outer_core";
  }

  (*_build_mesh)->set_subdomain_name_map() = subdomain_name_map;

  return std::move(*_build_mesh);
}