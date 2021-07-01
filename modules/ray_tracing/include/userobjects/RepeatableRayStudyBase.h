//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RayTracingStudy.h"

// Local includes
#include "ClaimRays.h"

/**
 * A RayTracingStudy that generates and traces Rays repeatedly that a user defines only once
 *
 * To use, override defineRays()
 */
class RepeatableRayStudyBase : public RayTracingStudy
{
public:
  RepeatableRayStudyBase(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual void meshChanged() override;
  virtual void generateRays() override;

  /**
   * Entry point for the user to create Rays
   *
   * Users _must_ override this function to fill _rays.
   *
   * See the comments in RepeatableRayStudyBase::generateRays()
   * for more information.
   */
  virtual void defineRays() = 0;

  /// Vector of Rays that the user will fill into in defineRays() (restartable)
  std::vector<std::shared_ptr<Ray>> & _rays;

  /// Whether or not the Rays filled into _rays are replicated across all processors
  const bool _define_rays_replicated;
  /// Whether or not Rays need to be claimed after defineRays()
  const bool _claim_after_define_rays;

  /// Whether or not we should call defineRays() on the next generateRays()
  /// Can be set to true in derived classes if they wish to redefine
  /// the rays after they have been defined once. (restartable)
  bool & _should_define_rays;

private:
  void claimRaysInternal();
  void defineRaysInternal();

  /**
   * Verifies that the Rays in _rays are replicated across processors.
   *
   * In optimized mode, this compares the sizes across all processors.
   * In non-optimized modes, this communicates the Rays from all processors
   * to the root and compares the Rays.
   */
  void verifyReplicatedRays();

  /// Storage for all of the Rays this processor is responsible for (restartable)
  std::vector<std::shared_ptr<Ray>> & _local_rays;

  /// The object used to claim Rays
  ClaimRays _claim_rays;

  /// Whether or not we should call claimRays() on the next generateRays() (restartable)
  bool & _should_claim_rays;
};
