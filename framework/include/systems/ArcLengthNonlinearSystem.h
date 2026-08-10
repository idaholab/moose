//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NonlinearSystem.h"

class ResidualObject;

/**
 * Nonlinear system that owns the arc-length continuation load tags
 *
 * Arc-length continuation splits the residual into a standard part and a load part that the solver
 * scales by the load parameter lambda. Users designate a load object with the replacing parameters
 * vector_tags and matrix_tags. This system creates the two tags that name that load, holds the
 * ghosted vector the load residual is assembled into, and checks that every residual object added
 * to it is routed to exactly one side of the split.
 */
class ArcLengthNonlinearSystem : public NonlinearSystem
{
public:
  /**
   * @param problem The problem that owns this system
   * @param name The name of this system
   * @param load_vector_tag_name Name of the vector tag that marks load objects
   * @param load_matrix_tag_name Name of the matrix tag that marks follower load objects
   */
  ArcLengthNonlinearSystem(FEProblemBase & problem,
                           const std::string & name,
                           const TagName & load_vector_tag_name,
                           const TagName & load_matrix_tag_name);

  /**
   * @return Vector tag ID of the load residual
   */
  TagID loadVectorTag() const { return _load_vector_tag; }

  /**
   * @return Matrix tag ID of the load Jacobian, the derivative of the load residual with respect
   * to the solution
   */
  TagID loadMatrixTag() const { return _load_matrix_tag; }

  /**
   * @return Whether any residual object has been routed to the load vector tag
   */
  bool hasLoadObjects() const { return _has_load_objects; }

  /**
   * Always errors out. The fused residual and Jacobian assembly path bypasses the arc-length load
   * tag split, which would leave the load residual unscaled by the load parameter lambda.
   */
  virtual void residualAndJacobianTogether() override;

private:
  /**
   * Checks that a load object replaced its default tags instead of appending to them, and records
   * that the system has a load. A load object that also carries one of the default residual vector
   * tags, the nontime tag or the time tag, or that carries the default system matrix tag alongside
   * the load matrix tag, is counted on both sides of the split. No tag is ever added or removed
   * here: TaggingInterface can only insert tags, so such an object is an error the user has to fix
   * in the input.
   */
  virtual void postAddResidualObject(ResidualObject & object) override;

  /// Vector tag ID of the load residual
  const TagID _load_vector_tag;

  /// Matrix tag ID of the load Jacobian
  const TagID _load_matrix_tag;

  /// Whether any residual object has been routed to the load vector tag
  bool _has_load_objects;
};
