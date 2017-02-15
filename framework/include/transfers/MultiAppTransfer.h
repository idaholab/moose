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

#ifndef MULTIAPPTRANSFER_H
#define MULTIAPPTRANSFER_H

// MOOSE includes
#include "Transfer.h"
#include "MooseEnum.h"

// libMesh includes
#include "libmesh/mesh_tools.h"

// Forward declarations
class MultiAppTransfer;
class MooseMesh;
class MultiApp;

template<>
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
  MooseEnum _direction;

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

  bool _displaced_source_mesh;
  bool _displaced_target_mesh;

  /**
   * Return the bounding boxes of all the "from" domains, including all the
   * domains not local to this processor.
   */
  std::vector<MeshTools::BoundingBox> getFromBoundingBoxes();

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

#endif /* MULTIAPPTRANSFER_H */
