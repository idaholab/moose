//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEPARTITIONER_H
#define MOOSEPARTITIONER_H

// MOOSE includes
#include "MooseObject.h"
#include "Restartable.h"

#include "libmesh/partitioner.h"

// Forward declarations
class MoosePartitioner;

template <>
InputParameters validParams<MoosePartitioner>();

/**
 * Base class for MOOSE partitioner
 */
class MoosePartitioner : public libMesh::Partitioner, public MooseObject, public Restartable
{
public:
  MoosePartitioner(const InputParameters & params);
};

#endif /* MOOSEPARTITIONER_H */
