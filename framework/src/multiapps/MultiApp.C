//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MultiApp.h"

#include "AppFactory.h"
#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"
#include "Console.h"
#include "Executioner.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "OutputWarehouse.h"
#include "RestartableDataIO.h"
#include "SetupInterface.h"
#include "UserObject.h"
#include "CommandLine.h"
#include "Conversion.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/numeric_vector.h"

// C++ includes
#include <fstream>
#include <iomanip>
#include <iterator>
#include <algorithm>

// Call to "uname"
#include <sys/utsname.h>

template <>
InputParameters
validParams<MultiApp>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<SetupInterface>();

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  std::ostringstream app_types_strings;
  registeredMooseAppIterator it = AppFactory::instance().registeredObjectsBegin();
  for (; it != AppFactory::instance().registeredObjectsEnd(); ++it)
    app_types_strings << it->first << " ";
  MooseEnum app_types_options(app_types_strings.str(), "", true);

  params.addParam<MooseEnum>("app_type",
                             app_types_options,
                             "The type of application to build (applications not "
                             "registered can be loaded with dynamic libraries. Master "
                             "application type will be used if not provided.");
  params.addParam<std::string>("library_path",
                               "",
                               "Path to search for dynamic libraries (please "
                               "avoid committing absolute paths in addition to "
                               "MOOSE_LIBRARY_PATH)");
  params.addParam<std::string>(
      "library_name",
      "",
      "The file name of the library (*.la file) that will be dynamically loaded.");
  params.addParam<std::vector<Point>>(
      "positions",
      "The positions of the App locations.  Each set of 3 values will represent a "
      "Point.  This and 'positions_file' cannot be both supplied. If this and "
      "'positions_file' are not supplied, a single position (0,0,0) will be used");
  params.addParam<std::vector<FileName>>("positions_file",
                                         "A filename that should be looked in for positions. Each "
                                         "set of 3 values in that file will represent a Point.  "
                                         "This and 'positions' cannot be both supplied");

  params.addRequiredParam<std::vector<FileName>>(
      "input_files",
      "The input file for each App.  If this parameter only contains one input file "
      "it will be used for all of the Apps.  When using 'positions_from_file' it is "
      "also admissable to provide one input_file per file.");
  params.addParam<Real>("bounding_box_inflation",
                        0.01,
                        "Relative amount to 'inflate' the bounding box of this MultiApp.");
  params.addParam<Point>("bounding_box_padding",
                         RealVectorValue(),
                         "Additional padding added to the dimensions of the bounding box. The "
                         "values are added to the x, y and z dimension respectively.");

  params.addPrivateParam<MPI_Comm>("_mpi_comm");

  // Set the default execution time
  params.set<ExecFlagEnum>("execute_on", true) = EXEC_TIMESTEP_BEGIN;

  params.addParam<unsigned int>("max_procs_per_app",
                                std::numeric_limits<unsigned int>::max(),
                                "Maximum number of processors to give to each App in this "
                                "MultiApp.  Useful for restricting small solves to just a few "
                                "procs so they don't get spread out");

  params.addParam<bool>(
      "output_in_position",
      false,
      "If true this will cause the output from the MultiApp to be 'moved' by its position vector");

  params.addParam<Real>("reset_time",
                        std::numeric_limits<Real>::max(),
                        "The time at which to reset Apps given by the 'reset_apps' parameter.  "
                        "Resetting an App means that it is destroyed and recreated, possibly "
                        "modeling the insertion of 'new' material for that app.");

  params.addParam<std::vector<unsigned int>>(
      "reset_apps",
      "The Apps that will be reset when 'reset_time' is hit.  These are the App "
      "'numbers' starting with 0 corresponding to the order of the App positions.  "
      "Resetting an App means that it is destroyed and recreated, possibly modeling "
      "the insertion of 'new' material for that app.");

  params.addParam<Real>(
      "move_time",
      std::numeric_limits<Real>::max(),
      "The time at which Apps designated by move_apps are moved to move_positions.");

  params.addParam<std::vector<unsigned int>>(
      "move_apps",
      "Apps, designated by their 'numbers' starting with 0 corresponding to the order "
      "of the App positions, to be moved at move_time to move_positions");

  params.addParam<std::vector<Point>>("move_positions",
                                      "The positions corresponding to each move_app.");

  params.addPrivateParam<std::shared_ptr<CommandLine>>("_command_line");
  params.addPrivateParam<bool>("use_positions", true);
  params.declareControllable("enable");
  params.registerBase("MultiApp");

  return params;
}

MultiApp::MultiApp(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    Restartable(this, "MultiApps"),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _app_type(isParamValid("app_type") ? std::string(getParam<MooseEnum>("app_type"))
                                       : _fe_problem.getMooseApp().type()),
    _use_positions(getParam<bool>("use_positions")),
    _input_files(getParam<std::vector<FileName>>("input_files")),
    _total_num_apps(0),
    _my_num_apps(0),
    _first_local_app(0),
    _orig_comm(_communicator.get()),
    _my_communicator(),
    _my_comm(_my_communicator.get()),
    _my_rank(0),
    _inflation(getParam<Real>("bounding_box_inflation")),
    _bounding_box_padding(getParam<Point>("bounding_box_padding")),
    _max_procs_per_app(getParam<unsigned int>("max_procs_per_app")),
    _output_in_position(getParam<bool>("output_in_position")),
    _reset_time(getParam<Real>("reset_time")),
    _reset_apps(getParam<std::vector<unsigned int>>("reset_apps")),
    _reset_happened(false),
    _move_time(getParam<Real>("move_time")),
    _move_apps(getParam<std::vector<unsigned int>>("move_apps")),
    _move_positions(getParam<std::vector<Point>>("move_positions")),
    _move_happened(false),
    _has_an_app(true),
    _backups(declareRestartableDataWithContext<SubAppBackups>("backups", this))
{
}

void
MultiApp::init(unsigned int num)
{
  _total_num_apps = num;
  buildComm();
  _backups.reserve(_my_num_apps);
  for (unsigned int i = 0; i < _my_num_apps; i++)
    _backups.emplace_back(std::make_shared<Backup>());

  _has_bounding_box.resize(_my_num_apps, false);
  _bounding_box.resize(_my_num_apps);
}

void
MultiApp::setupPositions()
{
  if (_use_positions)
  {
    fillPositions();
    init(_positions.size());
  }
}

void
MultiApp::initialSetup()
{
  if (!_has_an_app)
    return;

  Moose::ScopedCommSwapper swapper(_my_comm);

  _apps.resize(_my_num_apps);

  // If the user provided an unregistered app type, see if we can load it dynamically
  if (!AppFactory::instance().isRegistered(_app_type))
    _app.dynamicAppRegistration(
        _app_type, getParam<std::string>("library_path"), getParam<std::string>("library_name"));

  for (unsigned int i = 0; i < _my_num_apps; i++)
    createApp(i, _app.getGlobalTimeOffset());
}

void
MultiApp::fillPositions()
{
  if (_move_apps.size() != _move_positions.size())
    mooseError("The number of apps to move and the positions to move them to must be the same for "
               "MultiApp ",
               _name);

  if (isParamValid("positions") && isParamValid("positions_file"))
    mooseError(
        "Both 'positions' and 'positions_file' cannot be specified simultaneously in MultiApp ",
        name());

  if (isParamValid("positions"))
  {
    _positions = getParam<std::vector<Point>>("positions");

    if (_positions.size() < _input_files.size())
      mooseError("Not enough positions for the number of input files provided in MultiApp ",
                 name());
  }
  else if (isParamValid("positions_file"))
  {
    std::vector<FileName> positions_files = getParam<std::vector<FileName>>("positions_file");
    std::vector<FileName> input_files = getParam<std::vector<FileName>>("input_files");

    if (input_files.size() != 1 && positions_files.size() != input_files.size())
      mooseError("Number of input_files for MultiApp ",
                 name(),
                 " must either be only one or match the number of positions_file files");

    // Clear out the _input_files because we're going to rebuild it
    if (input_files.size() != 1)
      _input_files.clear();

    for (unsigned int p_file_it = 0; p_file_it < positions_files.size(); p_file_it++)
    {
      std::string positions_file = positions_files[p_file_it];

      std::vector<Real> positions_vec;

      // Read the file on the root processor then broadcast it
      if (processor_id() == 0)
      {
        MooseUtils::checkFileReadable(positions_file);

        std::ifstream is(positions_file.c_str());
        std::istream_iterator<Real> begin(is), end;
        positions_vec.insert(positions_vec.begin(), begin, end);

        if (positions_vec.size() % LIBMESH_DIM != 0)
          mooseError("Number of entries in 'positions_file' ",
                     positions_file,
                     " must be divisible by ",
                     LIBMESH_DIM,
                     " in MultiApp ",
                     name());
      }

      // Bradcast the vector to all processors
      std::size_t num_positions = positions_vec.size();
      _communicator.broadcast(num_positions);
      positions_vec.resize(num_positions);
      _communicator.broadcast(positions_vec);

      for (unsigned int i = 0; i < positions_vec.size(); i += LIBMESH_DIM)
      {
        if (input_files.size() != 1)
          _input_files.push_back(input_files[p_file_it]);

        Point position;

        // This is here so it will theoretically work with LIBMESH_DIM=1 or 2. That is completely
        // untested!
        for (unsigned int j = 0; j < LIBMESH_DIM; j++)
          position(j) = positions_vec[i + j];

        _positions.push_back(position);
      }
    }
  }
  else
  {
    _positions = {Point()};

    if (_positions.size() < _input_files.size())
      mooseError("Not enough positions for the number of input files provided in MultiApp ",
                 name());
  }

  mooseAssert(_input_files.size() == 1 || _positions.size() == _input_files.size(),
              "Number of positions and input files are not the same!");
}

void
MultiApp::preTransfer(Real /*dt*/, Real target_time)
{
  // First, see if any Apps need to be Reset
  if (!_reset_happened && target_time + 1e-14 >= _reset_time)
  {
    _reset_happened = true;
    for (auto & app : _reset_apps)
      resetApp(app);
  }

  // Now move any apps that should be moved
  if (_use_positions && !_move_happened && target_time + 1e-14 >= _move_time)
  {
    _move_happened = true;
    for (unsigned int i = 0; i < _move_apps.size(); i++)
      moveApp(_move_apps[i], _move_positions[i]);
  }
}

Executioner *
MultiApp::getExecutioner(unsigned int app)
{
  if (!_has_an_app)
    mooseError("No app for ", name(), " on processor ", _orig_rank);

  return _apps[globalAppToLocal(app)]->getExecutioner();
}

void
MultiApp::postExecute()
{
  for (const auto & app_ptr : _apps)
    app_ptr->getExecutioner()->postExecute();
}

void
MultiApp::backup()
{
  for (unsigned int i = 0; i < _my_num_apps; i++)
    _backups[i] = _apps[i]->backup();
}

void
MultiApp::restore()
{
  // Must be restarting / recovering so hold off on restoring
  // Instead - the restore will happen in createApp()
  // Note that _backups was already populated by dataLoad()
  if (_apps.empty())
    return;

  for (unsigned int i = 0; i < _my_num_apps; i++)
    _apps[i]->restore(_backups[i]);
}

BoundingBox
MultiApp::getBoundingBox(unsigned int app, bool displaced_mesh)
{
  if (!_has_an_app)
    mooseError("No app for ", name(), " on processor ", _orig_rank);

  unsigned int local_app = globalAppToLocal(app);
  FEProblemBase & fe_problem_base = _apps[local_app]->getExecutioner()->feProblem();
  MooseMesh & mesh = (displaced_mesh && fe_problem_base.getDisplacedProblem().get() != NULL)
                         ? fe_problem_base.getDisplacedProblem()->mesh()
                         : fe_problem_base.mesh();

  {
    Moose::ScopedCommSwapper swapper(_my_comm);
    if (displaced_mesh)
      _bounding_box[local_app] = MeshTools::create_bounding_box(mesh);
    else
    {
      if (!_has_bounding_box[local_app])
      {
        _bounding_box[local_app] = MeshTools::create_bounding_box(mesh);
        _has_bounding_box[local_app] = true;
      }
    }
  }
  BoundingBox bbox = _bounding_box[local_app];

  Point min = bbox.min();
  min -= _bounding_box_padding;
  Point max = bbox.max();
  max += _bounding_box_padding;

  Point inflation_amount = (max - min) * _inflation;

  Point inflated_min = min - inflation_amount;
  Point inflated_max = max + inflation_amount;

  // This is where the app is located.  We need to shift by this amount.
  Point p = position(app);

  Point shifted_min = inflated_min;
  Point shifted_max = inflated_max;

  // If the problem is RZ then we're going to invent a box that would cover the whole "3D" app
  // FIXME: Assuming all subdomains are the same coordinate system type!
  if (fe_problem_base.getCoordSystem(*(mesh.meshSubdomains().begin())) == Moose::COORD_RZ)
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

  return BoundingBox(shifted_min, shifted_max);
}

FEProblemBase &
MultiApp::appProblemBase(unsigned int app)
{
  if (!_has_an_app)
    mooseError("No app for ", name(), " on processor ", _orig_rank);

  unsigned int local_app = globalAppToLocal(app);

  return _apps[local_app]->getExecutioner()->feProblem();
}

FEProblem &
MultiApp::appProblem(unsigned int app)
{
  mooseDeprecated(
      "MultiApp::appProblem() is deprecated, call MultiApp::appProblemBase() instead.\n");
  if (!_has_an_app)
    mooseError("No app for ", name(), " on processor ", _orig_rank);

  unsigned int local_app = globalAppToLocal(app);

  return dynamic_cast<FEProblem &>(_apps[local_app]->getExecutioner()->feProblem());
}

const UserObject &
MultiApp::appUserObjectBase(unsigned int app, const std::string & name)
{
  if (!_has_an_app)
    mooseError("No app for ", MultiApp::name(), " on processor ", _orig_rank);

  return appProblemBase(app).getUserObjectBase(name);
}

Real
MultiApp::appPostprocessorValue(unsigned int app, const std::string & name)
{
  if (!_has_an_app)
    mooseError("No app for ", MultiApp::name(), " on processor ", _orig_rank);

  return appProblemBase(app).getPostprocessorValue(name);
}

NumericVector<Number> &
MultiApp::appTransferVector(unsigned int app, std::string /*var_name*/)
{
  return appProblemBase(app).getAuxiliarySystem().solution();
}

bool
MultiApp::hasLocalApp(unsigned int global_app)
{
  if (_has_an_app && global_app >= _first_local_app &&
      global_app <= _first_local_app + (_my_num_apps - 1))
    return true;

  return false;
}

MooseApp *
MultiApp::localApp(unsigned int local_app)
{
  mooseAssert(local_app < _apps.size(), "Index out of range: " + Moose::stringify(local_app));
  return _apps[local_app].get();
}

void
MultiApp::resetApp(unsigned int global_app, Real time)
{
  Moose::ScopedCommSwapper swapper(_my_comm);

  if (hasLocalApp(global_app))
  {
    unsigned int local_app = globalAppToLocal(global_app);

    // Extract the file numbers from the output, so that the numbering is maintained after reset
    std::map<std::string, unsigned int> m = _apps[local_app]->getOutputWarehouse().getFileNumbers();

    createApp(local_app, time);

    // Reset the file numbers of the newly reset apps
    _apps[local_app]->getOutputWarehouse().setFileNumbers(m);
  }
}

void
MultiApp::moveApp(unsigned int global_app, Point p)
{
  if (_use_positions)
  {
    _positions[global_app] = p;

    if (hasLocalApp(global_app))
    {
      unsigned int local_app = globalAppToLocal(global_app);

      if (_output_in_position)
        _apps[local_app]->setOutputPosition(p);
    }
  }
}

void
MultiApp::parentOutputPositionChanged()
{
  if (_use_positions && _output_in_position)
    for (unsigned int i = 0; i < _apps.size(); i++)
      _apps[i]->setOutputPosition(_app.getOutputPosition() + _positions[_first_local_app + i]);
}

void
MultiApp::createApp(unsigned int i, Real start_time)
{

  // Define the app name
  std::ostringstream multiapp_name;
  std::string full_name;
  multiapp_name << name() << std::setw(std::ceil(std::log10(_total_num_apps)))
                << std::setprecision(0) << std::setfill('0') << std::right << _first_local_app + i;

  // Only add parent name if it the parent is not the main app
  if (_app.multiAppLevel() > 0)
    full_name = _app.name() + "_" + multiapp_name.str();
  else
    full_name = multiapp_name.str();

  InputParameters app_params = AppFactory::instance().getValidParams(_app_type);
  app_params.set<FEProblemBase *>("_parent_fep") = &_fe_problem;
  app_params.set<std::shared_ptr<CommandLine>>("_command_line") = _app.commandLine();
  app_params.set<unsigned int>("_multiapp_level") = _app.multiAppLevel() + 1;
  app_params.set<unsigned int>("_multiapp_number") = _first_local_app + i;
  _apps[i] = AppFactory::instance().createShared(_app_type, full_name, app_params, _my_comm);
  auto & app = _apps[i];

  std::string input_file = "";
  if (_input_files.size() == 1) // If only one input file was provided, use it for all the solves
    input_file = _input_files[0];
  else
    input_file = _input_files[_first_local_app + i];

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
  app->setRestart(_app.isRestarting());
  app->setRecover(_app.isRecovering());

  // This means we have a backup of this app that we need to give to it
  // Note: This won't do the restoration immediately.  The Backup
  // will be cached by the MooseApp object so that it can be used
  // during FEProblemBase::initialSetup() during runInputFile()
  if (_app.isRestarting() || _app.isRecovering())
    app->restore(_backups[i]);

  if (_use_positions && getParam<bool>("output_in_position"))
    app->setOutputPosition(_app.getOutputPosition() + _positions[_first_local_app + i]);

  // Update the MultiApp level for the app that was just created
  app->setupOptions();
  preRunInputFile();
  app->runInputFile();
}

void
MultiApp::buildComm()
{
  int ierr;

  ierr = MPI_Comm_size(_communicator.get(), &_orig_num_procs);
  mooseCheckMPIErr(ierr);
  ierr = MPI_Comm_rank(_communicator.get(), &_orig_rank);
  mooseCheckMPIErr(ierr);

  struct utsname sysInfo;
  uname(&sysInfo);

  _node_name = sysInfo.nodename;

  // If we have more apps than processors then we're just going to divide up the work
  if (_total_num_apps >= (unsigned)_orig_num_procs)
  {
    _my_comm = MPI_COMM_SELF;
    _my_rank = 0;

    _my_num_apps = _total_num_apps / _orig_num_procs;
    unsigned int jobs_left = _total_num_apps - (_my_num_apps * _orig_num_procs);

    if (jobs_left != 0)
    {
      // Spread the remaining jobs out over the first set of processors
      if ((unsigned)_orig_rank < jobs_left) // (these are the "jobs_left_pids" ie the pids that are
                                            // snatching up extra jobs)
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
  ierr = MPI_Comm_rank(_communicator.get(), &rank);
  mooseCheckMPIErr(ierr);

  unsigned int procs_per_app = _orig_num_procs / _total_num_apps;

  if (_max_procs_per_app < procs_per_app)
    procs_per_app = _max_procs_per_app;

  int my_app = rank / procs_per_app;
  unsigned int procs_for_my_app = procs_per_app;

  if ((unsigned int)my_app > _total_num_apps - 1 && procs_for_my_app == _max_procs_per_app)
  {
    // If we've already hit the max number of procs per app then this processor
    // won't have an app at all
    _my_num_apps = 0;
    _has_an_app = false;
  }
  else if ((unsigned int)my_app >=
           _total_num_apps - 1) // The last app will gain any left-over procs
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
    _communicator.split(_first_local_app, rank, _my_communicator);

    ierr = MPI_Comm_rank(_my_comm, &_my_rank);
    mooseCheckMPIErr(ierr);
  }
  else
  {
    _communicator.split(MPI_UNDEFINED, rank, _my_communicator);

    _my_rank = 0;
  }
}

unsigned int
MultiApp::globalAppToLocal(unsigned int global_app)
{
  if (global_app >= _first_local_app && global_app <= _first_local_app + (_my_num_apps - 1))
    return global_app - _first_local_app;

  _console << _first_local_app << " " << global_app << '\n';
  mooseError("Invalid global_app!");
}

void
MultiApp::preRunInputFile()
{
}
