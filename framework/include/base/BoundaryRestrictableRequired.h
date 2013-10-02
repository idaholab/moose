/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef BOUNDARYRESTRICTABLEREQUIRED_H
#define BOUNDARYRESTRICTABLEREQUIRED_H

// Moose includes
#include "BoundaryRestrictable.h"

// Forward declarations
class BoundaryRestrictableRequired;

template<>
InputParameters validParams<BoundaryRestrictableRequired>();

/**
 * A class for requiring an object to be boundary restricted.
 * This class acts as a wrapper for BoundaryRestrictable, it allows
 * an additional validParams<> specialization that adds the 'boundary'
 * parameter as required.
 */
class BoundaryRestrictableRequired : public BoundaryRestrictable
{
public:
  BoundaryRestrictableRequired(const std::string name, InputParameters & parameters);
};

#endif // BOUNDARYRESTRICTABLEREQURIED_H
