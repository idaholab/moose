//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestRayLots.h"

registerMooseObject("RayTracingTestApp", TestRayLots);

InputParameters
TestRayLots::validParams()
{
  auto params = LotsOfRaysRayStudy::validParams();

  params.addParam<bool>("get_info", false, "Tests Ray::getInfo()");
  params.addParam<bool>("equality", false, "Tests the equality and inequality operators for Ray");

  return params;
}

TestRayLots::TestRayLots(const InputParameters & parameters) : LotsOfRaysRayStudy(parameters)
{
  registerRayData("data");
  registerRayAuxData("aux_data");
}

void
TestRayLots::postExecuteStudy()
{
  if (getParam<bool>("get_info"))
    for (const auto & ray : rayBank())
      libMesh::err << ray->getInfo() << std::endl;

  if (getParam<bool>("equality"))
  {
    std::size_t same_passes = 0, duplicate_passes = 0;

    std::vector<UserObject *> uos;
    _fe_problem.theWarehouse().query().condition<AttribSystem>("UserObject").queryInto(uos);
    RayTracingStudy * other_study = nullptr;
    for (auto & uo : uos)
      if (auto study = dynamic_cast<RayTracingStudy *>(uo))
        if (study != this)
        {
          other_study = study;
          break;
        }
    mooseAssert(other_study, "Failed to find other study");

    for (const auto & ray : rayBank())
    {
      std::vector<std::shared_ptr<Ray>> duplicate_rays;

      auto duplicate_ray = [&ray, &duplicate_rays](RayTracingStudy * study = nullptr)
      {
        auto duplicate = std::make_shared<Ray>(study != nullptr ? study : &ray->_study,
                                               ray->id(),
                                               ray->data().size(),
                                               ray->auxData().size(),
                                               false,
                                               Ray::ConstructRayKey());
        duplicate->_current_point = ray->_current_point;
        duplicate->_direction = ray->_direction;
        duplicate->_current_elem = ray->_current_elem;
        duplicate->_current_incoming_side = ray->_current_incoming_side;
        duplicate->_end_set = ray->_end_set;
        duplicate->_processor_crossings = ray->_processor_crossings;
        duplicate->_intersections = ray->_intersections;
        duplicate->_trajectory_changes = ray->_trajectory_changes;
        duplicate->_trajectory_changed = ray->_trajectory_changed;
        duplicate->_distance = ray->_distance;
        duplicate->_max_distance = ray->_max_distance;
        duplicate->_should_continue = ray->_should_continue;
        duplicate->_data = ray->_data;
        duplicate->_aux_data = ray->_aux_data;

        duplicate_rays.push_back(duplicate);

        return duplicate;
      };

      if (*ray != *ray || !(*ray == *ray))
        mooseError("Same ray equality failed");
      else
        ++same_passes;

      ++duplicate_ray()->_id;
      duplicate_ray()->_current_point = RayTracingCommon::invalid_point;
      duplicate_ray()->_direction = RayTracingCommon::invalid_point;
      bool found_other_elem = false;
      for (const auto & elem : meshBase().active_local_element_ptr_range())
        if (elem != ray->currentElem())
        {
          found_other_elem = true;
          duplicate_ray()->_current_elem = elem;
          break;
        }
      if (!found_other_elem) // might not have another elem with high proc counts
        ++duplicate_passes;
      if (ray->invalidCurrentIncomingSide())
        duplicate_ray()->_current_incoming_side = 0;
      else
        ++duplicate_ray()->_current_incoming_side;
      duplicate_ray()->_end_set = !ray->_end_set;
      ++duplicate_ray()->_processor_crossings;
      ++duplicate_ray()->_intersections;
      ++duplicate_ray()->_trajectory_changes;
      duplicate_ray()->_trajectory_changed = !ray->_trajectory_changed;
      duplicate_ray()->_distance += 1;
      duplicate_ray()->_max_distance = 0;
      duplicate_ray()->_should_continue = !ray->_should_continue;
      duplicate_ray()->_data.resize(ray->_data.size() + 1);
      duplicate_ray()->_data[0] += 1;
      duplicate_ray()->_aux_data.resize(ray->_aux_data.size() + 1);
      duplicate_ray()->_aux_data[0] += 1;
      duplicate_ray(other_study);

      for (const auto & duplicate : duplicate_rays)
      {
        const auto equal = *duplicate == *ray;
        if (equal)
          mooseError("Unequal rays are equal");

        const auto inequal = *duplicate != *ray;
        if (!inequal)
          mooseError("Unequal rays are not unequal");

        if (equal != !inequal)
          mooseError("Ray Equality != !inequality");

        ++duplicate_passes;
      }
    }

    comm().sum(same_passes);
    comm().sum(duplicate_passes);
    if (processor_id() == 0)
      _console << "Ray equality test: same_passes = " << same_passes
               << ", duplicate_passes = " << duplicate_passes << std::endl;
  }
}
