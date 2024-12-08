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
 * Evaluates origin shape functions to compute the target variables
 */
class MultiAppGeneralFieldShapeEvaluationTransfer : public MultiAppGeneralFieldTransfer
{
public:
  static InputParameters validParams();

  MultiAppGeneralFieldShapeEvaluationTransfer(const InputParameters & parameters);

protected:
  virtual void prepareEvaluationOfInterpValues(const unsigned int var_index) override;

  virtual void
  evaluateInterpValues(const std::vector<std::pair<Point, unsigned int>> & incoming_points,
                       std::vector<std::pair<Real, Real>> & outgoing_vals) override;

private:
  bool usesMooseAppCoordTransform() const override { return true; }
  /*
   * Build mesh functions local to the domain
   * @param[in] var_name the variable to build the mesh functions for
   * @param[out] the mesh functions
   */
  void buildMeshFunctions(const unsigned int var_index,
                          std::vector<libMesh::MeshFunction> & local_meshfuns);

  /*
   * Evaluate interpolation values for incoming points
   * @param[in] bounding boxes to restrict the evaluation domain
   * @param[in] the mesh functions to use for evaluation
   * @param[in] the points to evaluate the variable shape functions at
   * @param[out] the values of the variables
   */
  void evaluateInterpValuesWithMeshFunctions(
      const std::vector<BoundingBox> & local_bboxes,
      std::vector<libMesh::MeshFunction> & local_meshfuns,
      const std::vector<std::pair<Point, unsigned int>> & incoming_points,
      std::vector<std::pair<Real, Real>> & outgoing_vals);

  /*
   * Bounding boxes
   */
  std::vector<libMesh::BoundingBox> _local_bboxes;
  /*
   * Local mesh functions
   */
  std::vector<libMesh::MeshFunction> _local_meshfuns;
};
