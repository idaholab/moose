//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayTracingOverlayMeshTest.h"
#include "IntersectionElemsHelper.h"

registerMooseObject("RayTracingApp", RayTracingOverlayMeshTest);

InputParameters
RayTracingOverlayMeshTest::validParams()
{
  auto params = GeneralUserObject::validParams();

  params.addRequiredParam<MeshGeneratorName>("overlay_mesh", "The base mesh we want to overlay");

  return params;
}

RayTracingOverlayMeshTest::RayTracingOverlayMeshTest(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _main_mesh_name("main_mesh"),
    _overlay_mesh_name(getParam<MeshGeneratorName>("overlay_mesh")),
    _main_mesh(_fe_problem.mesh())
{
  _overlay_mesh = _app.getMeshGeneratorSystem().getSavedMesh(_overlay_mesh_name);
}

void
RayTracingOverlayMeshTest::initialize()
{
  auto _intersection_elems_helper = IntersectionElemsHelper(_main_mesh, *_overlay_mesh);
  _intersection_elems_helper.ElemIntersectionMap();
  auto overlay_elems_to_main = _intersection_elems_helper.getMainElemstoOverlay();
  auto main_elems_to_overlay = _intersection_elems_helper.getOverlayElemstoMain();
  _overlay_elems_to_main = buildElemMapping(overlay_elems_to_main);
  _main_elems_to_overlay = buildElemMapping(main_elems_to_overlay);
}

std::map<dof_id_type, std::vector<dof_id_type>>
RayTracingOverlayMeshTest::buildElemMapping(
    const std::map<const Elem *, std::vector<const Elem *>> & map)
{
  std::map<dof_id_type, std::vector<dof_id_type>> result;
  for (auto main_elem = map.begin(); main_elem != map.end(); main_elem++)
  {
    std::vector<dof_id_type> intersect_elems;
    for (auto & elem : main_elem->second)
      intersect_elems.push_back(elem->id());
    result.emplace((main_elem->first)->id(), intersect_elems);
    intersect_elems.clear();
  }

  return result;
}
