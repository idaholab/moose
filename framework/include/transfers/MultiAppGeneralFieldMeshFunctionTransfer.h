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

/**
 * It is a general field transfer. It will do the following things
 * 1) From part of source domain to part of domain. Support subdomains to
 *  subdomains
 * 2) Support vector vars and regular vars
 * 3) Support higher order FEM
 */
class MultiAppGeneralFieldMeshFunctionTransfer : public MultiAppGeneralFieldTransfer
{
public:
  static InputParameters validParams();

  MultiAppGeneralFieldMeshFunctionTransfer(const InputParameters & parameters);

protected:
  virtual void prepareEvaluationOfInterpValues(const VariableName & var_name) override;

  virtual void evaluateInterpValues(const std::vector<Point> & incoming_points,
                                    std::vector<std::pair<Real, Real>> & outgoing_vals) override;

private:
  /*
   * Build mesh functions
   */
  void buildMeshFunctions(const VariableName & var_name,
                          std::vector<std::shared_ptr<MeshFunction>> & local_meshfuns);

  /*
   * Evaluate interpolation values for incoming points
   */
  void evaluateInterpValuesWithMeshFunctions(
      const std::vector<BoundingBox> & local_bboxes,
      const std::vector<std::shared_ptr<MeshFunction>> & local_meshfuns,
      const std::vector<Point> & incoming_points,
      std::vector<std::pair<Real, Real>> & outgoing_vals);

  /// Error out when some points can not be located
  bool _error_on_miss;

  /*
   * Bounding boxes
   */
  std::vector<BoundingBox> _local_bboxes;
  /*
   * Local mesh funcitons
   */
  std::vector<std::shared_ptr<MeshFunction>> _local_meshfuns;
};
