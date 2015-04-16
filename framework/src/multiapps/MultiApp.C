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
#include "OutputWarehouse.h"
#include "AppFactory.h"
#include "MooseUtils.h"
#include "Console.h"
#include "InfixIterator.h"

// Regular expression includes
#include "pcrecpp.h"

// libMesh
#include "libmesh/mesh_tools.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <fstream>
#include <vector>
#include <algorithm>

#include <sys/utsname.h>
#include <dlfcn.h>

#define QUOTE(macro) stringifyName(macro)

template<>
InputParameters validParams<MultiApp>()
{
  InputParameters params = validParams<MooseObject>();

  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  std::ostringstream app_types_strings;
  registeredMooseAppIterator it = AppFactory::instance().registeredObjectsBegin();
  for ( ; it != AppFactory::instance().registeredObjectsEnd(); ++it)
    app_types_strings << it->first << " ";
  MooseEnum app_types_options(app_types_strings.str(), "", true);

  params.addRequiredParam<MooseEnum>("app_type", app_types_options, "The type of application to build (applications not registered can be loaded with dynamic libraries.");
  params.addParam<std::string>("library_path", "Path to search for dynamic libraries (please avoid committing absolute paths in addition to MOOSE_LIBRARY_PATH)");
  params.addParam<std::vector<Point> >("positions", "The positions of the App locations.  Each set of 3 values will represent a Point.  Either this must be supplied or 'positions_file'");
  params.addParam<FileName>("positions_file", "A filename that should be looked in for positions. Each set of 3 values in that file will represent a Point.  Either this must be supplied or 'positions'");

  params.addRequiredParam<std::vector<std::string> >("input_files", "The input file for each App.  If this parameter only contains one input file it will be used for all of the Apps.");
  params.addParam<Real>("bounding_box_inflation", 0.01, "Relative amount to 'inflate' the bounding box of this MultiApp.");

  params.addPrivateParam<MPI_Comm>("_mpi_comm");


  MultiMooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "timestep_begin";  // set the default

  params.addParam<MultiMooseEnum>("execute_on", execute_options, "Set to (linear|nonlinear|timestep_end|timestep_begin|custom) to execute only at that moment");

  params.addParam<unsigned int>("max_procs_per_app", std::numeric_limits<unsigned int>::max(), "Maximum number of processors to give to each App in this MultiApp.  Useful for restricting small solves to just a few procs so they don't get spread out");

  params.addParam<bool>("output_in_position", false, "If true this will cause the output from the MultiApp to be 'moved' by its position vector");

  params.addParam<Real>("reset_time", std::numeric_limits<Real>::max(), "The time at which to reset Apps given by the 'reset_apps' parameter.  Resetting an App means that it is destroyed and recreated, possibly modeling the insertion of 'new' material for that app.");

  params.addParam<std::vector<unsigned int> >("reset_apps", "The Apps that will be reset when 'reset_time' is hit.  These are the App 'numbers' starting with 0 corresponding to the order of the App positions.  Resetting an App means that it is destroyed and recreated, possibly modeling the insertion of 'new' material for that app.");

  params.addParam<Real>("move_time", std::numeric_limits<Real>::max(), "The time at which Apps designated by move_apps are moved to move_positions.");

  params.addParam<std::vector<unsigned int> >("move_apps", "Apps, designated by their 'numbers' starting with 0 corresponding to the order of the App positions, to be moved at move_time to move_positions");

  params.addParam<std::vector<Point> >("move_positions", "The positions corresponding to each move_app.");

  params.registerBase("MultiApp");

  return params;
}

MultiApp::MultiApp(const std::string & name, InputParameters parameters):
    MooseObject(name, parameters),
    SetupInterface(parameters),
    Restartable(parameters, "MultiApps"),
    _fe_problem(getParam<FEProblem *>("_fe_problem")),
    _app_type(getParam<MooseEnum>("app_type")),
    _input_files(getParam<std::vector<std::string> >("input_files")),
    _orig_comm(getParam<MPI_Comm>("_mpi_comm")),
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
}

MultiApp::~MultiApp()
{
  if (!_has_an_app)
    return;

  for (unsigned int i=0; i<_my_num_apps; i++)
  {
    MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);
    delete _apps[i];
    Moose::swapLibMeshComm(swapped);
  }

  // Close any open dynamic libraries
  for (std::map<std::string, void *>::iterator it = _lib_handles.begin(); it != _lib_handles.end(); ++it)
    dlclose(it->second);
}

void
MultiApp::initialSetup()
{
  // Fill in the _positions vector
  fillPositions();

  if (_move_apps.size() != _move_positions.size())
    mooseError("The number of apps to move and the positions to move them to must be the same for MultiApp "<<_name);

  _total_num_apps = _positions.size();
  mooseAssert(_input_files.size() == 1 || _positions.size() == _input_files.size(), "Number of positions and input files are not the same!");

  /// Set up our Comm and set the number of apps we're going to be working on
  buildComm();

  if (!_has_an_app)
    return;

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  _apps.resize(_my_num_apps);

  // If the user provided an unregistered app type, see if we can load it dynamically
  if (!AppFactory::instance().isRegistered(_app_type))
    dynamicRegisterApps(_app_type);

  for (unsigned int i=0; i<_my_num_apps; i++)
    createApp(i, _app.getGlobalTimeOffset());

  // Swap back
  Moose::swapLibMeshComm(swapped);
}

void
MultiApp::fillPositions()
{
  if (isParamValid("positions"))
    _positions = getParam<std::vector<Point> >("positions");
  else if (isParamValid("positions_file"))
  {
    // Read the file on the root processor then broadcast it
    if (processor_id() == 0)
    {
      std::string positions_file = getParam<FileName>("positions_file");
      MooseUtils::checkFileReadable(positions_file);

      std::ifstream is(positions_file.c_str());
      std::istream_iterator<Real> begin(is), end;
      _positions_vec.insert(_positions_vec.begin(), begin, end);
    }
    unsigned int num_values = _positions_vec.size();

    _communicator.broadcast(num_values);

    _positions_vec.resize(num_values);

    _communicator.broadcast(_positions_vec);

    mooseAssert(num_values % LIBMESH_DIM == 0, "Wrong number of entries in 'positions'");

    _positions.reserve(num_values / LIBMESH_DIM);

    for (unsigned int i = 0; i < num_values; i += 3)
      _positions.push_back(Point(_positions_vec[i], _positions_vec[i+1], _positions_vec[i+2]));

  }
  else
    mooseError("Must supply either 'positions' or 'positions_file' for MultiApp "<<_name);
}


void
MultiApp::preTransfer(Real /*dt*/, Real target_time)
{
  // First, see if any Apps need to be Reset
  if (!_reset_happened && target_time + 1e-14 >= _reset_time)
  {
    _reset_happened = true;
    for (unsigned int i=0; i<_reset_apps.size(); i++)
      resetApp(_reset_apps[i]);
  }

  // Now move any apps that should be moved
  if (!_move_happened && target_time + 1e-14 >= _move_time)
  {
    _move_happened = true;
    for (unsigned int i=0; i<_move_apps.size(); i++)
      moveApp(_move_apps[i], _move_positions[i]);
  }
}

Executioner *
MultiApp::getExecutioner(unsigned int app)
{
  if (!_has_an_app)
    mooseError("No app for " << _name << " on processor " << _orig_rank);

  return _apps[globalAppToLocal(app)]->getExecutioner();
}

MeshTools::BoundingBox
MultiApp::getBoundingBox(unsigned int app)
{
  if (!_has_an_app)
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
  if (problem->getCoordSystem(*(problem->mesh().meshSubdomains().begin())) == Moose::COORD_RZ)
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
  if (!_has_an_app)
    mooseError("No app for " << _name << " on processor " << _orig_rank);

  unsigned int local_app = globalAppToLocal(app);

  FEProblem * problem = dynamic_cast<FEProblem *>(&_apps[local_app]->getExecutioner()->problem());
  mooseAssert(problem, "Not an FEProblem!");

  return problem;
}

const UserObject &
MultiApp::appUserObjectBase(unsigned int app, const std::string & name)
{
  if (!_has_an_app)
    mooseError("No app for " << _name << " on processor " << _orig_rank);

  return appProblem(app)->getUserObjectBase(name);
}

Real
MultiApp::appPostprocessorValue(unsigned int app, const std::string & name)
{
  if (!_has_an_app)
    mooseError("No app for " << _name << " on processor " << _orig_rank);

  return appProblem(app)->getPostprocessorValue(name);
}

NumericVector<Number> &
MultiApp::appTransferVector(unsigned int app, std::string /*var_name*/)
{
  return appProblem(app)->getAuxiliarySystem().solution();
}

bool
MultiApp::hasLocalApp(unsigned int global_app)
{
  if (_has_an_app && global_app >= _first_local_app && global_app <= _first_local_app + (_my_num_apps-1))
    return true;

  return false;
}

void
MultiApp::resetApp(unsigned int global_app, Real time)
{
  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  if (hasLocalApp(global_app))
  {
    unsigned int local_app = globalAppToLocal(global_app);

    // Extract the file numbers from the output, so that the numbering is maintained after reset
    std::map<std::string, unsigned int> m = _apps[local_app]->getOutputWarehouse().getFileNumbers();

    // Delete and create a new App
    delete _apps[local_app];
    createApp(local_app, time);

    // Reset the file numbers of the newly reset apps
    _apps[local_app]->getOutputWarehouse().setFileNumbers(m);
  }

  // Swap back
  Moose::swapLibMeshComm(swapped);
}

void
MultiApp::moveApp(unsigned int global_app, Point p)
{

  _positions[global_app] = p;

  if (hasLocalApp(global_app))
  {
    unsigned int local_app = globalAppToLocal(global_app);

    if (_output_in_position)
      _apps[local_app]->setOutputPosition(p);
  }
}

void
MultiApp::parentOutputPositionChanged()
{
  if (_output_in_position)
    for (unsigned int i = 0; i < _apps.size(); i++)
      _apps[i]->setOutputPosition(_app.getOutputPosition() + _positions[_first_local_app + i]);
}

void
MultiApp::createApp(unsigned int i, Real start_time)
{

  // Define the app name
  std::ostringstream multiapp_name;
  std::string full_name;
  multiapp_name << _name <<  std::setw(std::ceil(std::log10(_total_num_apps)))
           << std::setprecision(0) << std::setfill('0') << std::right << _first_local_app + i;

  // Only add parent name if it the parent is not the main app
  if (_app.getOutputWarehouse().multiappLevel() > 0)
    full_name = _app.name() + "_" + multiapp_name.str();
  else
    full_name = multiapp_name.str();

  InputParameters app_params = AppFactory::instance().getValidParams(_app_type);
  app_params.set<FEProblem *>("_parent_fep") = _fe_problem;
  app_params.set<MooseSharedPointer<CommandLine> >("_command_line") = _app.commandLine();
  MooseApp * app = AppFactory::instance().create(_app_type, full_name, app_params, _my_comm);
  _apps[i] = app;

  std::string input_file = "";
  if (_input_files.size() == 1) // If only one input file was provided, use it for all the solves
    input_file = _input_files[0];
  else
    input_file = _input_files[_first_local_app+i];

  std::ostringstream output_base;

  // Create an output base by taking the output base of the master problem and appending
  // the name of the multiapp + a number to it
  if (!_app.getOutputFileBase().empty())
    output_base << _app.getOutputFileBase() + "_";
  else
  {
    std::string base = _app.getFileName();
    size_t pos = base.find_last_of('.');
    output_base << base.substr(0, pos) + "_out_";
  }

  // Append the sub app name to the output file base
  output_base << multiapp_name.str();
  app->setGlobalTimeOffset(start_time);
  app->setInputFileName(input_file);
  app->setOutputFileBase(output_base.str());
  app->setOutputFileNumbers(_app.getOutputWarehouse().getFileNumbers());

  if (getParam<bool>("output_in_position"))
    app->setOutputPosition(_app.getOutputPosition() + _positions[_first_local_app + i]);

  // Update the MultiApp level for the app that was just created
  app->getOutputWarehouse().multiappLevel() = _app.getOutputWarehouse().multiappLevel() + 1;
  app->setupOptions();
  app->runInputFile();
}

void
MultiApp::buildComm()
{
  int ierr;

  ierr = MPI_Comm_size(_orig_comm, &_orig_num_procs); mooseCheckMPIErr(ierr);
  ierr = MPI_Comm_rank(_orig_comm, &_orig_rank); mooseCheckMPIErr(ierr);

  struct utsname sysInfo;
  uname(&sysInfo);

  _node_name = sysInfo.nodename;

  // If we have more apps than processors then we're just going to divide up the work
  if (_total_num_apps >= (unsigned)_orig_num_procs)
  {
    _my_comm = MPI_COMM_SELF;
    _my_rank = 0;

    _my_num_apps = _total_num_apps/_orig_num_procs;
    unsigned int jobs_left = _total_num_apps - (_my_num_apps * _orig_num_procs);

    if (jobs_left != 0)
    {
      // Spread the remaining jobs out over the first set of processors
      if ((unsigned)_orig_rank < jobs_left)  // (these are the "jobs_left_pids" ie the pids that are snatching up extra jobs)
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

  unsigned int procs_per_app = _orig_num_procs / _total_num_apps;

  if (_max_procs_per_app < procs_per_app)
    procs_per_app = _max_procs_per_app;

  int my_app = rank / procs_per_app;
  unsigned int procs_for_my_app = procs_per_app;

  if ((unsigned int) my_app > _total_num_apps-1 && procs_for_my_app == _max_procs_per_app)
  {
    // If we've already hit the max number of procs per app then this processor
    // won't have an app at all
    _my_num_apps = 0;
    _has_an_app = false;
  }
  else if ((unsigned int) my_app >= _total_num_apps-1) // The last app will gain any left-over procs
  {
    my_app = _total_num_apps - 1;
//    procs_for_my_app += _orig_num_procs % _total_num_apps;
    _first_local_app = my_app;
    _my_num_apps = 1;
  }
  else
  {
    _first_local_app = my_app;
    _my_num_apps = 1;
  }

  if (_has_an_app)
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
  if (global_app >= _first_local_app && global_app <= _first_local_app + (_my_num_apps-1))
    return global_app-_first_local_app;

  _console << _first_local_app << " " << global_app << '\n';
  mooseError("Invalid global_app!");
}

void
MultiApp::dynamicRegisterApps(const std::string & app_name)
{
  // first convert the app name to a library name
  std::string library_name = appNameToLibName(app_name);

  // Create a vector of paths that we can search inside for libraries
  std::vector<std::string> paths;

  if (_pars.isParamValid("library_path"))
    MooseUtils::tokenize(_pars.get<std::string>("library_path"), paths, 1, ":");

  char * moose_lib_path_env = std::getenv("MOOSE_LIBRARY_PATH");
  if (moose_lib_path_env)
  {
    std::string moose_lib_path(moose_lib_path_env);
    std::vector<std::string> tmp_paths;

    MooseUtils::tokenize(moose_lib_path, tmp_paths, 1, ":");

    // merge the two vectors together (all possible search paths)
    paths.insert(paths.end(), tmp_paths.begin(), tmp_paths.end());
  }

  // Attempt to dynamically load the library
  for (std::vector<std::string>::const_iterator path_it = paths.begin(); path_it != paths.end(); ++path_it)
    if (MooseUtils::checkFileReadable(*path_it + '/' + library_name, false, false))
      loadLibraryAndDependencies(*path_it + '/' + library_name);

  // At this point the application should be registered so check it
  if (!AppFactory::instance().isRegistered(app_name))
  {
    std::ostringstream oss;

    oss << "Unable to locate library for \"" << app_name << "\".\nWe attempted to locate the library \"" << library_name << "\" in the following paths:\n\t";
    std::copy(paths.begin(), paths.end(), infix_ostream_iterator<std::string>(oss, "\n\t"));
    oss << "\n\nMake sure you have compiled the library and either set the \"library_path\" variable "
        << "in your input file or exported \"MOOSE_LIBRARY_PATH\".";
    mooseError(oss.str());
  }
}

std::string
MultiApp::appNameToLibName(const std::string & app_name) const
{
  std::string library_name(app_name);

  // Strip off the App part (should always be the last 3 letters of the name)
  size_t pos = library_name.find("App");
  if (pos != library_name.length() - 3)
    mooseError("Invalid application name: " << library_name);
  library_name.erase(pos);

  // Now get rid of the camel case, prepend lib, and append the method and suffix
  return std::string("lib") + MooseUtils::camelCaseToUnderscore(library_name) + '-' + QUOTE(METHOD) + ".la";
}

std::string
MultiApp::libNameToAppName(const std::string & library_name) const
{
  std::string app_name(library_name);

  // Strip off the leading "lib" and trailing ".la"
  if (pcrecpp::RE("lib(.+?)(?:-\\w+)?\\.la").Replace("\\1", &app_name) == 0)
    mooseError("Invalid library name: " << app_name);

  return MooseUtils::underscoreToCamelCase(app_name, true);
}

void
MultiApp::loadLibraryAndDependencies(const std::string & library_filename)
{
  std::string line;
  std::string dl_lib_filename;

  // This RE looks for absolute path libtool filenames (i.e. begins with a slash and ends with a .la)
  pcrecpp::RE re_deps("(/\\S*\\.la)");

  std::ifstream handle(library_filename.c_str());
  if (handle.is_open())
  {
    while (std::getline(handle, line))
    {
      // Look for the system dependent dynamic library filename to open
      if (line.find("dlname=") != std::string::npos)
        // Magic numbers are computed from length of this string "dlname=' and line minus that string plus quotes"
        dl_lib_filename = line.substr(8, line.size()-9);

      if (line.find("dependency_libs=") != std::string::npos)
      {
        pcrecpp::StringPiece input(line);
        pcrecpp::StringPiece depend_library;
        while (re_deps.FindAndConsume(&input, &depend_library))
          // Recurse here to load dependent libraries in depth-first order
          loadLibraryAndDependencies(depend_library.as_string());

        // There's only one line in the .la file containing the dependency libs so break after finding it
        break;
      }
    }
    handle.close();
  }

  // Time to load the library, First see if we've already loaded this particular dynamic library
  if (_lib_handles.find(library_filename) == _lib_handles.end() && // make sure we haven't already loaded this library
      dl_lib_filename != "")                                       // AND make sure we have a library name (we won't for static linkage)
  {
    std::pair<std::string, std::string> lib_name_parts = MooseUtils::splitFileName(library_filename);

    // Assemble the actual filename using the base path of the *.la file and the dl_lib_filename
    std::string dl_lib_full_path = lib_name_parts.first + '/' + dl_lib_filename;

    void * handle = dlopen(dl_lib_full_path.c_str(), RTLD_LAZY);
    if (!handle)
      mooseError("Cannot open library: " << dlerror());

    // Reset errors
    dlerror();

    // get the name of the unique registration method
    std::string reg_function = libNameToAppName(lib_name_parts.second) + "App__registerApps";

    // get the pointer to the method in the library
    void * registration_method = dlsym(handle, reg_function.c_str());

    // Get the pointer and cast it to function pointer with return type "void" taking no arguments
    typedef void (*register_app_t)();
    register_app_t *reg_ptr = reinterpret_cast<register_app_t *>( &registration_method );

    // Catch errors
    const char *dlsym_error = dlerror();
    if (dlsym_error)
      // We found a dynamic library that doesn't have registerApps() in it. This isn't an error so we'll just move on
      dlclose(handle);
    else
    {
      // Call the registration method
      (*reg_ptr)();

      // Store the handle so we can close it later
      _lib_handles.insert(std::make_pair(library_filename, handle));
    }
  }
}
