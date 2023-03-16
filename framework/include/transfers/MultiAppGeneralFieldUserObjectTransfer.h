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
 * Transfers values computed in the origin mesh by the source user object spatialValue() routine
 * at locations in the target mesh.
 */
class MultiAppGeneralFieldUserObjectTransfer : public MultiAppGeneralFieldTransfer
{
public:
  static InputParameters validParams();

  MultiAppGeneralFieldUserObjectTransfer(const InputParameters & parameters);

protected:
  virtual void prepareEvaluationOfInterpValues(const unsigned int /* var_index */) override;

  virtual void evaluateInterpValues(const std::vector<Point> & incoming_points,
                                    std::vector<std::pair<Real, Real>> & outgoing_vals) override;

private:
  /*
   * Evaluate interpolation values for incoming points
   * @param[in] bounding boxes to restrict the evaluation domain
   * @param[in] the mesh functions to use for evaluation
   * @param[in] the points to evaluate the variable shape functions at
   * @param[out] the values of the variables
   */
  void evaluateInterpValuesWithUserObjects(const std::vector<BoundingBox> & local_bboxes,
                                           const std::vector<Point> & incoming_points,
                                           std::vector<std::pair<Real, Real>> & outgoing_vals);

  /// Name of the source user object in all the source problems
  const std::string _user_object_name;

  /*
   * Bounding boxes
   */
  std::vector<BoundingBox> _local_bboxes;
};
