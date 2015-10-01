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

#include "MoosePartitioner.h"

class LibmeshPartitioner;

template<>
InputParameters validParams<LibmeshPartitioner>();

class LibmeshPartitioner : public MoosePartitioner
{
public:
  LibmeshPartitioner(const InputParameters & params);
  virtual ~LibmeshPartitioner();

  virtual Partitioner * getPartitioner();
protected:
  Partitioner * _partitioner;
  MooseEnum _partitioner_name;
};

#endif /* LIBMESHPARTITIONER_H */
