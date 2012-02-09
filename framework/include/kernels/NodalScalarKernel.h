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

#ifndef NODALSCALARKERNEL_H
#define NODALSCALARKERNEL_H

#include "ScalarKernel.h"
#include "Coupleable.h"


class NodalScalarKernel;

template<>
InputParameters validParams<NodalScalarKernel>();

class NodalScalarKernel :
  public ScalarKernel,
  public Coupleable
{
public:
  NodalScalarKernel(const std::string & name, InputParameters parameters);
  virtual ~NodalScalarKernel();

  virtual void reinit();

protected:
  std::vector<unsigned int> _node_ids;                  ///< List of node IDs
};

#endif /* NODALSCALARKERNEL_H */
