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
#ifndef MULTIAPP_H
#define MULTIAPP_H

#include "MooseApp.h"
#include "MooseEnum.h"
#include "Restartable.h"

// libMesh includes
#include "libmesh/mesh_tools.h"
#include "libmesh/numeric_vector.h"

class MultiApp;
class UserObject;
class FEProblem;
class Executioner;
class OutputWarehouse;
namespace libMesh{ namespace MeshTools { class BoundingBox; } }

template<>
InputParameters validParams<MultiApp>();

/**
 * A MultiApp represents one or more MOOSE applications that are running simultaneously.
 * These other MOOSE apps generally represent some "sub-solve" or "embedded-solves"
 * of the overall nonlinear solve.
 */
class MultiApp :
  public MooseObject,
  public Restartable
{
public:
  MultiApp(const std::string & name, InputParameters parameters);

  virtual ~MultiApp();

  virtual void init();

  /**
   * Gets called just before transfers are done _to_ the MultiApp
   * (Which is just before the MultiApp is solved)
   */
  virtual void preTransfer(Real dt, Real target_time);

  /**
   * Re-solve all of the Apps.
   *
   * Can be called multiple times to resolve the same timestep
   * if auto_advance=false. Time is not actually advanced until
   * advanceStep() is called.
   *
   * Note that auto_advance=false might not be compatible with
   * the options for the MultiApp
   */
  virtual void solveStep(Real dt, Real target_time, bool auto_advance=true) = 0;

  /**
   * Actually advances time and causes output.
   *
   * If auto_advance=true was used in solveStep() then this function
   * will do nothing.
   */
  virtual void advanceStep() = 0;

  /**
   * @param app The global app number to get the Executioner for
   * @return The Executioner associated with that App.
   */
  virtual Executioner * getExecutioner(unsigned int app);

  /**
   * Get the BoundingBox for the mesh associated with app
   * The bounding box will be shifted to be in the correct position
   * within the master domain.
   * If the MultiApp is in an RZ coordinate system the box will be
   * the size it would be if the geometry were 3D (ie if you were to revolve
   * the geometry around the axis to create the 3D geometry).
   * @param app The global app number you want to get the bounding box for
   */
  virtual MeshTools::BoundingBox getBoundingBox(unsigned int app);

  /**
   * @return When this MultiApp will be executed.
   */
  int executeOn() { return _execute_on; }

  /**
   * Get the FEProblem this MultiApp is part of.
   */
  FEProblem * problem() { return _fe_problem; }

  /**
   * Get the FEProblem for the global app is part of.
   * @param app The global app number
   */
  FEProblem * appProblem(unsigned int app);

  /**
   * Get a UserObject base for a specific global app
   * @param app The global app number you want to get a UserObject from.
   * @param name The name of the UserObject.
   */
  const UserObject & appUserObjectBase(unsigned int app, const std::string & name);

  /**
   * Get a Postprocessor value for a specified global app
   * @param app The global app number you want to get a Postprocessor from.
   * @param name The name of the Postprocessor.
   */
  Real appPostprocessorValue(unsigned int app, const std::string & name);

  /**
   * Get the vector to transfer to for this MultiApp.
   * In general this is the Auxiliary system solution vector.
   *
   * @param app The global app number you want the transfer vector for.
   * @param var_name The name of the variable you are going to be transferring to.
   * @return The vector to fill.
   */
  virtual NumericVector<Number> & appTransferVector(unsigned int app, std::string var_name);

  /**
   * @return Number of Global Apps in this MultiApp
   */
  unsigned int numGlobalApps() { return _total_num_apps; }

  /**
   * @return Number of Apps on local processor.
   */
  unsigned int numLocalApps() { return _my_num_apps; }

  /**
   * @return The global number of the first app on the local processor.
   */
  unsigned int firstLocalApp() { return _first_local_app; }

  /**
   * Whether or not this MultiApp has an app on this processor.
   */
  bool hasApp() { return _has_an_app; }

  /**
   * Whether or not the given global app number is on this processor.
   * @param global_app The global app number in question
   * @return True if the global app is on this processor
   */
  bool hasLocalApp(unsigned int global_app);

  /**
   * The physical position of a global App number
   * @param app The global app number you want the position for.
   * @return the position
   */
  Point position(unsigned int app) { return _positions[app]; }

  /**
   * "Reset" the App corresponding to the global App number
   * passed in.  "Reset" means that the App will be deleted
   * and recreated.  The time for the new App will be set
   * to the current simulation time.  This might be handy
   * if some sub-app in your simulation needs to get replaced
   * by a "new" piece of material.
   *
   * @param global_app The global app number to reset.
   * @param time The time to set as the the time for the new app, this should really be the time the old app was at.
   */
  virtual void resetApp(unsigned int global_app, Real time = 0.0);

  /**
   * Move the global_app to Point p.
   *
   * @param global_app The global app number in question
   * @param p The new position of the App.
   */
  virtual void moveApp(unsigned int global_app, Point p);

  /**
   * For apps outputting in position we need to change their output positions
   * if their parent app moves.
   */
  virtual void parentOutputPositionChanged();

  /**
   * Get the MPI communicator this MultiApp is operating on.
   * @return The MPI comm for this MultiApp
   */
  MPI_Comm & comm() { return _my_comm; }

  /**
   * Whether or not this processor is the "root" processor for the sub communicator.
   * The "root" processor has rank 0 in the sub communicator
   */
  bool isRootProcessor() { return _my_rank == 0; }

protected:
  /**
   * Helper function for creating an App instance.
   *
   * @param i The local app number to create.
   * @param start_time The initial time for the App
   */
  void createApp(unsigned int i, Real start_time);

  /**
   * Create an MPI communicator suitable for each app.
   *
   * Also find out which communicator we are using and what our first local app is.
   */
  void buildComm();

  /**
   * Map a global App number to the local number.
   * Note: This will error if given a global number that doesn't map to a local number.
   *
   * @param global_app The global app number.
   * @return The local app number.
   */
  unsigned int globalAppToLocal(unsigned int global_app);

  /// The FEProblem this MultiApp is part of
  FEProblem * _fe_problem;

  /// The type of application to build
  std::string _app_type;

  /// The positions as they came in from the input file
  std::vector<Real> _positions_vec;

  /// The positions of all of the apps
  std::vector<Point> _positions;

  /// The input file for each app's simulation
  std::vector<std::string> _input_files;

  /// The output file basename for each multiapp
  std::string _output_base;

  /// The total number of apps to simulate
  unsigned int _total_num_apps;

  /// The number of apps this object is involved in simulating
  unsigned int _my_num_apps;

  /// The number of the first app on this processor
  unsigned int _first_local_app;

  /// The comm that was passed to us specifying our pool of processors
  MPI_Comm _orig_comm;

  /// The MPI communicator this object is going to use.
  MPI_Comm _my_comm;

  /// The number of processors in the original comm
  int _orig_num_procs;

  /// The mpi "rank" of this processor in the original communicator
  int _orig_rank;

  /// Node Name
  std::string _node_name;

  /// The mpi "rank" of this processor in the sub communicator
  int _my_rank;

  /// Pointers to each of the Apps
  std::vector<MooseApp *> _apps;

  /// When this MultiApp will be executed
  MooseEnum _execute_on;

  /// Relative bounding box inflation
  Real _inflation;

  /// Maximum number of processors to give to each app
  unsigned int _max_procs_per_app;

  /// Whether or not to move the output of the MultiApp into position
  bool _output_in_position;

  /// The time at which to reset apps
  Real _reset_time;

  /// The apps to be reset
  std::vector<unsigned int> _reset_apps;

  /// Whether or not apps have been reset
  bool _reset_happened;

  /// The time at which to move apps
  Real _move_time;

  /// The apps to be moved
  std::vector<unsigned int> _move_apps;

  /// The new positions for the apps to be moved
  std::vector<Point> _move_positions;

  /// Whether or not the move has happened
  bool _move_happened;

  /// Whether or not this processor as an App _at all_
  bool _has_an_app;
};

#endif // MULTIAPP_H
