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
  buildComm();

  MPI_Comm swapped = swapLibMeshComm(_my_comm);

  _apps.resize(_my_num_apps);

  // Fake argv
  char *argv[4];

  argv[0]=(char*)"foo";
  argv[1]=(char*)"-i";
  argv[2]=(char*)"foo.i";
  argv[3]=(char*)"\0";

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
  swapLibMeshComm(swapped);
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
  unsigned int local_app = globalAppToLocal(app);

  FEProblem * problem = appProblem(app);

  MPI_Comm swapped = swapLibMeshComm(_my_comm);

  MooseMesh & mesh = problem->mesh();
  MeshTools::BoundingBox bbox = MeshTools::bounding_box(mesh);

  swapLibMeshComm(swapped);

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

void
MultiApp::buildComm()
{
  MPI_Comm_size(_orig_comm, (int*)&_orig_num_procs);
  MPI_Comm_rank(_orig_comm, (int*)&_orig_pid);

  // If we have more apps than processors then we're just going to divide up the work
  if(_total_num_apps >= _orig_num_procs)
  {
    _my_comm = MPI_COMM_SELF;

    _my_num_apps = _total_num_apps/_orig_num_procs;
    _first_local_app = _my_num_apps * _orig_pid;

    // The last processor will pick up any extra apps
    if(_orig_pid == _orig_num_procs - 1)
      _my_num_apps += _total_num_apps % _orig_num_procs;

    return;
  }

  // In this case we need to divide up the processors that are going to work on each app
  int rank, new_rank;
  MPI_Comm_rank(_orig_comm, &rank);
//  sleep(rank);

  int procs_per_app = _orig_num_procs / _total_num_apps;
  int my_app = rank / procs_per_app;
  int procs_for_my_app = procs_per_app;

  if(my_app >= _total_num_apps-1) // The last app will gain any left-over procs
  {
    my_app = _total_num_apps - 1;
    procs_for_my_app += _orig_num_procs % _total_num_apps;
  }

  // Only one app here
  _first_local_app = my_app;
  _my_num_apps = 1;

  std::vector<int> ranks_in_my_group(procs_for_my_app);

  // Add all the processors in that are in my group
  for(unsigned int i=0; i<procs_for_my_app; i++)
    ranks_in_my_group[i] = (my_app * procs_per_app) + i;

  MPI_Group orig_group, new_group;

  // Extract the original group handle
  MPI_Comm_group(_orig_comm, &orig_group);

  // Create a group
  MPI_Group_incl(orig_group, procs_for_my_app, &ranks_in_my_group[0], &new_group);

  // Create new communicator
  MPI_Comm_create(_orig_comm, new_group, &_my_comm);
}

MPI_Comm
MultiApp::swapLibMeshComm(MPI_Comm new_comm)
{
  MPI_Comm old_comm = libMesh::COMM_WORLD;
  libMesh::COMM_WORLD = new_comm;

  int pid;
  MPI_Comm_rank(new_comm, &pid);

  int n_procs;
  MPI_Comm_size(new_comm, &n_procs);

  libMesh::libMeshPrivateData::_processor_id = pid;
  libMesh::libMeshPrivateData::_n_processors = n_procs;

  Parallel::Communicator communicator(new_comm);

  libMesh::CommWorld = communicator;

  return old_comm;
}

unsigned int
MultiApp::globalAppToLocal(unsigned int global_app)
{
  if(global_app >= _first_local_app && global_app <= _first_local_app + (_my_num_apps-1))
    return global_app-_first_local_app;

  std::cout<<_first_local_app<<" "<<global_app<<std::endl;
  mooseError("Invalid global_app!");
}

