//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayTracingStudyTest.h"

#include "libmesh/parallel_sync.h"

registerMooseObject("RayTracingTestApp", RayTracingStudyTest);
registerMooseObject("RayTracingTestApp", RayTracingStudyNoBankingTest);
registerMooseObject("RayTracingTestApp", RayTracingStudyWithRegistrationTest);

InputParameters
RayTracingStudyTest::validParams()
{
  auto params = RayTracingStudy::validParams();

  params.addParam<bool>("add_duplicate_ray", false, "Add a duplicate Ray to the buffer");
  params.addParam<bool>(
      "add_local_non_unique_id_ray", false, "Add two Rays on each processor with the same ID");
  params.addParam<bool>(
      "add_global_non_unique_id_ray", false, "Add a Ray on each processor with the same ID");

  params.addParam<bool>("ray_error_if_tracing", false, "Tests Ray::errorIfTracing()");
  params.addParam<bool>(
      "ray_reset_counters", false, "Tests resetting a Ray's counters after it has began tracing");

  params.addParam<bool>(
      "register_ray_data_late", false, "Tests registering data late in registerRayDataInternal()");
  params.addParam<bool>(
      "register_ray_data_same_name",
      false,
      "Tests registering ray data with the same name in registerRayDataInternal()");
  params.addParam<bool>("ray_data_index_bad_name",
                        false,
                        "Tests requesting ray data index for a name that does not exist in "
                        "getRayDataIndexInternal()");
  params.addParam<bool>("ray_data_name_bad_index",
                        false,
                        "Tests requesting ray data name for an index that does not exist in "
                        "getRayDataNameInternal()");

  params.addParam<bool>(
      "get_ray_kernels_early", false, "Tests getting ray kernels early with getRayKernels()");
  params.addParam<bool>("get_ray_bcs_early", false, "Tests getting ray bcs early with getRayBCs()");

  params.addParam<bool>(
      "get_ray_bank_generating", false, "Tests getting the ray bank when it is not available");

  params.addParam<bool>("register_ray_no_registration",
                        false,
                        "Tests registering a Ray with Ray registration disabled");
  params.addParam<bool>("registered_ray_id_no_registration",
                        false,
                        "Tests getting a registered ray id with registration disabled");
  params.addParam<bool>("registered_ray_name_no_registration",
                        false,
                        "Tests getting a registered ray name with registration disabled");

  params.addParam<bool>(
      "reserve_bad", false, "Tests reserving space in the ray buffer outside of registration");

  params.addParam<bool>("subdomain_hmax_missing",
                        false,
                        "Tests requesting the subdomain hmax for an invalid subdomain");

  params.addParam<bool>(
      "get_elem_normals_unimplemented", false, "Tests getElemNormals not being overridden");

  params.addParam<bool>("elem_extrema_build_edge",
                        false,
                        "Tests building an edge from extrema when the vertices are incorrect");

  params.addParam<bool>("ray_data_index_other_exists",
                        false,
                        "Tests getting a Ray data index when aux data with the same name exists");
  params.addParam<bool>("ray_data_aux_index_other_exists",
                        false,
                        "Tests getting a Ray aux data index when data with the same name exists");

  params.set<bool>("_use_ray_registration") = false;

  return params;
}

RayTracingStudyTest::RayTracingStudyTest(const InputParameters & parameters)
  : RayTracingStudy(parameters)
{
  if (getParam<bool>("register_ray_data_same_name"))
  {
    registerRayData("foo");
    registerRayAuxData("foo");
  }
  if (getParam<bool>("ray_data_index_bad_name"))
    getRayDataIndex("foo");
  if (getParam<bool>("ray_data_name_bad_index"))
    getRayDataName(123);
  if (getParam<bool>("get_ray_kernels_early"))
  {
    std::vector<RayKernelBase *> rks;
    getRayKernels(rks, 0, 0);
  }
  if (getParam<bool>("get_ray_bcs_early"))
  {
    std::vector<RayBoundaryConditionBase *> rbcs;
    getRayBCs(rbcs, 0, 0);
  }
  if (getParam<bool>("registered_ray_id_no_registration"))
    registeredRayID("foo");
  if (getParam<bool>("registered_ray_name_no_registration"))
    registeredRayName(0);
  if (getParam<bool>("reserve_bad"))
    reserveRayBuffer(0);
  if (getParam<bool>("get_elem_normals_unimplemented"))
    getElemNormals(meshBase().elem_ptr(0), 0);
  if (getParam<bool>("elem_extrema_build_edge"))
  {
    ElemExtrema extrema(1337, 1338);
    extrema.buildEdge(meshBase().elem_ptr(0));
  }
  if (getParam<bool>("ray_data_index_other_exists"))
  {
    registerRayAuxData("foo");
    getRayDataIndex("foo");
  }
  if (getParam<bool>("ray_data_aux_index_other_exists"))
  {
    registerRayData("foo");
    getRayAuxDataIndex("foo");
  }
}

void
RayTracingStudyTest::generateRays()
{
  if (_mesh.getMesh().n_local_elem() != 0)
  {
    const Elem * elem = *_mesh.getActiveLocalElementRange()->begin();

    auto ray = acquireRay();

    ray->setStart(elem->vertex_average(), elem);
    ray->setStartingEndPoint(elem->point(0));

    if (getParam<bool>("add_duplicate_ray"))
    {
      std::shared_ptr<Ray> duplicate_ray = ray;
      moveRayToBuffer(ray);
      moveRayToBuffer(duplicate_ray);
    }

    if (getParam<bool>("add_local_non_unique_id_ray"))
    {
      std::shared_ptr<Ray> other_ray = acquireCopiedRay(*ray);
      moveRayToBuffer(ray);
      moveRayToBuffer(other_ray);
    }

    if (getParam<bool>("add_global_non_unique_id_ray"))
    {
      mooseAssert(n_processors() > 1, "Needs multiple ranks");

      std::map<processor_id_type, std::vector<std::shared_ptr<Ray>>> send_map;
      if (_pid == 0)
      {
        for (processor_id_type pid = 1; pid < n_processors(); ++pid)
          send_map[pid].push_back(ray);
        moveRayToBuffer(ray);
      }

      auto add_ray_functor = [&](processor_id_type, const std::vector<std::shared_ptr<Ray>> & rays)
      {
        std::shared_ptr<Ray> ray = rays[0];
        ray->clearStartingInfo();
        ray->setStart(elem->vertex_average());
        ray->setStartingEndPoint(elem->point(0));
        moveRayToBuffer(ray);
      };

      Parallel::push_parallel_packed_range(comm(), send_map, parallelStudy(), add_ray_functor);
    }

    if (getParam<bool>("ray_error_if_tracing") || getParam<bool>("ray_reset_counters"))
      moveRayToBuffer(ray);
  }

  if (getParam<bool>("register_ray_data_late"))
    registerRayData("foo");

  if (getParam<bool>("get_ray_bank_generating"))
    rayBank();

  if (getParam<bool>("register_ray_no_registration"))
    acquireRegisteredRay("foo");

  if (getParam<bool>("subdomain_hmax_missing"))
    subdomainHmax(1337);
}

void
RayTracingStudyTest::postExecuteStudy()
{
  if (getParam<bool>("ray_error_if_tracing"))
    for (auto & ray : rayBank())
      ray->clearStartingInfo();

  if (getParam<bool>("ray_reset_counters"))
    for (auto & ray : rayBank())
      ray->resetCounters();
}

RayTracingStudyNoBankingTest::RayTracingStudyNoBankingTest(const InputParameters & params)
  : RayTracingStudy(params)
{
  rayBank();
}

InputParameters
RayTracingStudyNoBankingTest::validParams()
{
  auto params = RayTracingStudy::validParams();
  params.set<bool>("_bank_rays_on_completion") = false;
  return params;
}

InputParameters
RayTracingStudyWithRegistrationTest::validParams()
{
  auto params = RayTracingStudy::validParams();

  params.addParam<bool>(
      "registered_ray_id_missing", false, "Tests registeredRayID() with an invalid id");
  params.addParam<bool>(
      "registered_ray_name_missing", false, "Tests registeredRayName() with an invalid name");

  return params;
}

RayTracingStudyWithRegistrationTest::RayTracingStudyWithRegistrationTest(
    const InputParameters & params)
  : RayTracingStudy(params)
{
  if (getParam<bool>("registered_ray_id_missing"))
    registeredRayID("foo");
  if (getParam<bool>("registered_ray_name_missing"))
    registeredRayName(0);
}
