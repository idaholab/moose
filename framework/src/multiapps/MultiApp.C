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

  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  std::ostringstream app_types_strings;

  registeredMooseAppIterator it = AppFactory::instance().registeredObjectsBegin();
  while(it != AppFactory::instance().registeredObjectsEnd())
  {
    app_types_strings << it->first;
    ++it;
    if(it != AppFactory::instance().registeredObjectsEnd())
      app_types_strings<< ", ";
  }

  MooseEnum app_types_options(app_types_strings.str());

  params.addRequiredParam<MooseEnum>("app_type", app_types_options, "The type of application to build.");
  params.addParam<std::vector<Point> >("positions", "The positions of the App locations.  Each set of 3 values will represent a Point.  Either this must be supplied or 'positions_file'");
  params.addParam<FileName>("positions_file", "A filename that should be looked in for positions. Each set of 3 values in that file will represent a Point.  Either this must be supplied or 'positions'");

  params.addRequiredParam<std::vector<std::string> >("input_files", "The input file for each App.  If this parameter only contains one input file it will be used for all of the Apps.");
  params.addParam<Real>("bounding_box_inflation", 0.01, "Relative amount to 'inflate' the bounding box of this MultiApp.");

  params.addPrivateParam<MPI_Comm>("_mpi_comm");


  MooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "timestep_begin";  // set the default

  params.addParam<MooseEnum>("execute_on", execute_options, "Set to (residual|jacobian|timestep|timestep_begin|custom) to execute only at that moment");

  params.addParam<unsigned int>("max_procs_per_app", std::numeric_limits<unsigned int>::max(), "Maximum number of processors to give to each App in this MultiApp.  Useful for restricting small solves to just a few procs so they don't get spread out");

  params.addParam<bool>("output_in_position", false, "If true this will cause the output from the MultiApp to be 'moved' by its position vector");

  params.addParam<Real>("reset_time", std::numeric_limits<Real>::max(), "The time at which to reset Apps given by the 'reset_apps' parameter.  Reseting an App means that it is destroyed and recreated, possibly modeling the insertion of 'new' material for that app.");

  params.addParam<std::vector<unsigned int> >("reset_apps", "The Apps that will be reset when 'reset_time' is hit.  These are the App 'numbers' starting with 0 corresponding to the order of the App positions.  Reseting an App means that it is destroyed and recreated, possibly modeling the insertion of 'new' material for that app.");

  params.addParam<Real>("move_time", std::numeric_limits<Real>::max(), "The time at which Apps designated by move_apps are moved to move_positions.");

  params.addParam<std::vector<unsigned int> >("move_apps", "Apps, designated by their 'numbers' starting with 0 corresponding to the order of the App positions, to be moved at move_time to move_positions");

  params.addParam<std::vector<Point> >("move_positions", "The positions corresponding to each move_app.");

  params.addPrivateParam<std::string>("built_by_action", "add_multi_app");

  return params;
}

MultiApp::MultiApp(const std::string & name, InputParameters parameters):
    MooseObject(name, parameters),
    _fe_problem(getParam<FEProblem *>("_fe_problem")),
    _app_type(getParam<MooseEnum>("app_type")),
    _input_files(getParam<std::vector<std::string> >("input_files")),
    _orig_comm(getParam<MPI_Comm>("_mpi_comm")),
    _execute_on(getParam<MooseEnum>("execute_on")),
    _inflation(getParam<Real>("bounding_box_inflation")),
    _max_procs_per_app(getParam<unsigned int>("max_procs_per_app")),
    _output_in_position(getParam<bool>("output_in_position")),
    _reset_time(getParam<Real>("reset_time")),
    _reset_apps(getParam<std::vector<unsigned int> >("reset_apps")),
    _reset_happened(false),
    _move_time(getParam<Real>("move_time")),
    _move_apps(getParam<std::vector<unsigned int> >("move_apps")),
    _move_positions(getParam<std::vector<Point> >("move_positions")),
    _move_happened(false),
    _has_an_app(true)
{
  if(isParamValid("positions"))
    _positions = getParam<std::vector<Point> >("positions");
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

    mooseAssert(num_values % LIBMESH_DIM == 0, "Wrong number of entries in 'positions'");

    _positions.reserve(num_values / LIBMESH_DIM);

    for(unsigned int i=0; i<num_values; i+=3)
      _positions.push_back(Point(_positions_vec[i], _positions_vec[i+1], _positions_vec[i+2]));
  }
  else
    mooseError("Must supply either 'positions' or 'positions_file' for MultiApp "<<_name);

  if(_move_apps.size() != _move_positions.size())
    mooseError("The number of apps to move and the positions to move them to must be the same for MultiApp "<<_name);

  _total_num_apps = _positions.size();
  mooseAssert(_input_files.size() == 1 || _positions.size() == _input_files.size(), "Number of positions and input files are not the same!");

  /// Set up our Comm and set the number of apps we're going to be working on
  buildComm();

  if(!_has_an_app)
    return;

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  _apps.resize(_my_num_apps);

  for(unsigned int i=0; i<_my_num_apps; i++)
    createApp(i);

  // Swap back
  Moose::swapLibMeshComm(swapped);
}

MultiApp::~MultiApp()
{
  if(!_has_an_app)
    return;

  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);
    delete _apps[i];
    Moose::swapLibMeshComm(swapped);
  }
}

void
MultiApp::preTransfer(Real dt, Real target_time)
{
  // First, see if any Apps need to be Reset
  if(!_reset_happened && target_time + 1e-14 >= _reset_time)
  {
    _reset_happened = true;
    for(unsigned int i=0; i<_reset_apps.size(); i++)
      resetApp(_reset_apps[i]);
  }

  // Now move any apps that should be moved
  if(!_move_happened && target_time + 1e-14 >= _move_time)
  {
    _move_happened = true;
    for(unsigned int i=0; i<_move_apps.size(); i++)
      moveApp(_move_apps[i], _move_positions[i]);
  }
}

Executioner *
MultiApp::getExecutioner(unsigned int app)
{
  if(!_has_an_app)
    mooseError("No app for " << _name << " on processor " << _orig_rank);

  return _apps[globalAppToLocal(app)]->getExecutioner();
}

MeshTools::BoundingBox
MultiApp::getBoundingBox(unsigned int app)
{
  if(!_has_an_app)
    mooseError("No app for " << _name << " on processor " << _orig_rank);

  FEProblem * problem = appProblem(app);

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  MooseMesh & mesh = problem->mesh();
  MeshTools::BoundingBox bbox = MeshTools::bounding_box(mesh);

  Moose::swapLibMeshComm(swapped);

  Point min = bbox.min();
  Point max = bbox.max();

  Point inflation_amount = (max-min)*_inflation;

  Point inflated_min = min - inflation_amount;
  Point inflated_max = max + inflation_amount;

  // This is where the app is located.  We need to shift by this amount.
  Point p = position(app);

  Point shifted_min = inflated_min;
  Point shifted_max = inflated_max;

  // If the problem is RZ then we're going to invent a box that would cover the whole "3D" app
  // FIXME: Assuming all subdomains are the same coordinate system type!
  if(problem->getCoordSystem(*(problem->mesh().meshSubdomains().begin())) == Moose::COORD_RZ)
  {
    shifted_min(0) = -inflated_max(0);
    shifted_min(1) = inflated_min(1);
    shifted_min(2) = -inflated_max(0);

    shifted_max(0) = inflated_max(0);
    shifted_max(1) = inflated_max(1);
    shifted_max(2) = inflated_max(0);
  }

  // Shift them to the position they're supposed to be
  shifted_min += p;
  shifted_max += p;

  return MeshTools::BoundingBox(shifted_min, shifted_max);
}

FEProblem *
MultiApp::appProblem(unsigned int app)
{
  if(!_has_an_app)
    mooseError("No app for " << _name << " on processor " << _orig_rank);

  unsigned int local_app = globalAppToLocal(app);

  FEProblem * problem = dynamic_cast<FEProblem *>(&_apps[local_app]->getExecutioner()->problem());
  mooseAssert(problem, "Not an FEProblem!");

  return problem;
}

const UserObject &
MultiApp::appUserObjectBase(unsigned int app, const std::string & name)
{
  if(!_has_an_app)
    mooseError("No app for " << _name << " on processor " << _orig_rank);

  return appProblem(app)->getUserObjectBase(name);
}

Real
MultiApp::appPostprocessorValue(unsigned int app, const std::string & name)
{
  if(!_has_an_app)
    mooseError("No app for " << _name << " on processor " << _orig_rank);

  return appProblem(app)->getPostprocessorValue(name);
}

NumericVector<Number> &
MultiApp::appTransferVector(unsigned int app, std::string var_name)
{
  return appProblem(app)->getAuxiliarySystem().solution();
}

bool
MultiApp::hasLocalApp(unsigned int global_app)
{
  if(_has_an_app && global_app >= _first_local_app && global_app <= _first_local_app + (_my_num_apps-1))
    return true;

  return false;
}

void
MultiApp::resetApp(unsigned int global_app)
{
  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  if(hasLocalApp(global_app))
  {
    unsigned int local_app = globalAppToLocal(global_app);
    delete _apps[local_app];
    createApp(local_app, 1); // The 1 is for the output file name
  }

  // Swap back
  Moose::swapLibMeshComm(swapped);
}

void
MultiApp::moveApp(unsigned int global_app, Point p)
{
  _positions[global_app] = p;

  if(hasLocalApp(global_app))
  {
    unsigned int local_app = globalAppToLocal(global_app);
    _apps[local_app]->setOutputPosition(p);
  }
}

void
MultiApp::parentOutputPositionChanged()
{
  if(getParam<bool>("output_in_position"))
  {
    Point parent_position = _app.getOutputPosition();

    for(unsigned int i=0; i<_apps.size(); i++)
    {
      MooseApp * app = _apps[i];
      app->setOutputPosition(parent_position + _positions[_first_local_app + i]);
    }
  }
}

void
MultiApp::createApp(unsigned int i, unsigned int output_sequence)
{
  InputParameters app_params = AppFactory::instance().getValidParams(_app_type);
  MooseApp * app = AppFactory::instance().create(_app_type, "multi_app", app_params);

  std::ostringstream output_base;

  // Create an output base by taking the output base of the master problem and appending
  // the name of the multiapp + a number to it
  if(_fe_problem)
    output_base << _fe_problem->out().fileBase() << "_" ;

  output_base << _name
              << std::setw(std::ceil(std::log10(_total_num_apps)))
              << std::setprecision(0)
              << std::setfill('0')
              << std::right
              << _first_local_app + i;

  if(output_sequence)
    output_base << "_" << output_sequence;

  _apps[i] = app;

  std::string input_file = "";
  if(_input_files.size() == 1) // If only one input file was provided, use it for all the solves
    input_file = _input_files[0];
  else
    input_file = _input_files[_first_local_app+i];

  app->setInputFileName(input_file);
  app->setOutputFileBase(output_base.str());

  if(getParam<bool>("output_in_position"))
  {
    Point parent_position = _app.getOutputPosition();
    app->setOutputPosition(parent_position + _positions[_first_local_app + i]);
  }

  app->setupOptions();
  app->runInputFile();
}


void
MultiApp::buildComm()
{
  int ierr;

  ierr = MPI_Comm_size(_orig_comm, &_orig_num_procs); mooseCheckMPIErr(ierr);
  ierr = MPI_Comm_rank(_orig_comm, &_orig_rank); mooseCheckMPIErr(ierr);

  // If we have more apps than processors then we're just going to divide up the work
  if(_total_num_apps >= _orig_num_procs)
  {
    _my_comm = MPI_COMM_SELF;
    _my_rank = 0;

    _my_num_apps = _total_num_apps/_orig_num_procs;
    unsigned int jobs_left = _total_num_apps - (_my_num_apps * _orig_num_procs);

    if(jobs_left != 0)
    {
      // Spread the remaining jobs out over the first set of processors
      if(_orig_rank < jobs_left)  // (these are the "jobs_left_pids" ie the pids that are snatching up extra jobs)
      {
        _my_num_apps += 1;
        _first_local_app = _my_num_apps * _orig_rank;
      }
      else
      {
        unsigned int num_apps_in_jobs_left_pids = (_my_num_apps + 1) * jobs_left;
        unsigned int distance_to_jobs_left_pids = _orig_rank - jobs_left;

        _first_local_app = num_apps_in_jobs_left_pids + (_my_num_apps * distance_to_jobs_left_pids);
      }
    }
    else
      _first_local_app = _my_num_apps * _orig_rank;

    return;
  }

  // In this case we need to divide up the processors that are going to work on each app
  int rank;
  ierr = MPI_Comm_rank(_orig_comm, &rank); mooseCheckMPIErr(ierr);
//  sleep(rank);

  unsigned int procs_per_app = _orig_num_procs / _total_num_apps;

  if(_max_procs_per_app < procs_per_app)
    procs_per_app = _max_procs_per_app;

  int my_app = rank / procs_per_app;
  unsigned int procs_for_my_app = procs_per_app;

  if((unsigned int) my_app > _total_num_apps-1 && procs_for_my_app == _max_procs_per_app)
  {
    // If we've already hit the max number of procs per app then this processor
    // won't have an app at all
    _my_num_apps = 0;
    _has_an_app = false;
  }
  else if((unsigned int) my_app >= _total_num_apps-1) // The last app will gain any left-over procs
  {
    my_app = _total_num_apps - 1;
    procs_for_my_app += _orig_num_procs % _total_num_apps;
    _first_local_app = my_app;
    _my_num_apps = 1;
  }
  else
  {
    _first_local_app = my_app;
    _my_num_apps = 1;
  }

  if(_has_an_app)
  {
    ierr = MPI_Comm_split(_orig_comm, _first_local_app, rank, &_my_comm); mooseCheckMPIErr(ierr);
    ierr = MPI_Comm_rank(_my_comm, &_my_rank); mooseCheckMPIErr(ierr);
  }
  else
  {
    ierr = MPI_Comm_split(_orig_comm, MPI_UNDEFINED, rank, &_my_comm); mooseCheckMPIErr(ierr);
    _my_rank = 0;
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

