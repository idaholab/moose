/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DOMAININTEGRALQFUNCTION_H
#define DOMAININTEGRALQFUNCTION_H

#include "AuxKernel.h"
#include "CrackFrontDefinition.h"

/**
 * Coupled auxiliary value
 */
class DomainIntegralQFunction : public AuxKernel
{
public:
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

template <>
InputParameters validParams<DomainIntegralQFunction>();

#endif // DOMAININTEGRALQFUNCTION_H
