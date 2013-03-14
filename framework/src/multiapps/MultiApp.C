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
#include "MultiApp.h"

// Moose
#include "AppFactory.h"
#include "SetupInterface.h"
#include "Executioner.h"
#include "UserObject.h"
#include "FEProblem.h"

// libMesh
#include "libmesh/mesh_tools.h"

#include <iostream>
#include <iomanip>

template<>
InputParameters validParams<MultiApp>()
{
  InputParameters params = validParams<MooseObject>();

  params.addPrivateParam<bool>("use_displaced_mesh", false);

  params.addRequiredParam<std::string>("app_type", "The type of application to build.");
  params.addRequiredParam<std::vector<Real> >("positions", "The positions of the App locations.  Each set of 3 values will represent a Point.");
  params.addRequiredParam<std::vector<std::string> >("input_files", "The input file for each App.  If this parameter only contains one input file it will be used for all of the Apps.");
  params.addParam<std::string>("output_base", "multi_out", "This basename will have a number appended to it and will be used as the output file basename for the multiapps");

  params.addPrivateParam<MPI_Comm>("_mpi_comm");


  MooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "timestep_begin";  // set the default

  params.addParam<MooseEnum>("execute_on", execute_options, "Set to (residual|jacobian|timestep|timestep_begin|custom) to execute only at that moment");


  params.addPrivateParam<std::string>("built_by_action", "add_multi_app");

  return params;
}

MultiApp::MultiApp(const std::string & name, InputParameters parameters):
    MooseObject(name, parameters),
    _fe_problem(getParam<FEProblem *>("_fe_problem")),
    _app_type(getParam<std::string>("app_type")),
    _positions_vec(getParam<std::vector<Real> >("positions")),
    _input_files(getParam<std::vector<std::string> >("input_files")),
    _output_base(getParam<std::string>("output_base")),
    _orig_comm(getParam<MPI_Comm>("_mpi_comm")),
    _execute_on(getParam<MooseEnum>("execute_on"))
{
  { // Read the positions out of the vector
    unsigned int num_vec_entries = _positions_vec.size();

    mooseAssert(num_vec_entries % LIBMESH_DIM == 0, "Wrong number of entries in 'positions'");

    _positions.reserve(num_vec_entries / LIBMESH_DIM);

    for(unsigned int i=0; i<num_vec_entries; i+=3)
      _positions.push_back(Point(_positions_vec[i], _positions_vec[i+1], _positions_vec[i+2]));
  }

  _total_num_apps = _positions.size();
  mooseAssert(_input_files.size() == 1 || _positions.size() == _input_files.size(), "Number of positions and input files are not the same!");

  /// Set up our Comm and set the number of apps we're going to be working on
  buildComms();

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  _apps.resize(_my_num_apps);

  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    InputParameters app_params = AppFactory::instance()->getValidParams(_app_type);
    MooseApp * app = AppFactory::instance()->create(_app_type, "multi_app", app_params);

    std::ostringstream output_base;

    output_base << _output_base
                << std::setw(_total_num_apps/10)
                << std::setprecision(0)
                << std::setfill('0')
                << std::right
                << _first_local_app + i;

    _apps[i] = app;

    std::string input_file = "";
    if(_input_files.size() == 1) // If only one input file was provide, use it for all the solves
      input_file = _input_files[0];
    else
      input_file = _input_files[_first_local_app+i];

    app->setInputFileName(input_file);
    app->setOutputFileBase(output_base.str());
    app->setupOptions();
    app->runInputFile();
  }

  // Swap back
  Moose::swapLibMeshComm(swapped);
}

MultiApp::~MultiApp()
{
  for(unsigned int i=0; i<_my_num_apps; i++)
    delete _apps[i];
}

Executioner *
MultiApp::getExecutioner(unsigned int app)
{
  return _apps[globalAppToLocal(app)]->getExecutioner();
}

MeshTools::BoundingBox
MultiApp::getBoundingBox(unsigned int app)
{
  FEProblem * problem = appProblem(app);

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  MooseMesh & mesh = problem->mesh();
  MeshTools::BoundingBox bbox = MeshTools::bounding_box(mesh);

  Moose::swapLibMeshComm(swapped);

  return bbox;
}

FEProblem *
MultiApp::appProblem(unsigned int app)
{
  unsigned int local_app = globalAppToLocal(app);

  FEProblem * problem = dynamic_cast<FEProblem *>(&_apps[local_app]->getExecutioner()->problem());
  mooseAssert(problem, "Not an FEProblem!");

  return problem;
}

const UserObject &
MultiApp::appUserObjectBase(unsigned int app, const std::string & name)
{
  return appProblem(app)->getUserObjectBase(name);
}

Real
MultiApp::appPostprocessorValue(unsigned int app, const std::string & name)
{
  return appProblem(app)->getPostprocessorValue(name);
}

bool
MultiApp::hasLocalApp(unsigned int global_app)
{
  if(global_app >= _first_local_app && global_app <= _first_local_app + (_my_num_apps-1))
    return true;

  return false;
}

void
MultiApp::buildComms()
{
  // Whether or not we've found the first app that will be on this processor
  bool found_first = false;

  // Start with zero apps
  _my_num_apps = 0;

  _app_comms.resize(_total_num_apps);

  MPI_Comm_size(_orig_comm, (int*)&_orig_num_procs);

  MPI_Group orig_group;

  // Extract the original group handle
  MPI_Comm_group(_orig_comm, &orig_group);

  // Extract the original rank for the process
  MPI_Comm_rank(_orig_comm, (int*)&_orig_rank);

  // Loop over all the global apps and build a communicator for each one
  // It might not be obvious now, but there will be times when this is needed
  // such as in volumetric transfers
  for(unsigned int app=0; app < _total_num_apps; app++)
  {
    int num_procs_for_app;
    std::vector<int> ranks_for_app;

    // If we have more apps than processors then we're just going to divide up the work
    if(_total_num_apps >= _orig_num_procs)
    {
      num_procs_for_app = 1;

      unsigned int num_apps_per_proc = _total_num_apps/_orig_num_procs;
      unsigned int proc_for_app = app/num_apps_per_proc;

      // The last few apps all get assigned to the last processor
      if(proc_for_app >=_orig_num_procs)
        proc_for_app = _orig_num_procs - 1;

      ranks_for_app.resize(num_procs_for_app, proc_for_app);
    }
    else
    {
      // In this case we need to divide up the processors that are going to work on each app
      unsigned int procs_per_app = _orig_num_procs / _total_num_apps;
      unsigned int first_proc_for_app = app * procs_per_app;

      num_procs_for_app = procs_per_app;

      // The last app will snag any extra procs
      if(app == _total_num_apps-1)
        num_procs_for_app += _orig_num_procs % _total_num_apps;

      ranks_for_app.resize(num_procs_for_app);

      // Add all the processors in that are in this group
      for(int i=0; i<num_procs_for_app; i++)
        ranks_for_app[i] = first_proc_for_app + i;
    }

    MPI_Group app_group;
    MPI_Group_incl(orig_group, num_procs_for_app, &ranks_for_app[0], &app_group);

    MPI_Comm app_comm;
    MPI_Comm_create(_orig_comm, app_group, &app_comm);

    _app_comms[app] = app_comm;

    // If this processor was assigned to this app then hold onto it
    if(std::find(ranks_for_app.begin(), ranks_for_app.end(), _orig_rank) != ranks_for_app.end())
    {
      if(!found_first)
      {
        found_first = true;
        _first_local_app = app;
        _my_comm = app_comm; // All apps on this processor use the same comm
        MPI_Comm_rank(_my_comm, (int*)&_my_rank);
      }

      _my_num_apps++;
    }
  }
}

unsigned int
MultiApp::globalAppToLocal(unsigned int global_app)
{
  if(global_app >= _first_local_app && global_app <= _first_local_app + (_my_num_apps-1))
    return global_app-_first_local_app;

  std::cout<<_first_local_app<<" "<<global_app<<std::endl;
  mooseError("Invalid global_app!");
}

