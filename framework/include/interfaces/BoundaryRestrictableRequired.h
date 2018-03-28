//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BOUNDARYRESTRICTABLEREQUIRED_H
#define BOUNDARYRESTRICTABLEREQUIRED_H

// Moose includes
#include "BoundaryRestrictable.h"

// Forward declarations
class BoundaryRestrictableRequired;

template <>
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
  BoundaryRestrictableRequired(const MooseObject * moose_object, bool nodal);
};

#endif // BOUNDARYRESTRICTABLEREQURIED_H
