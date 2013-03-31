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

#ifndef MULTIAPPNEARESTNODETRANSFER_H
#define MULTIAPPNEARESTNODETRANSFER_H

#include "MultiAppTransfer.h"

class MooseVariable;
class MultiAppNearestNodeTransfer;

template<>
InputParameters validParams<MultiAppNearestNodeTransfer>();

/**
 * Copy the value to the target domain from the nearest node in the source domain.
 */
class MultiAppNearestNodeTransfer :
  public MultiAppTransfer
{
public:
  MultiAppNearestNodeTransfer(const std::string & name, InputParameters parameters);
  virtual ~MultiAppNearestNodeTransfer() {}

  virtual void execute();

protected:
  /**
   * Return the nearest node to the point p.
   * @param mesh The mesh you want to search.
   * @param p The point you want to find the nearest node to.
   * @return The Node closest to point p.
   */
  Node * getNearestNode(MeshBase & mesh, const Point & p);

  AuxVariableName _to_var_name;
  VariableName _from_var_name;
};

#endif /* MULTIAPPVARIABLEVALUESAMPLEPOSTPROCESSORTRANSFER_H */
