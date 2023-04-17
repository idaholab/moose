//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntersectionElemsReporter.h"

#include "RayTracingOverlayMeshTest.h"

registerMooseObject("RayTracingTestApp", IntersectionElemsReporter);

InputParameters
IntersectionElemsReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addRequiredParam<UserObjectName>(
      "overlay_uo_name", "The name of the RayTracingOverlayMeshTest user object to pull data from");
  params.addClassDescription("Reports the elems mapping between ovelay mesh and main mesh.");
  return params;
}

IntersectionElemsReporter::IntersectionElemsReporter(const InputParameters & parameters)
  : GeneralReporter(parameters)
{
  declareValueByName<const RayTracingOverlayMeshTest *>(
      "overlay_mesh_test",
      REPORTER_MODE_ROOT,
      &getUserObject<RayTracingOverlayMeshTest>("overlay_uo_name"));
}

void
to_json(nlohmann::json & json, const RayTracingOverlayMeshTest * const & overlay_mesh_test)
{

  const auto build_data_structure = [&json](const auto & map, auto & type)
  {
    nlohmann::json entry;
    if (map.size() == 0)
      // Create an empty array
      json = nlohmann::json::array();
    else
      // Output data to json
      for (auto main_elem = map.begin(); main_elem != map.end(); main_elem++)
      {
        std::string main_id = type + "_id";
        entry[main_id] = main_elem->first;
        auto array = main_elem->second;
        entry["mapping_id"] = array;
        json.push_back(entry);
      }
  };
  // Build data structure for store
  auto & main_entry = overlay_mesh_test->getOverlayElemstoMain();
  auto & overlay_entry = overlay_mesh_test->getMainElemstoOverlay();

  std::string type = overlay_mesh_test->getMainMeshName();
  build_data_structure(main_entry, type);
  type = overlay_mesh_test->getOverlayMeshName();
  build_data_structure(overlay_entry, type);
}
