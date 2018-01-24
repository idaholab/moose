//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTIAPPINTERPOLATIONTRANSFER_H
#define MULTIAPPINTERPOLATIONTRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"

#include "libmesh/mesh_base.h"

// Forward declarations
class MultiAppInterpolationTransfer;

template <>
InputParameters validParams<MultiAppInterpolationTransfer>();

/**
 * Copy the value to the target domain from the nearest node in the source domain.
 */
class MultiAppInterpolationTransfer : public MultiAppTransfer
{
public:
  MultiAppInterpolationTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

protected:
  /**
   * Return the nearest node to the point p.
   * @param p The point you want to find the nearest node to.
   * @param distance This will hold the distance between the returned node and p
   * @param nodes_begin - iterator to the beginning of the node list
   * @param nodes_end - iterator to the end of the node list
   * @return The Node closest to point p.
   */
  Node * getNearestNode(const Point & p,
                        Real & distance,
                        const MeshBase::const_node_iterator & nodes_begin,
                        const MeshBase::const_node_iterator & nodes_end);

  AuxVariableName _to_var_name;
  VariableName _from_var_name;

  unsigned int _num_points;
  Real _power;
  MooseEnum _interp_type;
  Real _radius;
};

#endif /* MULTIAPPINTERPOLATIONTRANSFER_H */
