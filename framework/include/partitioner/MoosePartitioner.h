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

#ifndef MOOSEPARTITIONER_H
#define MOOSEPARTITIONER_H

#include "MooseObject.h"
#include "Restartable.h"

#include "libmesh/partitioner.h"

//Forward declarations
namespace libMesh
{
  class MeshBase;
}

class MoosePartitioner;

template<>
InputParameters validParams<MoosePartitioner>();

/**
 * Base class for MOOSE partitioner
 */
class MoosePartitioner :
  public MooseObject,
  public Restartable
{
public:
  MoosePartitioner(const InputParameters & params);
  virtual ~MoosePartitioner();

  virtual Partitioner * getPartitioner() = 0;
};

#endif /* MOOSEPARTITIONER_H */
