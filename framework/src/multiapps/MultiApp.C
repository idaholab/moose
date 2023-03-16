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
#include "DelimitedFileReader.h"
#include "MooseAppCoordTransform.h"
#include "MultiAppTransfer.h"
#include "Transient.h"

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
                             "registered can be loaded with dynamic libraries. Parent "
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
  params.addParam<bool>(
      "run_in_position",
      false,
      "If true this will cause the mesh from the MultiApp to be 'moved' by its position vector");

  params.addParam<Real>("global_time_offset",
                        0,
                        "The time offset relative to the parent application for the purpose of "
                        "starting a subapp at a different time from the parent application. The "
                        "global time will be ahead by the offset specified here.");
  params.addParam<std::vector<Real>>(
      "reset_time",
      std::vector<Real>(),
      "The time(s) at which to reset Apps given by the 'reset_apps' parameter.  "
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

  params.addParam<std::vector<FileName>>(
      "cli_args_files",
      "File names that should be looked in for additional command line arguments "
      "to pass to the sub apps. Each line of a file is set to each sub app. If only "
      "one line is provided, it will be applied to all sub apps.");

  params.addRangeCheckedParam<Real>("relaxation_factor",
                                    1.0,
                                    "relaxation_factor>0 & relaxation_factor<2",
                                    "Fraction of newly computed value to keep."
                                    "Set between 0 and 2.");
  params.addDeprecatedParam<std::vector<std::string>>(
      "relaxed_variables",
      std::vector<std::string>(),
      "Use transformed_variables.",
      "List of subapp variables to relax during Multiapp coupling iterations");
  params.addParam<std::vector<std::string>>(
      "transformed_variables",
      std::vector<std::string>(),
      "List of subapp variables to use coupling algorithm on during Multiapp coupling iterations");
  params.addParam<std::vector<PostprocessorName>>(
      "transformed_postprocessors",
      std::vector<PostprocessorName>(),
      "List of subapp postprocessors to use coupling "
      "algorithm on during Multiapp coupling iterations");

  params.addDeprecatedParam<bool>("clone_master_mesh",
                                  false,
                                  "True to clone parent app mesh and use it for this MultiApp.",
                                  "clone_master_mesh is deprecated, use clone_parent_mesh instead");
  params.addParam<bool>(
      "clone_parent_mesh", false, "True to clone parent app mesh and use it for this MultiApp.");

  params.addParam<bool>("keep_solution_during_restore",
                        false,
                        "This is useful when doing MultiApp coupling iterations. It takes the "
                        "final solution from the previous coupling iteration"
                        "and re-uses it as the initial guess "
                        "for the next coupling iteration");

  params.addPrivateParam<std::shared_ptr<CommandLine>>("_command_line");
  params.addPrivateParam<bool>("use_positions", true);
  params.declareControllable("enable");
  params.declareControllable("cli_args", {EXEC_PRE_MULTIAPP_SETUP});
  params.registerBase("MultiApp");

  params.addParamNamesToGroup("positions positions_file run_in_position output_in_position",
                              "Positions / transformations of the MultiApp frame of reference");
  params.addParamNamesToGroup("min_procs_per_app max_procs_per_app", "Parallelism");
  params.addParamNamesToGroup("reset_time reset_apps", "Reset MultiApp");
  params.addParamNamesToGroup("move_time move_apps move_positions", "Timed move of MultiApps");
  params.addParamNamesToGroup("relaxation_factor transformed_variables transformed_postprocessors "
                              "keep_solution_during_restore",
                              "Fixed point acceleration of MultiApp quantities");
  params.addParamNamesToGroup("library_name library_path", "Dynamic loading");
  params.addParamNamesToGroup("cli_args cli_args_files", "Passing command line argument");
  return params;
}

MultiApp::MultiApp(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    Restartable(this, "MultiApps"),
    PerfGraphInterface(this, std::string("MultiApp::") + _name),
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
    _reset_times(getParam<std::vector<Real>>("reset_time")),
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
    _run_in_position(getParam<bool>("run_in_position")),
    _solve_step_timer(registerTimedSection("solveStep", 3, "Executing MultiApps", false)),
    _init_timer(registerTimedSection("init", 3, "Initializing MultiApp")),
    _backup_timer(registerTimedSection("backup", 3, "Backing Up MultiApp")),
    _restore_timer(registerTimedSection("restore", 3, "Restoring MultiApp")),
    _reset_timer(registerTimedSection("resetApp", 3, "Resetting MultiApp"))
{
  if (parameters.isParamSetByUser("cli_args") && parameters.isParamValid("cli_args") &&
      parameters.isParamValid("cli_args_files"))
    paramError("cli_args",
               "'cli_args' and 'cli_args_files' cannot be specified simultaneously in MultiApp ");

  if ((_reset_apps.size() > 0 && _reset_times.size() == 0) ||
      (_reset_apps.size() == 0 && _reset_times.size() > 0))
    mooseError("reset_time and reset_apps may only be specified together");

  // Check that the reset times are sorted by the user
  auto sorted_times = _reset_times;
  std::sort(sorted_times.begin(), sorted_times.end());
  if (_reset_times.size() && _reset_times != sorted_times)
    paramError("reset_time", "List of reset times must be sorted in increasing order");
}

void
MultiApp::init(unsigned int num_apps, bool batch_mode)
{
  auto config = rankConfig(
      processor_id(), n_processors(), num_apps, _min_procs_per_app, _max_procs_per_app, batch_mode);
  init(num_apps, config);
}

void
MultiApp::init(unsigned int num_apps, const LocalRankConfig & config)
{
  TIME_SECTION(_init_timer);

  _total_num_apps = num_apps;
  _rank_config = config;
  buildComm();
  _backups.reserve(_my_num_apps);
  for (unsigned int i = 0; i < _my_num_apps; i++)
    _backups.emplace_back(std::make_shared<Backup>());

  _has_bounding_box.resize(_my_num_apps, false);
  _reset_happened.resize(_reset_times.size(), false);
  _bounding_box.resize(_my_num_apps);

  if ((_cli_args.size() > 1) && (_total_num_apps != _cli_args.size()))
    paramError("cli_args",
               "The number of items supplied must be 1 or equal to the number of sub apps.");

  // if cliArgs() != _cli_args, then cliArgs() was overridden and we need to check it
  auto cla = cliArgs();
  if (cla != _cli_args)
  {
    if ((cla.size() > 1) && (_total_num_apps != cla.size()))
      mooseError("The number of items supplied as command line argument to subapps must be 1 or "
                 "equal to the number of sub apps. Note: you use a multiapp that provides its own "
                 "command line parameters so the error is not in cli_args");
  }
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

  TIME_SECTION("createApps", 2, "Instantiating Sub-Apps", false);

  // Read commandLine arguments that will be used when creating apps
  readCommandLineArguments();

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
MultiApp::readCommandLineArguments()
{
  if (isParamValid("cli_args_files"))
  {
    _cli_args_from_file.clear();

    std::vector<FileName> cli_args_files = getParam<std::vector<FileName>>("cli_args_files");
    std::vector<FileName> input_files = getParam<std::vector<FileName>>("input_files");

    // If we use parameter "cli_args_files", at least one file should be provided
    if (!cli_args_files.size())
      paramError("cli_args_files", "You need to provide at least one commandLine argument file ");

    // If we multiple input files, then we need to check if the number of input files
    // match with the number of argument files
    if (cli_args_files.size() != 1 && cli_args_files.size() != input_files.size())
      paramError("cli_args_files",
                 "The number of commandLine argument files ",
                 cli_args_files.size(),
                 " for MultiApp ",
                 name(),
                 " must either be only one or match the number of input files ",
                 input_files.size());

    // Go through all argument files
    std::vector<std::string> cli_args;
    for (unsigned int p_file_it = 0; p_file_it < cli_args_files.size(); p_file_it++)
    {
      std::string cli_args_file = cli_args_files[p_file_it];
      // Clear up
      cli_args.clear();
      // Read the file on the root processor then broadcast it
      if (processor_id() == 0)
      {
        MooseUtils::checkFileReadable(cli_args_file);

        std::ifstream is(cli_args_file.c_str());
        std::copy(std::istream_iterator<std::string>(is),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(cli_args));

        // We do not allow empty files
        if (!cli_args.size())
          paramError("cli_args_files",
                     "There is no commandLine argument in the commandLine argument file ",
                     cli_args_file);

        // If we have position files, we need to
        // make sure the number of commandLine argument strings
        // match with the number of positions
        if (_npositions_inputfile.size())
        {
          auto num_positions = _npositions_inputfile[p_file_it];
          // Check if the number of commandLine argument strings equal to
          // the number of positions
          if (cli_args.size() == 1)
            for (MooseIndex(num_positions) num = 0; num < num_positions; num++)
              _cli_args_from_file.push_back(cli_args.front());
          else if (cli_args.size() == num_positions)
            for (auto && cli_arg : cli_args)
              _cli_args_from_file.push_back(cli_arg);
          else if (cli_args.size() != num_positions)
            paramError("cli_args_files",
                       "The number of commandLine argument strings ",
                       cli_args.size(),
                       " in the file ",
                       cli_args_file,
                       " must either be only one or match the number of positions ",
                       num_positions);
        }
        else
        {
          // If we do not have position files, we will check if the number of
          // commandLine argument strings match with the total number of subapps
          for (auto && cli_arg : cli_args)
            _cli_args_from_file.push_back(cli_arg);
        }
      }
    }

    // Broad cast all arguments to everyone
    _communicator.broadcast(_cli_args_from_file);
  }

  if (_cli_args_from_file.size() && _cli_args_from_file.size() != 1 &&
      _cli_args_from_file.size() != _total_num_apps)
    mooseError(" The number of commandLine argument strings ",
               _cli_args_from_file.size(),
               " must either be only one or match the total "
               "number of sub apps ",
               _total_num_apps);

  if (_cli_args_from_file.size() && cliArgs().size())
    mooseError("Cannot set commandLine arguments from both input_file and external files");
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
      MooseUtils::DelimitedFileReader file(positions_file, &_communicator);
      file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
      file.read();

      const std::vector<Point> & data = file.getDataAsPoints();
      for (const auto & d : data)
        _positions.push_back(d);

      // Save the number of positions for this input file
      _npositions_inputfile.push_back(data.size());

      for (unsigned int i = 0; i < data.size(); ++i)
        if (input_files.size() != 1)
          _input_files.push_back(input_files[p_file_it]);
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
  // Get a transient executioner to get a user-set tolerance
  Real timestep_tol = 1e-13;
  if (dynamic_cast<Transient *>(_fe_problem.getMooseApp().getExecutioner()))
    timestep_tol =
        dynamic_cast<Transient *>(_fe_problem.getMooseApp().getExecutioner())->timestepTol();

  // First, see if any Apps need to be reset
  for (unsigned int i = 0; i < _reset_times.size(); i++)
  {
    if (!_reset_happened[i] && (target_time + timestep_tol >= _reset_times[i]))
    {
      _reset_happened[i] = true;
      if (_reset_apps.size() > 0)
        for (auto & app : _reset_apps)
          resetApp(app);

      // If we reset an application, then we delete the old objects, including the coordinate
      // transformation classes. Consequently we need to reset the coordinate transformation classes
      // in the associated transfer classes
      for (auto * const transfer : _associated_transfers)
        transfer->getAppInfo();

      // Similarly we need to transform the mesh again
      if (_run_in_position)
        for (const auto i : make_range(_my_num_apps))
        {
          auto app_ptr = _apps[i];
          if (usingPositions())
            app_ptr->getExecutioner()->feProblem().coordTransform().transformMesh(
                app_ptr->getExecutioner()->feProblem().mesh(), _positions[_first_local_app + i]);
          else
            app_ptr->getExecutioner()->feProblem().coordTransform().transformMesh(
                app_ptr->getExecutioner()->feProblem().mesh(), Point(0, 0, 0));
        }

      // If the time step covers multiple reset times, set them all as having 'happened'
      for (unsigned int j = i; j < _reset_times.size(); j++)
        if (target_time + timestep_tol >= _reset_times[j])
          _reset_happened[j] = true;

      break;
    }
  }

  // Now move any apps that should be moved
  if (_use_positions && !_move_happened && target_time + timestep_tol >= _move_time)
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
  TIME_SECTION(_backup_timer);

  if (_fe_problem.verboseMultiApps())
    _console << "Backed up MultiApp ... ";

  for (unsigned int i = 0; i < _my_num_apps; i++)
    _backups[i] = _apps[i]->backup();

  if (_fe_problem.verboseMultiApps())
    _console << name() << std::endl;
}

void
MultiApp::restore(bool force)
{
  TIME_SECTION(_restore_timer);

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

    if (_fe_problem.verboseMultiApps())
      _console << "Restoring MultiApp ... ";

    for (unsigned int i = 0; i < _my_num_apps; i++)
      _apps[i]->restore(_backups[i]);

    if (_fe_problem.verboseMultiApps())
      _console << name() << std::endl;

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
               "This parameter should only be provided in parent app");

  _keep_solution_during_restore = keep_solution_during_restore;
}

void
MultiApp::transformBoundingBox(BoundingBox & box, const MultiAppCoordTransform & transform)
{
  const Real min_x = box.first(0);
  const Real max_x = box.second(0);
  const Real min_y = box.first(1);
  const Real max_y = box.second(1);
  const Real min_z = box.first(2);
  const Real max_z = box.second(2);

  std::array<Point, 8> box_corners = {{Point(min_x, min_y, min_z),
                                       Point(max_x, min_y, min_z),
                                       Point(min_x, max_y, min_z),
                                       Point(max_x, max_y, min_z),
                                       Point(min_x, min_y, max_z),
                                       Point(max_x, min_y, max_z),
                                       Point(min_x, max_y, max_z),
                                       Point(max_x, max_y, max_z)}};

  // transform each corner
  for (auto & corner : box_corners)
    corner = transform(corner);

  // Create new bounding box
  Point new_box_min = box_corners[0];
  Point new_box_max = new_box_min;
  for (const auto p : make_range(1, 8))
    for (const auto d : make_range(Moose::dim))
    {
      const Point & pt = box_corners[p];
      if (new_box_min(d) > pt(d))
        new_box_min(d) = pt(d);

      if (new_box_max(d) < pt(d))
        new_box_max(d) = pt(d);
    }
  box.first = new_box_min;
  box.second = new_box_max;
}

BoundingBox
MultiApp::getBoundingBox(unsigned int app,
                         bool displaced_mesh,
                         const MultiAppCoordTransform * const coord_transform)
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

  Point shifted_min = inflated_min;
  Point shifted_max = inflated_max;

  if ((!coord_transform || coord_transform->skipCoordinateCollapsing()) &&
      fe_problem_base.getCoordSystem(*(mesh.meshSubdomains().begin())) == Moose::COORD_RZ)
  {
    // If the problem is RZ then we're going to invent a box that would cover the whole "3D" app
    // FIXME: Assuming all subdomains are the same coordinate system type!
    shifted_min(0) = -inflated_max(0);
    shifted_min(1) = inflated_min(1);
    shifted_min(2) = -inflated_max(0);

    shifted_max(0) = inflated_max(0);
    shifted_max(1) = inflated_max(1);
    shifted_max(2) = inflated_max(0);
  }

  if (coord_transform)
  {
    BoundingBox transformed_bbox(shifted_min, shifted_max);
    transformBoundingBox(transformed_bbox, *coord_transform);
    return transformed_bbox;
  }
  else
  {
    // This is where the app is located.  We need to shift by this amount.
    Point p = position(app);

    // Shift them to the position they're supposed to be
    shifted_min += p;
    shifted_max += p;
    return BoundingBox(shifted_min, shifted_max);
  }
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
MultiApp::appTransferVector(unsigned int app, std::string var_name)
{
  return *(appProblemBase(app).getSystem(var_name).solution);
}

bool
MultiApp::hasLocalApp(unsigned int global_app) const
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
  TIME_SECTION(_reset_timer);

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
      if (_run_in_position)
        paramError("run_in_position", "Moving apps and running apps in position is not supported");
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
  const std::string multiapp_name = getMultiAppName(name(), _first_local_app + i, _total_num_apps);
  std::string full_name;

  // Only add parent name if the parent is not the main app
  if (_app.multiAppLevel() > 0)
    full_name = _app.name() + "_" + multiapp_name;
  else
    full_name = multiapp_name;

  InputParameters app_params = AppFactory::instance().getValidParams(_app_type);
  app_params.set<FEProblemBase *>("_parent_fep") = &_fe_problem;

  // Set the command line parameters with a copy of the main application command line parameters,
  // the copy is required so that the addArgument command below doesn't accumulate more and more
  // of the same cli_args, which is important when running in batch mode.
  std::shared_ptr<CommandLine> app_cli = std::make_shared<CommandLine>(*_app.commandLine());

  if (cliArgs().size() > 0 || _cli_args_from_file.size() > 0)
  {
    for (const std::string & str : MooseUtils::split(getCommandLineArgsParamHelper(i), ";"))
    {
      std::ostringstream oss;
      oss << full_name << ":" << str;
      app_cli->addArgument(oss.str());
    }
  }
  app_cli->initForMultiApp(full_name);
  app_params.set<std::shared_ptr<CommandLine>>("_command_line") = app_cli;

  if (_fe_problem.verboseMultiApps())
    _console << COLOR_CYAN << "Creating MultiApp " << name() << " of type " << _app_type
             << " of level " << _app.multiAppLevel() + 1 << " and number " << _first_local_app + i
             << " on processor " << processor_id() << " with full name " << full_name
             << COLOR_DEFAULT << std::endl;
  app_params.set<unsigned int>("_multiapp_level") = _app.multiAppLevel() + 1;
  app_params.set<unsigned int>("_multiapp_number") = _first_local_app + i;
  if (getParam<bool>("clone_master_mesh") || getParam<bool>("clone_parent_mesh"))
  {
    if (_fe_problem.verboseMultiApps())
      _console << COLOR_CYAN << "Cloned parent app mesh will be used for MultiApp " << name()
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
  if (_output_in_position && _run_in_position)
    paramError("run_in_position",
               "Sub-apps are already displaced, so they are already output in position");

  // Update the MultiApp level for the app that was just created
  app->setupOptions();
  // if multiapp does not have file base in Outputs input block, output file base will
  // be empty here since setupOptions() does not set the default file base with the multiapp
  // input file name. Parent app will create the default file base for multiapp by taking the
  // output base of the parent app problem and appending the name of the multiapp plus a number to
  // it
  if (app->getOutputFileBase().empty())
    setAppOutputFileBase(i);
  preRunInputFile();

  // Transfer coupling relaxation information to the subapps
  _apps[i]->fixedPointConfig().sub_relaxation_factor = getParam<Real>("relaxation_factor");
  _apps[i]->fixedPointConfig().sub_transformed_vars =
      getParam<std::vector<std::string>>("transformed_variables");
  // Handle deprecated parameter
  if (!parameters().isParamSetByAddParam("relaxed_variables"))
    _apps[i]->fixedPointConfig().sub_transformed_vars =
        getParam<std::vector<std::string>>("relaxed_variables");
  _apps[i]->fixedPointConfig().sub_transformed_pps =
      getParam<std::vector<PostprocessorName>>("transformed_postprocessors");

  app->runInputFile();
  auto fixed_point_solve = &(_apps[i]->getExecutioner()->fixedPointSolve());
  if (fixed_point_solve)
    fixed_point_solve->allocateStorage(false);

  // Transform the app mesh if requested
  if (_run_in_position)
  {
    if (usingPositions())
      app->getExecutioner()->feProblem().coordTransform().transformMesh(
          app->getExecutioner()->feProblem().mesh(), _positions[_first_local_app + i]);
    else
      app->getExecutioner()->feProblem().coordTransform().transformMesh(
          app->getExecutioner()->feProblem().mesh(), Point(0, 0, 0));
  }
}

std::string
MultiApp::getCommandLineArgsParamHelper(unsigned int local_app)
{
  auto cla = cliArgs();

  mooseAssert(cla.size() || _cli_args_from_file.size(), "There is no commandLine argument \n");

  // Single set of "cli_args" to be applied to all sub apps
  if (cla.size() == 1)
    return cla[0];
  else if (_cli_args_from_file.size() == 1)
    return _cli_args_from_file[0];
  else if (cla.size())
    // Unique set of "cli_args" to be applied to each sub apps
    return cla[local_app + _first_local_app];
  else
    return _cli_args_from_file[local_app + _first_local_app];
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

void
MultiApp::buildComm()
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

  _my_num_apps = _rank_config.num_local_apps;
  _first_local_app = _rank_config.first_local_app_index;

  _has_an_app = _rank_config.num_local_apps > 0;
  if (_rank_config.first_local_app_index >= _total_num_apps)
    mooseError("Internal error, a processor has an undefined app.");

  if (_has_an_app)
  {
    _communicator.split(_rank_config.first_local_app_index, rank, _my_communicator);
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

void
MultiApp::addAssociatedTransfer(MultiAppTransfer & transfer)
{
  _associated_transfers.push_back(&transfer);
}

void
MultiApp::setAppOutputFileBase()
{
  for (unsigned int i = 0; i < _my_num_apps; ++i)
    setAppOutputFileBase(i);
}

void
MultiApp::setAppOutputFileBase(unsigned int index)
{
  const std::string multiapp_name =
      getMultiAppName(name(), _first_local_app + index, _total_num_apps);
  _apps[index]->setOutputFileBase(_app.getOutputFileBase() + "_" + multiapp_name);
}

std::string
MultiApp::getMultiAppName(const std::string & base_name, dof_id_type index, dof_id_type total)
{
  std::ostringstream multiapp_name;
  multiapp_name << base_name << std::setw(std::ceil(std::log10(total))) << std::setprecision(0)
                << std::setfill('0') << std::right << index;
  return multiapp_name.str();
}
