#include "TransientMultiApp.h"

#include "LayeredSideFluxAverage.h"

// libMesh
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<TransientMultiApp>()
{
  InputParameters params = validParams<MultiApp>();
  params += validParams<TransientInterface>();
  return params;
}


TransientMultiApp::TransientMultiApp(const std::string & name, InputParameters parameters):
    MultiApp(name, parameters),
    TransientInterface(parameters, name, "multiapps")
{
  MPI_Comm swapped = swapLibMeshComm(_my_comm);

  _transient_executioners.resize(_my_num_apps);
  // Grab Transient Executioners from each app
  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    MooseApp * app = _apps[i];
    Transient * ex = dynamic_cast<Transient *>(app->getExecutioner());
    if(!ex)
      mooseError("MultiApp " << name << " is not using a Transient Executioner!");
    ex->preExecute();
    _transient_executioners[i] = ex;
  }

  // Swap back
  swapLibMeshComm(swapped);
}

TransientMultiApp::~TransientMultiApp()
{
  MPI_Comm swapped = swapLibMeshComm(_my_comm);

  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    Transient * ex = _transient_executioners[i];
    ex->postExecute();
  }

  // Swap back
  swapLibMeshComm(swapped);
}

void
TransientMultiApp::solveStep()
{
  MPI_Comm swapped = swapLibMeshComm(_my_comm);

  int rank;
  MPI_Comm_rank(_orig_comm, &rank);

  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    Transient * ex = _transient_executioners[i];
    ex->takeStep(_dt);
    ex->endStep();
  }

  // Swap back
  swapLibMeshComm(swapped);
}

Real
TransientMultiApp::computeDT()
{
  MPI_Comm swapped = swapLibMeshComm(_my_comm);

  Real smallest_dt = std::numeric_limits<Real>::max();

  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    Transient * ex = _transient_executioners[i];
    Real dt = ex->computeConstrainedDT();

    smallest_dt = std::min(dt, smallest_dt);
  }

  // Swap back
  swapLibMeshComm(swapped);

  Parallel::min(smallest_dt);

  return smallest_dt;
}

