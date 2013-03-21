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
#include "Output.h"
#include "AppFactory.h"
#include "MooseUtils.h"

// libMesh
#include "libmesh/mesh_tools.h"

#include <iostream>
#include <iomanip>
#include <iterator>
#include <fstream>
#include <vector>
#include <algorithm>

template<>
InputParameters validParams<MultiApp>()
{
  InputParameters params = validParams<MooseObject>();

  params.addPrivateParam<bool>("use_displaced_mesh", false);

  std::ostringstream app_types_strings;

  registeredMooseAppIterator it = AppFactory::instance()->registeredObjectsBegin();
  while(it != AppFactory::instance()->registeredObjectsEnd())
  {
    app_types_strings << it->first;
    ++it;
    if(it != AppFactory::instance()->registeredObjectsEnd())
      app_types_strings<< ", ";
  }

  MooseEnum app_types_options(app_types_strings.str());

  params.addRequiredParam<MooseEnum>("app_type", app_types_options, "The type of application to build.");
  params.addParam<std::vector<Real> >("positions", "The positions of the App locations.  Each set of 3 values will represent a Point.  Either this must be supplied or 'positions_file'");
  params.addParam<FileName>("positions_file", "A filename that should be looked in for positions. Each set of 3 values in that file will represent a Point.  Either this must be supplied or 'positions'");

  params.addRequiredParam<std::vector<std::string> >("input_files", "The input file for each App.  If this parameter only contains one input file it will be used for all of the Apps.");

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
    _app_type(getParam<MooseEnum>("app_type")),
    _input_files(getParam<std::vector<std::string> >("input_files")),
    _orig_comm(getParam<MPI_Comm>("_mpi_comm")),
    _execute_on(getParam<MooseEnum>("execute_on"))
{
  if(isParamValid("positions"))
    _positions_vec = getParam<std::vector<Real> >("positions");
  else if(isParamValid("positions_file"))
  {
    // Read the file on the root processor then broadcast it
    if(libMesh::processor_id() == 0)
    {
      std::string positions_file = getParam<FileName>("positions_file");
      MooseUtils::checkFileReadable(positions_file);

      std::ifstream is(positions_file.c_str());
      std::istream_iterator<Real> begin(is), end;
      _positions_vec.insert(_positions_vec.begin(), begin, end);
    }
    unsigned int num_values = _positions_vec.size();

    Parallel::broadcast(num_values);

    _positions_vec.resize(num_values);

    Parallel::broadcast(_positions_vec);
  }
  else
    mooseError("Must supply either 'positions' or 'positions_file' for MultiApp "<<_name);

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
  buildComm();

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  _apps.resize(_my_num_apps);

  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    InputParameters app_params = AppFactory::instance()->getValidParams(_app_type);
    MooseApp * app = AppFactory::instance()->create(_app_type, "multi_app", app_params);

    std::ostringstream output_base;

    // Create an output base by taking the output base of the master problem and appending
    // the name of the multiapp + a number to it
    output_base << _fe_problem->out().fileBase()
                << "_" << _name
                << std::setw(std::ceil(std::log10(_total_num_apps)))
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
MultiApp::buildComm()
{
  MPI_Comm_size(_orig_comm, (int*)&_orig_num_procs);
  MPI_Comm_rank(_orig_comm, (int*)&_orig_rank);

  // If we have more apps than processors then we're just going to divide up the work
  if(_total_num_apps >= _orig_num_procs)
  {
    _my_comm = MPI_COMM_SELF;
    _my_rank = 0;

    _my_num_apps = _total_num_apps/_orig_num_procs;
    _first_local_app = _my_num_apps * _orig_rank;

    // The last processor will pick up any extra apps
    if(_orig_rank == _orig_num_procs - 1)
      _my_num_apps += _total_num_apps % _orig_num_procs;

    return;
  }

  // In this case we need to divide up the processors that are going to work on each app
  int rank;
  MPI_Comm_rank(_orig_comm, &rank);
//  sleep(rank);

  int procs_per_app = _orig_num_procs / _total_num_apps;
  int my_app = rank / procs_per_app;
  int procs_for_my_app = procs_per_app;

  if((unsigned int) my_app >= _total_num_apps-1) // The last app will gain any left-over procs
  {
    my_app = _total_num_apps - 1;
    procs_for_my_app += _orig_num_procs % _total_num_apps;
  }

  // Only one app here
  _first_local_app = my_app;
  _my_num_apps = 1;

  std::vector<int> ranks_in_my_group(procs_for_my_app);

  // Add all the processors in that are in my group
  for(int i=0; i<procs_for_my_app; i++)
    ranks_in_my_group[i] = (my_app * procs_per_app) + i;

  MPI_Group orig_group, new_group;

  // Extract the original group handle
  MPI_Comm_group(_orig_comm, &orig_group);

  // Create a group
  MPI_Group_incl(orig_group, procs_for_my_app, &ranks_in_my_group[0], &new_group);

  // Create new communicator
  MPI_Comm_create(_orig_comm, new_group, &_my_comm);
  MPI_Comm_rank(_my_comm, (int*)&_my_rank);
}

unsigned int
MultiApp::globalAppToLocal(unsigned int global_app)
{
  if(global_app >= _first_local_app && global_app <= _first_local_app + (_my_num_apps-1))
    return global_app-_first_local_app;

  std::cout<<_first_local_app<<" "<<global_app<<std::endl;
  mooseError("Invalid global_app!");
}

