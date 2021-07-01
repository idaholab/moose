//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RepeatableRayStudyBase.h"

// libMesh includes
#include "libmesh/parallel_sync.h"

#include "DataIO.h"

InputParameters
RepeatableRayStudyBase::validParams()
{
  auto params = RayTracingStudy::validParams();

  // Whether or not the _rays filled by defineRays() are replicated
  // (the same across all processors)
  params.addPrivateParam<bool>("_define_rays_replicated", true);
  // Whether or not Rays need to be claimed after calling defineRays()
  params.addPrivateParam<bool>("_claim_after_define_rays", true);

  return params;
}

RepeatableRayStudyBase::RepeatableRayStudyBase(const InputParameters & parameters)
  : RayTracingStudy(parameters),
    _rays(declareRestartableDataWithContext<std::vector<std::shared_ptr<Ray>>>("rays", this)),
    _define_rays_replicated(getParam<bool>("_claim_after_define_rays")
                                ? getParam<bool>("_define_rays_replicated")
                                : false),
    _claim_after_define_rays(getParam<bool>("_claim_after_define_rays")),
    _should_define_rays(declareRestartableData<bool>("should_define_rays", true)),
    _local_rays(
        declareRestartableDataWithContext<std::vector<std::shared_ptr<Ray>>>("local_rays", this)),
    _claim_rays(*this,
                _rays,
                _local_rays,
                /* do_exchange = */ !_define_rays_replicated),
    _should_claim_rays(
        declareRestartableData<bool>("claim_after_define_rays", _claim_after_define_rays))
{
  if (!_claim_after_define_rays && getParam<bool>("_define_rays_replicated"))
    mooseWarning("The combination of private parameters:",
                 "\n  '_define_rays_replicated' == true",
                 "\n  '_claim_after_define_rays' == false",
                 "\nis not a valid combination.",
                 "\n\n_define_rays_replicated is being set to false.");
}

void
RepeatableRayStudyBase::generateRays()
{
  // Initially, the user is to define the Rays that they want to trace by overriding
  // defineRays() and filling into _rays within this method. These Rays are not
  // the Rays that will actually be traced, they just serve as a template for
  // Rays that will be put into the tracer to be traced.
  //
  // If the private parameter '_claim_after_define_rays' == true, it is assumed
  // that the Rays that were filled into _rays from the overridden defineRays()
  // do not have their starting element and incoming sides set. The Rays in _rays
  // will be "claimed" later and communicated to the processors that will start them
  // with their starting element set and incoming side set (if any). An example of this
  // is in RepeatableRayStudy.
  //
  // If the private parameter '_claim_after_define_rays' == false, it is assumed
  // that the Rays that were filled into _rays from the overridden defineRays():
  // - Have their starting element and incoming side (if applicable) set and it is
  //   correct
  // - Are on the processor that will start them (the processor that contains
  //   the starting element)
  // At this point, the Rays in _rays will be also inserted into _local_rays
  // because they are on the right processor with starting information set.
  // An example of this is in LotsOfRaysRayStudy.
  if (_should_define_rays)
  {
    _should_define_rays = false;

    defineRaysInternal();
  }

  if (_should_claim_rays)
  {
    _should_claim_rays = false;

    claimRaysInternal();
  }

  // Reserve ahead of time how many Rays we are adding to the buffer
  reserveRayBuffer(_local_rays.size());

  // To make this study "repeatable", we will not trace the Rays that
  // are ready to go in _local_rays. We will instead create new Rays
  // that are duplicates of the ones in _local_rays, and trace those.
  // This ensures that on multiple executions of this study, we always
  // have the information to create the same Rays.
  for (const auto & ray : _local_rays)
  {
    // This acquires a new ray that is copied from a Ray that has already
    // been claimed to begin on this processor with the user-defined trajectory
    std::shared_ptr<Ray> copied_ray = acquireCopiedRay(*ray);

    // This calls std::move() on the ray, which means that copied_ray in this context
    // is no longer valid. We use the move method because copied_ray is a shared_ptr
    // and otherwise we would increase the count as we add it to the buffer and also
    // decrease the count once this goes out of scope.
    moveRayToBuffer(copied_ray);
  }
}

void
RepeatableRayStudyBase::meshChanged()
{
  RayTracingStudy::meshChanged();

  // Need to reclaim after this
  _should_claim_rays = true;

  // Invalidate all of the old starting info because we can't be sure those elements still exist
  for (const auto & ray : _rays)
  {
    ray->invalidateStartingElem();
    ray->invalidateStartingIncomingSide();
  }
  for (const auto & ray : _local_rays)
  {
    ray->invalidateStartingElem();
    ray->invalidateStartingIncomingSide();
  }
}

void
RepeatableRayStudyBase::claimRaysInternal()
{
  TIME_SECTION("claimRaysInternal", 2, "Claiming Rays");

  _claim_rays.claim();
}

void
RepeatableRayStudyBase::defineRaysInternal()
{
  {
    TIME_SECTION("defineRaysInternal", 2, "Defining Rays");

    _rays.clear();
    _local_rays.clear();

    defineRays();
  }

  // Do we actually have Rays
  auto num_rays = _rays.size();
  _communicator.sum(num_rays);
  if (!num_rays)
    mooseError("No Rays were moved to _rays in defineRays()");
  for (const auto & ray : _rays)
    if (!ray)
      mooseError("A nullptr Ray was found in _rays after defineRays().");

  // The Rays in _rays are ready to go as is: they have their starting element
  // set, their incoming set (if any), and are on the processor that owns said
  // starting element. Therefore, we move them right into _local_rays and
  // set that we don't need to claim.
  if (!_claim_after_define_rays)
  {
    _local_rays.reserve(_rays.size());
    for (const std::shared_ptr<Ray> & ray : _rays)
      _local_rays.emplace_back(ray);

    _should_claim_rays = false;
  }
  // Claiming is required after defining. The Rays in _rays should not
  // have their starting elems or incoming sides set - verify that.
  else
  {
    for (const std::shared_ptr<Ray> & ray : _rays)
      if (ray->currentElem() || !ray->invalidCurrentIncomingSide())
        mooseError(
            "A Ray was found in _rays after defineRays() that has a starting element or "
            "incoming side set.\n\n",
            "With the mode in which the private param '_claim_after_define_rays' == true,",
            "\nthe defined Rays at this point should not have their starting elem/side set.\n",
            "\nTheir starting information will be set internally using a claiming process.\n\n",
            ray->getInfo());
  }

  // Sanity checks on if the Rays are actually replicated
  if (_define_rays_replicated && verifyRays())
    verifyReplicatedRays();
}

void
RepeatableRayStudyBase::verifyReplicatedRays()
{
  TIME_SECTION("verifyReplicatedRays", 2, "Verifying Replicated Rays");

  const std::string error_suffix =
      "\n\nThe Rays added in defineRays() must be replicated across all processors\nwith the "
      "private param '_define_rays_replicated' == true.";

  // First, verify that our _rays have unique IDs beacuse we will do mapping based on Ray ID
  verifyUniqueRayIDs(_rays.begin(),
                     _rays.end(),
                     /* global = */ false,
                     "in _rays after calling defineRays()." + error_suffix);

  // Tag for sending rays from rank 0 -> all other ranks
  const auto tag = comm().get_unique_tag();

  // Send a copy of the rays on rank 0 to all other processors for verification
  if (_pid == 0)
  {
    std::vector<Parallel::Request> requests(n_processors() - 1);
    auto request_it = requests.begin();

    for (processor_id_type pid = 0; pid < n_processors(); ++pid)
      if (pid != 0)
        comm().send_packed_range(
            pid, parallelStudy(), _rays.begin(), _rays.end(), *request_it++, tag);

    Parallel::wait(requests);
  }
  // All other processors will receive and verify that their rays match the rays on rank 0
  else
  {
    // Map of RayID -> Ray for comparison from the Rays on rank 0 to the local rays
    std::unordered_map<RayID, const Ray *> ray_map;
    ray_map.reserve(_rays.size());
    for (const auto & ray : _rays)
      ray_map.emplace(ray->id(), ray.get());

    // Receive the duplicated rays from rank 0
    std::vector<std::shared_ptr<Ray>> rank_0_rays;
    rank_0_rays.reserve(_rays.size());
    comm().receive_packed_range(
        0, parallelStudy(), std::back_inserter(rank_0_rays), (std::shared_ptr<Ray> *)nullptr, tag);

    // The sizes better match
    if (rank_0_rays.size() != _rays.size())
      mooseError("The size of _rays on rank ",
                 _pid,
                 " does not match the size of rays on rank 0.",
                 error_suffix);

    // Make sure we have a matching local ray for each ray from rank 0
    for (const auto & ray : rank_0_rays)
    {
      const auto find = ray_map.find(ray->id());
      if (find == ray_map.end())
        mooseError("A Ray was found on rank ",
                   _pid,
                   " with an ID that does not exist on rank 0.",
                   error_suffix,
                   "\n\n",
                   ray->getInfo());

      const Ray * root_ray = find->second;
      if (*root_ray != *ray)
      {
        mooseError("A Ray was found on rank ",
                   _pid,
                   " that does not exist on rank 0.",
                   error_suffix,
                   "\n\nLocal ray:\n\n",
                   ray->getInfo(),
                   "\n\nRank 0 ray:\n\n",
                   root_ray->getInfo());
      }
    }
  }
}
