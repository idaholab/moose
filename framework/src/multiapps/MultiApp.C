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
#include "NonlinearSystemBase.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/numeric_vector.h"

// C++ includes
#include <fstream>
#include <iomanip>
#include <iterator>
#include <algorithm>

// Call to "uname"
#ifdef LIBMESH_HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

defineLegacyParams(MultiApp);

InputParameters
MultiApp::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += SetupInterface::validParams();

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
  params.addParam<unsigned int>("min_procs_per_app",
                                1,
                                "Minimum number of processors to give to each App in this "
                                "MultiApp.  Useful for larger, distributed mesh solves.");

  params.addParam<bool>(
      "output_in_position",
      false,
      "If true this will cause the output from the MultiApp to be 'moved' by its position vector");

  params.addParam<Real>("global_time_offset",
                        0,
                        "The time offset relative to the master application for the purpose of "
                        "starting a subapp at different time from the master application. The "
                        "global time will be ahead by the offset specified here.");
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

  params.addParam<std::vector<std::string>>(
      "cli_args",
      std::vector<std::string>(),
      "Additional command line arguments to pass to the sub apps. If one set is provided the "
      "arguments are applied to all, otherwise there must be a set for each sub app.");

  params.addRangeCheckedParam<Real>("relaxation_factor",
                                    1.0,
                                    "relaxation_factor>0 & relaxation_factor<2",
                                    "Fraction of newly computed value to keep."
                                    "Set between 0 and 2.");
  params.addParam<std::vector<std::string>>("relaxed_variables",
                                            std::vector<std::string>(),
                                            "List of variables to relax during Picard Iteration");

  params.addParam<bool>(
      "clone_master_mesh", false, "True to clone master mesh and use it for this MultiApp.");

  params.addParam<bool>("keep_solution_during_restore",
                        false,
                        "This is useful when doing Picard.  It takes the "
                        "final solution from the previous Picard iteration"
                        "and re-uses it as the initial guess "
                        "for the next picard iteration");

  params.addPrivateParam<std::shared_ptr<CommandLine>>("_command_line");
  params.addPrivateParam<bool>("use_positions", true);
  params.declareControllable("enable");
  params.declareControllable("cli_args", {EXEC_PRE_MULTIAPP_SETUP});
  params.registerBase("MultiApp");

  return params;
}

MultiApp::MultiApp(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    Restartable(this, "MultiApps"),
    PerfGraphInterface(this),
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
    _min_procs_per_app(getParam<unsigned int>("min_procs_per_app")),
    _output_in_position(getParam<bool>("output_in_position")),
    _global_time_offset(getParam<Real>("global_time_offset")),
    _reset_time(getParam<Real>("reset_time")),
    _reset_apps(getParam<std::vector<unsigned int>>("reset_apps")),
    _reset_happened(false),
    _move_time(getParam<Real>("move_time")),
    _move_apps(getParam<std::vector<unsigned int>>("move_apps")),
    _move_positions(getParam<std::vector<Point>>("move_positions")),
    _move_happened(false),
    _has_an_app(true),
    _backups(declareRestartableDataWithContext<SubAppBackups>("backups", this)),
    _cli_args(getParam<std::vector<std::string>>("cli_args")),
    _keep_solution_during_restore(getParam<bool>("keep_solution_during_restore")),
    _perf_backup(registerTimedSection("backup", 3)),
    _perf_restore(registerTimedSection("restore", 3)),
    _perf_init(registerTimedSection("init", 3)),
    _perf_reset_app(registerTimedSection("resetApp", 3))
{
}

void
MultiApp::init(unsigned int num_apps, bool batch_mode)
{
  TIME_SECTION(_perf_init);

  _total_num_apps = num_apps;
  _rank_config = buildComm(batch_mode);
  _backups.reserve(_my_num_apps);
  for (unsigned int i = 0; i < _my_num_apps; i++)
    _backups.emplace_back(std::make_shared<Backup>());

  _has_bounding_box.resize(_my_num_apps, false);
  _bounding_box.resize(_my_num_apps);

  if ((_cli_args.size() > 1) && (_total_num_apps != _cli_args.size()))
    paramError("cli_args",
               "The number of items supplied must be 1 or equal to the number of sub apps.");
}

void
MultiApp::setupPositions()
{
  if (_use_positions)
  {
    fillPositions();
    init(_positions.size());
    createApps();
  }
}

void
MultiApp::createApps()
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
  {
    createApp(i, _global_time_offset);
    _app.parser().hitCLIFilter(_apps[i]->name(), _app.commandLine()->getArguments());
  }
}

void
MultiApp::initialSetup()
{
  if (!_use_positions)
    // if not using positions, we create the sub-apps in initialSetup instead of right after
    // construction of MultiApp
    createApps();
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
MultiApp::finalize()
{
  for (const auto & app_ptr : _apps)
  {
    auto * executioner = app_ptr->getExecutioner();
    mooseAssert(executioner, "Executioner is nullptr");

    executioner->feProblem().execute(EXEC_FINAL);
    executioner->feProblem().outputStep(EXEC_FINAL);
  }
}

void
MultiApp::postExecute()
{
  for (const auto & app_ptr : _apps)
  {
    auto * executioner = app_ptr->getExecutioner();
    mooseAssert(executioner, "Executioner is nullptr");

    executioner->postExecute();
  }
}

void
MultiApp::backup()
{
  TIME_SECTION(_perf_backup);

  _console << "Beginning backing up MultiApp " << name() << std::endl;
  for (unsigned int i = 0; i < _my_num_apps; i++)
    _backups[i] = _apps[i]->backup();
  _console << "Finished backing up MultiApp " << name() << std::endl;
}

void
MultiApp::restore(bool force)
{
  TIME_SECTION(_perf_restore);

  if (force || needsRestoration())
  {
    // Must be restarting / recovering from main app so hold off on restoring
    // Instead - the restore will happen in sub-apps' initialSetup()
    // Note that _backups was already populated by dataLoad() in the main app
    if (_fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL)
      return;

    // We temporarily copy and store solutions for all subapps
    if (_keep_solution_during_restore)
    {
      _end_solutions.resize(_my_num_apps);

      for (unsigned int i = 0; i < _my_num_apps; i++)
      {
        _end_solutions[i] =
            _apps[i]->getExecutioner()->feProblem().getNonlinearSystemBase().solution().clone();
        auto & sub_multiapps =
            _apps[i]->getExecutioner()->feProblem().getMultiAppWarehouse().getObjects();

        // multiapps of each subapp should do the same things
        // It is implemented recursively
        for (auto & multi_app : sub_multiapps)
          multi_app->keepSolutionDuringRestore(_keep_solution_during_restore);
      }
    }

    _console << "Begining restoring MultiApp " << name() << std::endl;
    for (unsigned int i = 0; i < _my_num_apps; i++)
      _apps[i]->restore(_backups[i]);
    _console << "Finished restoring MultiApp " << name() << std::endl;

    // Now copy the latest solutions back for each subapp
    if (_keep_solution_during_restore)
    {
      for (unsigned int i = 0; i < _my_num_apps; i++)
      {
        _apps[i]->getExecutioner()->feProblem().getNonlinearSystemBase().solution() =
            *_end_solutions[i];

        // We need to synchronize solution so that local_solution has the right values
        _apps[i]->getExecutioner()->feProblem().getNonlinearSystemBase().update();
      }

      _end_solutions.clear();
    }
  }
  else
  {
    for (unsigned int i = 0; i < _my_num_apps; i++)
    {
      for (auto & sub_app :
           _apps[i]->getExecutioner()->feProblem().getMultiAppWarehouse().getObjects())
        sub_app->restore(false);
    }
  }
}

void
MultiApp::keepSolutionDuringRestore(bool keep_solution_during_restore)
{
  if (_pars.isParamSetByUser("keep_solution_during_restore"))
    paramError("keep_solution_during_restore",
               "This parameter should be provided in only master app");

  _keep_solution_during_restore = keep_solution_during_restore;
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

  return appProblemBase(app).getPostprocessorValueByName(name);
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
  TIME_SECTION(_perf_reset_app);

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

  // Set the command line parameters with a copy of the main application command line parameters,
  // the copy is required so that the addArgument command below doesn't accumulate more and more
  // of the same cli_args, which is important when running in batch mode.
  std::shared_ptr<CommandLine> app_cli = std::make_shared<CommandLine>(*_app.commandLine());
  app_cli->initForMultiApp(full_name);
  app_params.set<std::shared_ptr<CommandLine>>("_command_line") = app_cli;

  if (_cli_args.size() > 0)
  {
    for (const std::string & str : MooseUtils::split(getCommandLineArgsParamHelper(i), ";"))
    {
      std::ostringstream oss;
      oss << full_name << ":" << str;
      app_params.get<std::shared_ptr<CommandLine>>("_command_line")->addArgument(oss.str());
    }
  }

  _console << COLOR_CYAN << "Creating MultiApp " << name() << " of type " << _app_type
           << " of level " << _app.multiAppLevel() + 1 << " and number " << _first_local_app + i
           << ":" << COLOR_DEFAULT << std::endl;
  app_params.set<unsigned int>("_multiapp_level") = _app.multiAppLevel() + 1;
  app_params.set<unsigned int>("_multiapp_number") = _first_local_app + i;
  if (getParam<bool>("clone_master_mesh"))
  {
    _console << COLOR_CYAN << "Cloned master mesh will be used for subapp " << name()
             << COLOR_DEFAULT << std::endl;
    app_params.set<const MooseMesh *>("_master_mesh") = &_fe_problem.mesh();
    auto displaced_problem = _fe_problem.getDisplacedProblem();
    if (displaced_problem)
      app_params.set<const MooseMesh *>("_master_displaced_mesh") = &displaced_problem->mesh();
  }
  _apps[i] = AppFactory::instance().createShared(_app_type, full_name, app_params, _my_comm);
  auto & app = _apps[i];

  std::string input_file = "";
  if (_input_files.size() == 1) // If only one input file was provided, use it for all the solves
    input_file = _input_files[0];
  else
    input_file = _input_files[_first_local_app + i];

  app->setGlobalTimeOffset(start_time);
  app->setInputFileName(input_file);
  app->setOutputFileNumbers(_app.getOutputWarehouse().getFileNumbers());
  app->setRestart(_app.isRestarting());
  app->setRecover(_app.isRecovering());

  // This means we have a backup of this app that we need to give to it
  // Note: This won't do the restoration immediately.  The Backup
  // will be cached by the MooseApp object so that it can be used
  // during FEProblemBase::initialSetup() during initialSetup()
  if (_app.isRestarting() || _app.isRecovering())
    app->setBackupObject(_backups[i]);

  if (_use_positions && getParam<bool>("output_in_position"))
    app->setOutputPosition(_app.getOutputPosition() + _positions[_first_local_app + i]);

  // Update the MultiApp level for the app that was just created
  app->setupOptions();
  // if multiapp does not have file base in Outputs input block, output file base will
  // be empty here since setupOptions() does not set the default file base with the multiapp
  // input file name. Master will create the default file base for multiapp by taking the
  // output base of the master problem and appending the name of the multiapp plus a number to it
  if (app->getOutputFileBase().empty())
    app->setOutputFileBase(_app.getOutputFileBase() + "_" + multiapp_name.str());
  preRunInputFile();
  app->runInputFile();

  auto & picard_solve = _apps[i]->getExecutioner()->picardSolve();
  picard_solve.setMultiAppRelaxationFactor(getParam<Real>("relaxation_factor"));
  picard_solve.setMultiAppRelaxationVariables(
      getParam<std::vector<std::string>>("relaxed_variables"));
  if (getParam<Real>("relaxation_factor") != 1.0)
  {
    // Store a copy of the previous solution here
    FEProblemBase & fe_problem_base = _apps[i]->getExecutioner()->feProblem();
    fe_problem_base.getNonlinearSystemBase().addVector("self_relax_previous", false, PARALLEL);
  }
}

std::string
MultiApp::getCommandLineArgsParamHelper(unsigned int local_app)
{

  // Single set of "cli_args" to be applied to all sub apps
  if (_cli_args.size() == 1)
    return _cli_args[0];

  // Unique set of "cli_args" to be applied to each sub apps
  else
    return _cli_args[local_app + _first_local_app];
}

LocalRankConfig
rankConfig(dof_id_type rank,
           dof_id_type nprocs,
           dof_id_type napps,
           dof_id_type min_app_procs,
           dof_id_type max_app_procs,
           bool batch_mode)
{
  if (min_app_procs > nprocs)
    mooseError("minimum number of procs per app is higher than the available number of procs");
  else if (min_app_procs > max_app_procs)
    mooseError("minimum number of procs per app must be lower than the max procs per app");

  mooseAssert(rank < nprocs, "rank must be smaller than the number of procs");

  // A "slot" is a group of procs/ranks that are grouped together to run a
  // single (sub)app/sim in parallel.

  auto slot_size = std::max(std::min(nprocs / napps, max_app_procs), min_app_procs);
  dof_id_type nslots = std::min(nprocs / slot_size, napps);
  auto leftover_procs = nprocs - nslots * slot_size;
  auto apps_per_slot = napps / nslots;
  auto leftover_apps = napps % nslots;

  std::vector<int> slot_for_rank(nprocs);
  dof_id_type slot = 0;
  dof_id_type procs_in_slot = 0;
  for (dof_id_type rankiter = 0; rankiter <= rank; rankiter++)
  {
    if (slot < nslots)
      slot_for_rank[rankiter] = slot;
    else
      slot_for_rank[rankiter] = -1;
    procs_in_slot++;
    // this slot keeps growing until we reach slot size plus possibly an extra
    // proc if there were any leftover from the slotization of nprocs - this
    // must also make sure we don't go over max app procs.
    if (procs_in_slot == slot_size + 1 * (slot < leftover_procs && slot_size < max_app_procs))
    {
      procs_in_slot = 0;
      slot++;
    }
  }

  if (slot_for_rank[rank] < 0)
    // ranks assigned a negative slot don't have any apps running on them.
    return {0, 0, 0, 0, false};
  dof_id_type slot_num = slot_for_rank[rank];

  bool is_first_local_rank = rank == 0 || (slot_for_rank[rank - 1] != slot_for_rank[rank]);
  auto n_local_apps = apps_per_slot + 1 * (slot_num < leftover_apps);

  dof_id_type app_index = 0;
  for (dof_id_type slot = 0; slot < slot_num; slot++)
  {
    auto num_slot_apps = apps_per_slot + 1 * (slot < leftover_apps);
    app_index += num_slot_apps;
  }

  if (batch_mode)
    return {n_local_apps, app_index, 1, slot_num, is_first_local_rank};
  return {n_local_apps, app_index, n_local_apps, app_index, is_first_local_rank};
}

LocalRankConfig
MultiApp::buildComm(bool batch_mode)
{
  int ierr;

  ierr = MPI_Comm_size(_communicator.get(), &_orig_num_procs);
  mooseCheckMPIErr(ierr);
  ierr = MPI_Comm_rank(_communicator.get(), &_orig_rank);
  mooseCheckMPIErr(ierr);

#ifdef LIBMESH_HAVE_SYS_UTSNAME_H
  struct utsname sysInfo;
  uname(&sysInfo);
  _node_name = sysInfo.nodename;
#else
  _node_name = "Unknown";
#endif

  int rank;
  ierr = MPI_Comm_rank(_communicator.get(), &rank);
  mooseCheckMPIErr(ierr);

  auto config = rankConfig(_orig_rank,
                           _orig_num_procs,
                           _total_num_apps,
                           _min_procs_per_app,
                           _max_procs_per_app,
                           batch_mode);
  _my_num_apps = config.num_local_apps;
  _first_local_app = config.first_local_app_index;

  _has_an_app = config.num_local_apps > 0;
  if (config.first_local_app_index >= _total_num_apps)
    mooseError("Internal error, a processor has an undefined app.");

  if (_has_an_app)
  {
    _communicator.split(config.first_local_app_index, rank, _my_communicator);
    ierr = MPI_Comm_rank(_my_comm, &_my_rank);
    mooseCheckMPIErr(ierr);
  }
  else
  {
    _communicator.split(MPI_UNDEFINED, rank, _my_communicator);
    _my_rank = 0;
  }
  return config;
}

unsigned int
MultiApp::globalAppToLocal(unsigned int global_app)
{
  if (global_app >= _first_local_app && global_app <= _first_local_app + (_my_num_apps - 1))
    return global_app - _first_local_app;

  std::stringstream ss;
  ss << "Requesting app " << global_app << ", but processor " << processor_id() << " ";
  if (_my_num_apps == 0)
    ss << "does not own any apps";
  else if (_my_num_apps == 1)
    ss << "owns app " << _first_local_app;
  else
    ss << "owns apps " << _first_local_app << "-" << _first_local_app + (_my_num_apps - 1);
  ss << ".";
  mooseError("Invalid global_app!\n", ss.str());
  return 0;
}

void
MultiApp::preRunInputFile()
{
}
