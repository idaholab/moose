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

#ifndef LIBMESHPARTITIONER_H
#define LIBMESHPARTITIONER_H

// MOOSE includes
#include "MooseEnum.h"
#include "MoosePartitioner.h"

class LibmeshPartitioner;
class MooseMesh;

namespace libMesh
{
class SubdomainPartitioner;
}

template <>
InputParameters validParams<LibmeshPartitioner>();

class LibmeshPartitioner : public MoosePartitioner
{
public:
  LibmeshPartitioner(const InputParameters & params);
  virtual ~LibmeshPartitioner();

  virtual std::unique_ptr<Partitioner> clone() const;
  virtual void partition(MeshBase & mesh, const unsigned int n);
  virtual void partition(MeshBase & mesh);
  virtual void
  prepare_blocks_for_subdomain_partitioner(SubdomainPartitioner & subdomain_partitioner);

protected:
  virtual void _do_partition(MeshBase & mesh, const unsigned int n);

  std::unique_ptr<Partitioner> _partitioner;
  MooseEnum _partitioner_name;
  const std::vector<std::vector<SubdomainName>> & _subdomain_blocks;
  MooseMesh & _mesh;
};

#endif /* LIBMESHPARTITIONER_H */
