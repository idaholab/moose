//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppConservativeTransfer.h"

#include "libmesh/mesh_function.h"

/**
 * It is a general field transfer. It will do the following things
 * 1) From part of source domain to part of domain. Support subdomains to
 *  subdomains
 * 2) Support vector vars and regular vars
 * 3) Support higher order FEM
 */
class MultiAppGeneralFieldTransfer : public MultiAppConservativeTransfer
{
public:
  static InputParameters validParams();

  MultiAppGeneralFieldTransfer(const InputParameters & parameters);

  virtual void execute() override;

private:
  /// A map from pid to a set of points
  typedef std::unordered_map<processor_id_type, std::vector<Point>> ProcessorToPointVec;

  /// Point information
  struct PointInfo
  {
    unsigned int problem_id;   // problem id
    dof_id_type dof_object_id; // node or elem id
    dof_id_type offset;        // Useful when there are more than one point in a given dof object
  };

  /// A map from pid to a set of point info
  typedef std::unordered_map<processor_id_type, std::vector<PointInfo>> ProcessorToPointInfoVec;

  /// A map from dof object to interpolation values
  typedef std::unordered_map<std::pair<unsigned int, dof_id_type>,
                             std::vector<std::pair<Real, processor_id_type>>>
      DofobjectToInterpValVec;

  /// The number of variables to transfer
  unsigned int _var_size;
  bool _error_on_miss;

  /// Target mesh blocks this transfer is restricted to
  std::set<SubdomainID> _to_blocks;

  /// Source mesh blocks this transfer is restricted to
  std::set<SubdomainID> _from_blocks;

  /// Number of froms per processor
  std::vector<unsigned int> _froms_per_proc;

  /// Bounding boxes for all processors
  std::vector<BoundingBox> _bboxes;

  /// A map from processor to pointInfo vector
  ProcessorToPointInfoVec _processor_to_pointInfoVec;

  /**
   * Performs the transfer for the variable of index i
   */
  void transferVariable(unsigned int i);

  /*
   * Extract to-points for which we need to compute interpolation values on the source domains
   */
  void extractOutgoingPoints(const VariableName & var_name, ProcessorToPointVec & outgoing_points);

  /*
   * Which processors include this point
   */
  void locatePointReceivers(const Point point, std::vector<processor_id_type> & processors);

  /*
   * Whether or not it is block restricted transfer for target domains
   */
  bool blockRestrictedTarget() const;

  /*
   * Whether or not it is block restricted transfer for source domains
   */
  bool blockRestrictedSource() const;

  /*
   * Whether or not a given element has blocks
   */
  bool hasBlocks(std::set<SubdomainID> & blocks, const Elem * elem) const;

  /*
   * Whether or not a given node has blocks
   */
  bool hasBlocks(std::set<SubdomainID> & blocks, const MooseMesh * mesh, const Node * node) const;

  /*
   * Local from bounding boxes for current processor
   */
  void extractLocalFromBoundingBoxes(std::vector<BoundingBox> & local_bboxes);

  /*
   * Build mesh functions
   */
  void buildMeshFunctions(const VariableName & var_name,
                          std::vector<std::shared_ptr<MeshFunction>> & local_meshfuns);

  /*
   * Evaluate interpolation values for incoming points
   */
  void evaluateInterpValues(const std::vector<BoundingBox> & local_bboxes,
                            const std::vector<std::shared_ptr<MeshFunction>> & local_meshfuns,
                            const std::vector<Point> & incoming_points,
                            std::vector<Real> & outgoing_vals);

  /*
   * cache incoming values
   */
  void cacheIncomingInterpVals(processor_id_type pid,
                               const VariableName & var_name,
                               std::vector<PointInfo> & pointInfoVec,
                               const std::vector<Real> & incoming_vals,
                               DofobjectToInterpValVec & dofobject_to_valsvec);

  /*
   * Set values to solution
   */
  void setSolutionVectorValues(const VariableName & var,
                               DofobjectToInterpValVec & dofobject_to_valsvec);

  /*
   * Cache pointInfo
   */
  void cacheOutgoingPointInfor(const Point point, const dof_id_type dof_object_id, const unsigned int problem_id, ProcessorToPointVec & outgoing_points);
};
