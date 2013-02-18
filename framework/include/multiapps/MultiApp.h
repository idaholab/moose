#ifndef MULTIAPP_H
#define MULTIAPP_H

#include "MooseApp.h"

// libMesh includes
#include "libmesh/mesh_tools.h"

class MultiApp;

template<>
InputParameters validParams<MultiApp>();

/**
 * A MultiApp represents one or more MOOSE applications that are running simultaneously.
 * These other MOOSE apps generally represent some "sub-solve" or "embedded-solves"
 * of the overall nonlinear solve.
 */
class MultiApp : public MooseObject
{
public:
  MultiApp(const std::string & name, InputParameters parameters);

  virtual ~MultiApp();

  /**
   * Re-solve all of the Apps.
   */
  virtual void solveStep() = 0;

  /**
   * @param app The global app number to get the Executioner for
   * @return The Executioner associated with that App.
   */
  virtual Executioner * getExecutioner(unsigned int app);

  /**
   * Get the BoundingBox for the mesh associated with app
   * @param app The global app number you want to get the bounding box for
   */
  virtual MeshTools::BoundingBox getBoundingBox(unsigned int app);

protected:
  /**
   * Create an MPI communicator suitable for all of these apps.
   */
  void buildComm();

  /**
   * Swap the libMesh MPI communicator out for ours.
   */
  MPI_Comm swapLibMeshComm(MPI_Comm new_comm);

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

  /// The number of processors in the origal comm
  unsigned int _orig_num_procs;

  /// The mpi "rank" of this processor in the original communicator
  unsigned int _orig_pid;

  /// Pointers to each of the Apps
  std::vector<MooseApp *> _apps;
};

#endif // MULTIAPP_H
