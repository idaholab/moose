//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "CrackFrontDefinition.h"

/**
 * Coupled auxiliary value
 */
class DomainIntegralQFunction : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  DomainIntegralQFunction(const InputParameters & parameters);

  virtual ~DomainIntegralQFunction() {}

protected:
  virtual void initialSetup();
  virtual Real computeValue();
  void projectToFrontAtPoint(Real & dist_to_front, Real & dist_along_tangent);

private:
  const Real _j_integral_radius_inner;
  const Real _j_integral_radius_outer;
  const CrackFrontDefinition * const _crack_front_definition;
  bool _has_crack_front_point_index;
  const unsigned int _crack_front_point_index;
  bool _treat_as_2d;
  bool _is_point_on_intersecting_boundary;
};
