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

#ifndef MULTIAPPNODALINTERPOLATIONTRANSFER_H
#define MULTIAPPNODALINTERPOLATIONTRANSFER_H

#include "MultiAppTransfer.h"

class MooseVariable;
class MultiAppNodalInterpolationTransfer;

template<>
InputParameters validParams<MultiAppNodalInterpolationTransfer>();

/**
 * Copy the value to the target domain from the nearest node in the source domain.
 */
class MultiAppNodalInterpolationTransfer :
  public MultiAppTransfer
{
public:
  MultiAppNodalInterpolationTransfer(const std::string & name, InputParameters parameters);
  virtual ~MultiAppNodalInterpolationTransfer() {}

  virtual void execute();

protected:
  /**
   * Return the nearest node to the point p.
   * @param mesh The mesh you want to search.
   * @param p The point you want to find the nearest node to.
   * @param distance This will hold the distance between the returned node and p
   * @return The Node closest to point p.
   */
  Node * getNearestNode(const Point & p, Real & distance, const MeshBase::const_node_iterator & nodes_begin, const MeshBase::const_node_iterator & nodes_end);

  AuxVariableName _to_var_name;
  VariableName _from_var_name;

  bool _displaced_source_mesh;
  bool _displaced_target_mesh;

  unsigned int _num_points;
  Real _power;
  MooseEnum _interp_type;
  Real _radius;
};

#endif /* MULTIAPPVARIABLEVALUESAMPLEPOSTPROCESSORTRANSFER_H */
