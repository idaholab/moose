//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Transfer.h"
#include "MooseEnum.h"

#include "libmesh/bounding_box.h"

// Forward declarations
class MultiAppTransfer;
class MooseMesh;
class MultiApp;

template <>
InputParameters validParams<MultiAppTransfer>();

/**
 * Base class for all MultiAppTransfer objects.
 *
 * MultiAppTransfers are objects that push and pull values to and from
 * MultiApp objects from and to the main (master) system.
 *
 * Classes that inherit from this class still need to override the
 * execute() method from Transfer.
 */
class MultiAppTransfer : public Transfer
{
public:
  MultiAppTransfer(const InputParameters & parameters);

  enum DIRECTION
  {
    TO_MULTIAPP,
    FROM_MULTIAPP
  };

  /// Used to construct InputParameters
  static MooseEnum directions() { return MooseEnum("to_multiapp from_multiapp"); }

  /// The direction this Transfer is going in
  int direction() { return _direction; }

  /**
   * Utility to verify that the vEariable in the destination system exists.
   */
  void variableIntegrityCheck(const AuxVariableName & var_name) const;

  /// Return the MultiApp that this transfer belongs to
  const std::shared_ptr<MultiApp> getMultiApp() const { return _multi_app; }

  /// Return the execution flags, handling "same_as_multiapp"
  virtual const std::vector<ExecFlagType> & execFlags() const;

protected:
  /// The MultiApp this Transfer is transferring data to or from
  std::shared_ptr<MultiApp> _multi_app;

  /// Whether we're transferring to or from the MultiApp
  const MooseEnum _direction;

  /**
   * This method will fill information into the convenience member variables
   * (_to_problems, _from_meshes, etc.)
   */
  void getAppInfo();

  std::vector<FEProblemBase *> _to_problems;
  std::vector<FEProblemBase *> _from_problems;
  std::vector<EquationSystems *> _to_es;
  std::vector<EquationSystems *> _from_es;
  std::vector<MooseMesh *> _to_meshes;
  std::vector<MooseMesh *> _from_meshes;
  std::vector<Point> _to_positions;
  std::vector<Point> _from_positions;

  /// True if displaced mesh is used for the source mesh, otherwise false
  bool _displaced_source_mesh;
  /// True if displaced mesh is used for the target mesh, otherwise false
  bool _displaced_target_mesh;

  ///@{
  /**
   * Return the bounding boxes of all the "from" domains, including all the domains not local to
   * this processor. The is a boundary restricted version which will return a degenerate minimum
   * boundary box (min, min, min, min, min, min) in the case where the source domain doesn't
   * have any active nodes on the boundary.
   */
  std::vector<BoundingBox> getFromBoundingBoxes();
  std::vector<BoundingBox> getFromBoundingBoxes(BoundaryID boundary_id);
  ///@}

  /**
   * Return the number of "from" domains that each processor owns.
   */
  std::vector<unsigned int> getFromsPerProc();

  /**
   * If we are transferring to a multiapp, return the appropriate solution
   * vector
   */
  NumericVector<Real> & getTransferVector(unsigned int i_local, std::string var_name);

  // Given local app index, returns global app index.
  std::vector<unsigned int> _local2global_map;
};

  /**
   * Helper method for checking the 'check_multiapp_execute_on' flag.
   *
   * This method was added to allow the check to be delayed by child classes,
   * see StochasticToolsTransfer for an example.
   */
  void checkMultiAppExecuteOn();
};
