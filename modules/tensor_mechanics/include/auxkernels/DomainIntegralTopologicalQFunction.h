/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DOMAININTEGRALTOPOLOGICALQFUNCTION_H
#define DOMAININTEGRALTOPOLOGICALQFUNCTION_H

#include "AuxKernel.h"
#include "CrackFrontDefinition.h"

/**
 * Coupled auxiliary value
 */
class DomainIntegralTopologicalQFunction : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  DomainIntegralTopologicalQFunction(const InputParameters & parameters);

  virtual ~DomainIntegralTopologicalQFunction() {}

protected:
  virtual void initialSetup();
  virtual Real computeValue();

private:
  const unsigned int _ring_number;
  const CrackFrontDefinition * const _crack_front_definition;
  bool _has_crack_front_point_index;
  const unsigned int _crack_front_point_index;
  bool _treat_as_2d;
};

template <>
InputParameters validParams<DomainIntegralTopologicalQFunction>();

#endif // DOMAININTEGRALTOPOLOGICALQFUNCTION_H
