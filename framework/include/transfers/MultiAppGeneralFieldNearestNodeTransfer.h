//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppGeneralFieldTransfer.h"
#include "libmesh/mesh_function.h"
#include "MooseHashing.h"
#include "KDTree.h"

/**
 * It is a general field transfer. It will do the following things
 * 1) From part of source domain to part of domain. Support subdomains to
 *  subdomains
 * 2) Support vector vars and regular vars
 * 3) Support higher order FEM
 */
class MultiAppGeneralFieldNearestNodeTransfer : public MultiAppGeneralFieldTransfer
{
public:
  static InputParameters validParams();

  MultiAppGeneralFieldNearestNodeTransfer(const InputParameters & parameters);

protected:
  virtual void prepareEvaluationOfInterpValues(const VariableName & var_name) override;

  virtual void evaluateInterpValues(const std::vector<Point> & incoming_points,
                                    std::vector<std::pair<Real, Real>> & outgoing_vals) override;

private:
  /*
   * Build mesh functions
   */
  void buildKDTrees(const VariableName & var_name,
                    std::vector<std::shared_ptr<KDTree>> & local_kdtrees,
                    std::vector<std::vector<Point>> & local_points,
                    std::vector<std::vector<Real>> & local_values);

  /*
   * Evaluate interpolation values for incoming points
   */
  void evaluateInterpValuesNearestNode(const std::vector<std::shared_ptr<KDTree>> & local_kdtrees,
                                       const std::vector<std::vector<Point>> & local_points,
                                       const std::vector<std::vector<Real>> & local_values,
                                       const std::vector<Point> & incoming_points,
                                       std::vector<std::pair<Real, Real>> & outgoing_vals);

  std::vector<std::shared_ptr<KDTree>> _local_kdtrees;

  std::vector<std::vector<Point>> _local_points;

  std::vector<std::vector<Real>> _local_values;

  unsigned int _num_nearest_points;
};
