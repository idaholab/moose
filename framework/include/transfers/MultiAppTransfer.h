//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Transfer.h"
#include "MultiMooseEnum.h"
#include "MultiApp.h"
#include "MooseAppCoordTransform.h"

#include "libmesh/bounding_box.h"

class MooseMesh;

/**
 * Base class for all MultiAppTransfer objects.
 *
 * MultiAppTransfers are objects that push and pull values to and from
 * MultiApp objects from and to the main (parent app) system.
 *
 * Classes that inherit from this class still need to override the
 * execute() method from Transfer.
 */
class MultiAppTransfer : public Transfer
{
public:
  static InputParameters validParams();

  MultiAppTransfer(const InputParameters & parameters);

  /**
   * Utility to verify that the variable in the destination system exists.
   */
  void variableIntegrityCheck(const AuxVariableName & var_name, bool is_from_multiapp) const;

  void initialSetup() override;

  /**
   * Use this getter to obtain the MultiApp for transfers with a single direction
   */
  const std::shared_ptr<MultiApp> getMultiApp() const
  {
    if (_from_multi_app && _to_multi_app && _from_multi_app != _to_multi_app)
      mooseError("Unclear which app you want to retrieve from Transfer ", name());
    else if (_from_multi_app)
      return _from_multi_app;
    else if (_to_multi_app)
      return _to_multi_app;
    else if (_multi_app)
      return _multi_app;
    else
      mooseError("Should not get here, there should be a multiapp");
  }

  /// Get the MultiApp to transfer data from
  const std::shared_ptr<MultiApp> getFromMultiApp() const
  {
    if (!_from_multi_app)
      mooseError(
          "A from_multiapp was requested but is unavailable. Check the from_multi_app parameter");
    else
      return _from_multi_app;
  }

  /// Get the MultiApp to transfer data to
  const std::shared_ptr<MultiApp> getToMultiApp() const
  {
    if (!_to_multi_app)
      mooseError(
          "A to_multiapp was requested but is unavailable. Check the to_multi_app parameter");
    else
      return _to_multi_app;
  }

  /**
   * Get the name of thing being transferred from
   * @return the name of the multiapp or "Parent"
   */
  std::string getFromName() const
  {
    if (_from_multi_app)
      return _from_multi_app->name();
    else
      return "Parent";
  }

  /**
   * Get the name of thing being transferred to
   * @return the name of the multiapp or "Parent"
   */
  std::string getToName() const
  {
    if (_to_multi_app)
      return _to_multi_app->name();
    else
      return "Parent";
  }

  /**
   * Add the option to skip coordinate collapsing in coordinate transformation operations
   */
  static void addSkipCoordCollapsingParam(InputParameters & params);

  /// Whether the transfer owns a non-null from_multi_app
  bool hasFromMultiApp() const { return !(!_from_multi_app); }

  /// Whether the transfer owns a non-null to_multi_app
  bool hasToMultiApp() const { return !(!_to_multi_app); }

  /**
   * This method will fill information into the convenience member variables
   * (_to_problems, _from_meshes, etc.)
   */
  virtual void getAppInfo();

protected:
  /**
   * Add the bounding box factor parameter to the supplied input parameters
   */
  static void addBBoxFactorParam(InputParameters & params);

  /**
   * Transform a bounding box according to the transformations in the provided coordinate
   * transformation object
   */
  static void transformBoundingBox(libMesh::BoundingBox & box,
                                   const MultiAppCoordTransform & transform);

  /// Deprecated class attribute for compatibility with the apps
  std::shared_ptr<MultiApp> _multi_app;

  std::vector<FEProblemBase *> _to_problems;
  std::vector<FEProblemBase *> _from_problems;
  std::vector<libMesh::EquationSystems *> _to_es;
  std::vector<libMesh::EquationSystems *> _from_es;
  std::vector<MooseMesh *> _to_meshes;
  std::vector<MooseMesh *> _from_meshes;
  std::vector<Point> _to_positions;
  std::vector<Point> _from_positions;
  std::vector<std::unique_ptr<MultiAppCoordTransform>> _to_transforms;
  std::vector<std::unique_ptr<MultiAppCoordTransform>> _from_transforms;

  /// Whether to skip coordinate collapsing (transformations of coordinates between applications
  /// using different frames of reference)
  const bool _skip_coordinate_collapsing;

  /// True if displaced mesh is used for the source mesh, otherwise false
  bool _displaced_source_mesh;
  /// True if displaced mesh is used for the target mesh, otherwise false
  bool _displaced_target_mesh;

  /// Extend (or contract) bounding box by a factor in all directions
  /// Greater than one values of this member may be necessary because the nearest bounding
  /// box does not necessarily give you the closest node/element. It will depend
  /// on the partition and geometry. A node/element will more likely find its
  /// nearest source element/node by extending bounding boxes. If each of the
  /// bounding boxes covers the entire domain, a node/element will be able to
  /// find its nearest source element/node for sure,
  /// but at the same time, more communication will be involved and can be expensive.
  Real _bbox_factor;

  ///@{
  /**
   * Return the bounding boxes of all the "from" domains, including all the domains not local to
   * this processor. There is a boundary restricted version which will return a degenerate minimum
   * boundary box (min, min, min, min, min, min) in the case where the source domain doesn't
   * have any active nodes on the boundary.
   * Note: bounding boxes are in the reference space when using coordinate transformations /
   * positions
   * Note: global bounding boxes are not indexed by app number. But rather outer indexing is by
   * process, then the inner indexing is by local app number.
   */
  std::vector<libMesh::BoundingBox> getFromBoundingBoxes();
  std::vector<libMesh::BoundingBox> getFromBoundingBoxes(BoundaryID boundary_id);
  ///@}

  /**
   * Return the number of "from" domains that each processor owns.
   * Note: same indexing as getFromBoundingBoxes
   */
  std::vector<unsigned int> getFromsPerProc();

  /**
   * If we are transferring to a multiapp, return the appropriate solution
   * vector
   */
  libMesh::NumericVector<Real> & getTransferVector(unsigned int i_local, std::string var_name);

  /// Given local app index, returns global app index.
  std::vector<unsigned int> _to_local2global_map;
  /// Given local app index, returns global app index.
  std::vector<unsigned int> _from_local2global_map;

  /// Return the global app index from the local index in the "from-multiapp" transfer direction
  unsigned int getGlobalSourceAppIndex(unsigned int i_from) const;
  /// Return the global app index from the local index in the "to-multiapp" transfer direction
  unsigned int getGlobalTargetAppIndex(unsigned int i_to) const;
  /// Return the local app index from the global index in the "from-multiapp" transfer direction
  /// We use the fact that global app indexes are consecutive on a given rank
  unsigned int getLocalSourceAppIndex(unsigned int i_from) const;

  /// Whether the transfer supports siblings transfer
  virtual void checkSiblingsTransferSupported() const
  {
    mooseError("Siblings transfer not supported. You cannot transfer both from a multiapp to "
               "another multiapp");
  }

  /**
   * Error if executing this MooseObject on EXEC_TRANSFER in a source multiapp (from_multiapp, e.g.
   * child/sibling app). Note that, conversely, when the parent app is the source application, it is
   * usually \emph desired to use EXEC_TRANSFER for a MooseObject that provides the values to
   * transfer.
   * @param object_name name of the object to check the execute_on flags for
   */
  void errorIfObjectExecutesOnTransferInSourceApp(const std::string & object_name) const;

  /**
   * Get the target app point from a point in the reference frame
   * @param p the point in the reference frame
   * @param local_i_to the local target problem into
   * @param phase the phase of the transfer where this is being attempted in case we have
   *              to output an info message that the coordinate collapse is not being applied
   * @return the point in the target app frame
   */
  Point getPointInTargetAppFrame(const Point & p,
                                 unsigned int local_i_to,
                                 const std::string & phase) const;

  /**
   * Helper method for checking the 'check_multiapp_execute_on' flag.
   *
   * This method was added to allow the check to be delayed by child classes,
   * see StochasticToolsTransfer for an example.
   */
  void checkMultiAppExecuteOn();

  /**
   * Helper for checking a problem for a variable.
   *
   * @param fe_problem The problem that should contain the variable
   * @param var_name The name of the variable that should exist within the problem
   * @param param_name (optional) The input file parameter name for throwing paramError, if not
   *                   provided a mooseError is thrown.
   */
  void checkVariable(const FEProblemBase & fe_problem,
                     const VariableName & var_name,
                     const std::string & param_name = "") const;

  /// Extends bounding boxes to avoid missing points
  void extendBoundingBoxes(const Real factor, std::vector<libMesh::BoundingBox> & bboxes) const;

private:
  /**
   * Whether this transfer handles non-translation-based transformations, e.g. whether it uses the
   * \p MooseAppCoordTransform object
   */
  virtual bool usesMooseAppCoordTransform() const { return false; }

  /// The MultiApps this Transfer is transferring data to or from
  std::shared_ptr<MultiApp> _from_multi_app;
  std::shared_ptr<MultiApp> _to_multi_app;

  void getFromMultiAppInfo();
  void getToMultiAppInfo();

  /// The moose coordinate transformation object describing rotations, scaling, and coordinate
  /// system of the from application
  std::unique_ptr<MooseAppCoordTransform> _from_moose_app_transform;

  /// The moose coordinate transformation object describing rotations, scaling, and coordinate
  /// system of the to application
  std::unique_ptr<MooseAppCoordTransform> _to_moose_app_transform;
};
