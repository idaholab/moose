//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTIAPP_H
#define MULTIAPP_H

#include "MooseObject.h"
#include "SetupInterface.h"
#include "Restartable.h"

#include "libmesh/communicator.h"

class MultiApp;
class UserObject;
class FEProblemBase;
class FEProblem;
class Executioner;
class MooseApp;
class Backup;

// libMesh forward declarations
namespace libMesh
{
class BoundingBox;
namespace MeshTools
{
class BoundingBox;
}
template <typename T>
class NumericVector;
} // namespace libMesh

template <>
InputParameters validParams<MultiApp>();

/**
 * Helper class for holding Sub-app backups
 */
class SubAppBackups : public std::vector<std::shared_ptr<Backup>>
{
};

/**
 * A MultiApp represents one or more MOOSE applications that are running simultaneously.
 * These other MOOSE apps generally represent some "sub-solve" or "embedded-solves"
 * of the overall nonlinear solve. If your system support dynamic libraries unregistered
 * Multiapps can be loaded on the fly by setting the exporting the appropriate library
 * path using "MOOSE_LIBRARY_PATH" or by specifying a single input file library path
 * in Multiapps InputParameters object.
 */
class MultiApp : public MooseObject, public SetupInterface, public Restartable
{
public:
  MultiApp(const InputParameters & parameters);

  virtual void preExecute() {}

  virtual void postExecute();

  /**
   * Called just after construction to allow derived classes to set _positions;
   */
  void setupPositions();

  virtual void initialSetup() override;

  /**
   * Method that reports whether the application has been fully solved or not.
   * Most transient multiapps are never fully solved, however this method can be
   * overridden in derived classes.
   */
  virtual bool isSolved() const { return false; }

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
   *
   * @return Whether or not all of the solves were successful (i.e. all solves made it to the
   * target_time)
   */
  virtual bool solveStep(Real dt, Real target_time, bool auto_advance = true) = 0;

  /**
   * Advances the multi-apps time step which is important for dt selection.
   * (Note this does not advance the *time*. That is done in Transient::endStep,
   * which is called either directly from solveStep() for loose coupling cases
   * or through finishStep() for Picard coupling cases)
   */
  virtual void incrementTStep() {}

  /**
   * Deprecated method. Use finishStep
   */
  virtual void advanceStep()
  {
    mooseDeprecated("advanceStep() is deprecated; please use finishStep() instead");
    finishStep();
  }

  /**
   * Calls multi-apps executioners' endStep and postStep methods which creates output and advances
   * time (not the time step; see incrementTStep()) among other things. This method is only called
   * for Picard calculations because for loosely coupled calculations the executioners' endStep and
   * postStep methods are called from solveStep().
   */
  virtual void finishStep() {}

  /**
   * Save off the state of every Sub App
   *
   * This allows us to "Restore" this state later
   */
  virtual void backup();

  /**
   * Restore the state of every Sub App
   *
   * This allows us to "Restore" this state later
   */
  virtual void restore();

  /**
   * Whether or not this MultiApp should be restored at the beginning of
   * each Picard iteration.
   */
  virtual bool needsRestoration() { return true; }

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
   * @param displaced_mesh True if the bounding box is retrieved for the displaced mesh, other false
   */
  virtual BoundingBox getBoundingBox(unsigned int app, bool displaced_mesh);

  /**
   * Get the FEProblemBase this MultiApp is part of.
   */
  FEProblemBase & problemBase() { return _fe_problem; }

  /**
   * Get the FEProblemBase for the global app is part of.
   * @param app The global app number
   */
  FEProblemBase & appProblemBase(unsigned int app);

  /**
   * Get the FEProblem for the global app is part of.
   * @param app The global app number
   */
  FEProblem & appProblem(unsigned int app);

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
  unsigned int numLocalApps() { return _apps.size(); }

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
   * Get the local MooseApp object
   * @param local_app The local app number
   */
  MooseApp * localApp(unsigned int local_app);

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
   * @param time The time to set as the the time for the new app, this should really be the time the
   * old app was at.
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
   * _must_ fill in _positions with the positions of the sub-aps
   */
  virtual void fillPositions();

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

  /// call back executed right before app->runInputFile()
  virtual void preRunInputFile();

  /**
   * Initialize the MultiApp by creating the provided number of apps.
   *
   * This is called in the constructor, by default it utilizes the 'positions' input parameters.
   */
  void init(unsigned int num);

  /// The FEProblemBase this MultiApp is part of
  FEProblemBase & _fe_problem;

  /// The type of application to build
  std::string _app_type;

  /// The positions of all of the apps
  std::vector<Point> _positions;

  /// Toggle use of "positions"
  const bool _use_positions;

  /// The input file for each app's simulation
  std::vector<FileName> _input_files;

  /// The output file basename for each multiapp
  std::string _output_base;

  /// The total number of apps to simulate
  unsigned int _total_num_apps;

  /// The number of apps this object is involved in simulating
  unsigned int _my_num_apps;

  /// The number of the first app on this processor
  unsigned int _first_local_app;

  /// The original comm handle
  const MPI_Comm & _orig_comm;

  /// The communicator object that holds the MPI_Comm that we're going to use
  libMesh::Parallel::Communicator _my_communicator;

  /// The MPI communicator this object is going to use.
  MPI_Comm & _my_comm;

  /// The number of processors in the original comm
  int _orig_num_procs;

  /// The mpi "rank" of this processor in the original communicator
  int _orig_rank;

  /// Node Name
  std::string _node_name;

  /// The mpi "rank" of this processor in the sub communicator
  int _my_rank;

  /// Pointers to each of the Apps
  std::vector<std::shared_ptr<MooseApp>> _apps;

  /// Flag if this multi-app computed its bounding box (valid only for non-displaced meshes)
  std::vector<bool> _has_bounding_box;

  /// This multi-app's bounding box
  std::vector<BoundingBox> _bounding_box;

  /// Relative bounding box inflation
  Real _inflation;

  /// Additional padding added to the bounding box, useful for 1D meshes
  Point _bounding_box_padding;

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

  /// Backups for each local App
  SubAppBackups & _backups;
};

template <>
inline void
dataStore(std::ostream & stream, SubAppBackups & backups, void * context)
{
  MultiApp * multi_app = static_cast<MultiApp *>(context);

  multi_app->backup();

  if (!multi_app)
    mooseError("Error storing std::vector<Backup*>");

  for (unsigned int i = 0; i < backups.size(); i++)
    dataStore(stream, backups[i], context);
}

template <>
inline void
dataLoad(std::istream & stream, SubAppBackups & backups, void * context)
{
  MultiApp * multi_app = static_cast<MultiApp *>(context);

  if (!multi_app)
    mooseError("Error loading std::vector<Backup*>");

  for (unsigned int i = 0; i < backups.size(); i++)
    dataLoad(stream, backups[i], context);

  multi_app->restore();
}

#endif // MULTIAPP_H
