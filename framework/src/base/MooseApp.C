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

// MOOSE includes
#include "MooseApp.h"
#include "MooseSyntax.h"
#include "MooseInit.h"
#include "Executioner.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"
#include "PetscSupport.h"
#include "Conversion.h"

// libMesh includes
#include "libmesh/mesh_refinement.h"
#include "libmesh/string_to_enum.h"

const unsigned short FIELD_WIDTH = 25;
const unsigned short LINE_LENGTH = 100;

template<>
InputParameters validParams<MooseApp>()
{
  InputParameters params;

  params.addCommandLineParam<std::string>("input_file", "-i <input_file>", "Specify an input file");
  params.addCommandLineParam<std::string>("mesh_only", "--mesh-only", "Setup and Output the input mesh only.");

  params.addCommandLineParam<bool>("show_input", "--show-input", "Shows the parsed input file before running the simulation.");
  params.addCommandLineParam<bool>("no_color", "--no-color", "Disable coloring of all Console outputs.");

  params.addCommandLineParam<bool>("help", "-h --help", "Displays CLI usage statement.");

  params.addCommandLineParam<std::string>("dump", "--dump [search_string]", "Shows a dump of available input file syntax.");
  params.addCommandLineParam<std::string>("yaml", "--yaml", "Dumps input file syntax in YAML format.");
  params.addCommandLineParam<bool>("syntax", "--syntax", "Dumps the associated Action syntax paths ONLY");

  params.addCommandLineParam<unsigned int>("n_threads", "--n-threads=<n>", 1, "Runs the specified number of threads (Intel TBB) per process");

  params.addCommandLineParam<bool>("warn_unused", "-w --warn-unused", "Warn about unused input file options");
  params.addCommandLineParam<bool>("error_unused", "-e --error-unused", "Error when encountering unused input file options");
  params.addCommandLineParam<bool>("error_override", "-o --error-override", "Error when encountering overridden or parameters supplied multiple times");

  params.addCommandLineParam<bool>("parallel_mesh", "--parallel-mesh", "The libMesh Mesh underlying MooseMesh should always be a ParallelMesh");

  params.addCommandLineParam<unsigned int>("refinements", "-r <n>", 0, "Specify additional initial uniform refinements for automatic scaling");

  params.addCommandLineParam<std::string>("recover", "--recover [file_base]", "Continue the calculation.  If file_base is omitted then the most recent recovery file will be utilized");

  params.addCommandLineParam<bool>("half_transient", "--half-transient", "When true the simulation will only run half of its specified transient (ie half the timesteps).  This is useful for testing recovery and restart");

  params.addCommandLineParam<bool>("trap_fpe", "--trap-fpe", "Enable Floating Point Exception handling in critical sections of code.  This is enabled automatically in DEBUG mode");

  params.addCommandLineParam<bool>("timing", "-t --timing", "Enable all performance logging for timing purposes. This will disable all screen output of performance logs for all Console objects.");

  params.addPrivateParam<int>("_argc");
  params.addPrivateParam<char**>("_argv");

  return params;
}

// Free function for stringstream formatting
void insertNewline(std::stringstream &oss, std::streampos &begin, std::streampos &curr)
{
  if (curr - begin > LINE_LENGTH)
  {
    oss << "\n";
    begin = oss.tellp();
    oss << std::setw(FIELD_WIDTH + 2) << "";  // "{ "
  }
}

MooseApp::MooseApp(const std::string & name, InputParameters parameters):
    ParallelObject(*parameters.get<Parallel::Communicator *>("_comm")), // Can't call getParam() before pars is set
    _name(name),
    _pars(parameters),
    _comm(getParam<Parallel::Communicator *>("_comm")),
    _output_position_set(false),
    _start_time_set(false),
    _start_time(0.0),
    _global_time_offset(0.0),
    _command_line(NULL),
    _action_factory(*this),
    _action_warehouse(*this, _syntax, _action_factory),
    _parser(*this, _action_warehouse),
    _executioner(NULL),
    _use_nonlinear(true),
    _sys_info(NULL),
    _enable_unused_check(WARN_UNUSED),
    _factory(*this),
    _error_overridden(false),
    _ready_to_exit(false),
    _initial_from_file(false),
    _parallel_mesh_on_command_line(false),
    _recover(false),
    _restart(false),
    _half_transient(false),
    _output_warehouse(new OutputWarehouse),
    _alternate_output_warehouse(NULL)
{
  if (isParamValid("_argc") && isParamValid("_argv"))
  {
    int argc = getParam<int>("_argc");
    char ** argv = getParam<char**>("_argv");

    _sys_info = new SystemInfo(argc, argv);
    _command_line = new CommandLine(argc, argv);
    _command_line->addCommandLineOptionsFromParams(_pars);
  }
}

MooseApp::~MooseApp()
{
  delete _command_line;
  delete _sys_info;
  delete _executioner;
  _action_warehouse.clear();

  // MUST be deleted before _comm is destroyed!
  delete _output_warehouse;

  // Note: Communicator MUST be destroyed last because everything else is using it!
  delete _comm;
}

void
MooseApp::setupOptions()
{
  if (isParamValid("error_unused"))
    setCheckUnusedFlag(true);
  else if (isParamValid("warn_unused"))
    setCheckUnusedFlag(false);

  if (isParamValid("error_override"))
    setErrorOverridden();

  if (isParamValid("parallel_mesh"))
    _parallel_mesh_on_command_line = true;

  if (isParamValid("half_transient"))
    _half_transient = true;

  if (isParamValid("no_color"))
    Moose::__color_console = false;

  // Set the timing parameter (see src/outputs/Console.C)
  if (isParamValid("timing"))
    _pars.set<bool>("timing") = true;
  else
    _pars.set<bool>("timing") = false;

  if (isParamValid("trap_fpe"))
    // Setting Global Variable
    Moose::__trap_fpe = true;

  if (isParamValid("help"))
  {
    _command_line->printUsage();
    _ready_to_exit = true;
  }
  else if (isParamValid("dump"))
  {
    _parser.initSyntaxFormatter(Parser::INPUT_FILE, true);

    // Get command line argument following --dump on command line
    std::string dump_following_arg = getParam<std::string>("dump");

    // If the argument following --dump is non-existent or begins with
    // a dash, call buildFullTree() with an empty string, otherwise
    // pass the argument following --dump.
    if (dump_following_arg.empty() || (dump_following_arg.find('-') == 0))
      _parser.buildFullTree("");
    else
      _parser.buildFullTree(dump_following_arg);

    _ready_to_exit = true;
  }
  else if (isParamValid("yaml"))
  {
    _parser.initSyntaxFormatter(Parser::YAML, true);

    // Get command line argument following --yaml on command line
    std::string yaml_following_arg = getParam<std::string>("yaml");

    // If the argument following --yaml is non-existent or begins with
    // a dash, call buildFullTree() with an empty string, otherwise
    // pass the argument following --yaml.
    if (yaml_following_arg.empty() || (yaml_following_arg.find('-') == 0))
      _parser.buildFullTree("");
    else
      _parser.buildFullTree(yaml_following_arg);

    _ready_to_exit = true;
  }
  else if (isParamValid("syntax"))
  {
    std::multimap<std::string, Syntax::ActionInfo> syntax = _syntax.getAssociatedActions();
    Moose::out << "**START SYNTAX DATA**\n";
    for (std::multimap<std::string, Syntax::ActionInfo>::iterator it = syntax.begin(); it != syntax.end(); ++it)
    {
      Moose::out << it->first << "\n";
    }
    Moose::out << "**END SYNTAX DATA**\n" << std::endl;
    _ready_to_exit = true;
  }
  else if (_input_filename != "") // They already specified an input filename
  {
    _parser.parse(_input_filename);
    _action_warehouse.build();
    return;
  }
  else if (isParamValid("input_file"))
  {
    if (isParamValid("recover"))
    {
      _recover = true;

      // Get command line argument following --recover on command line
      std::string recover_following_arg = getParam<std::string>("recover");

      // If the argument following --recover is non-existent or begins with
      // a dash then we are going to eventually find the newest recovery file to use
      if (!(recover_following_arg.empty() || (recover_following_arg.find('-') == 0)))
        _recover_base = recover_following_arg;
    }

    _input_filename = getParam<std::string>("input_file");
    _parser.parse(_input_filename);
    _action_warehouse.build();
  }
  else
  {
    _command_line->printUsage();
    _ready_to_exit = true;
  }
}

void
MooseApp::setInputFileName(std::string input_filename)
{
  _input_filename = input_filename;
}

std::string
MooseApp::getOutputFileBase()
{
  return _output_file_base;
}

void
MooseApp::runInputFile()
{
  std::string mesh_file_name;
  if (isParamValid("mesh_only"))
  {
    meshOnly(getParam<std::string>("mesh_only"));
    _ready_to_exit = true;
  }

  // If ready to exit has been set, then just return
  if (_ready_to_exit)
    return;

  _action_warehouse.executeAllActions();
  _executioner = _action_warehouse.executioner();

  // If requested, see if there are unidentified name/value pairs in the input file
  if (isParamValid("error_unused") || _enable_unused_check == ERROR_UNUSED)
  {
    std::vector<std::string> all_vars = _parser.getPotHandle()->get_variable_names();
    _parser.checkUnidentifiedParams(all_vars, true);
  }
  else if (isParamValid("warn_unused") || _enable_unused_check == WARN_UNUSED)
  {
    std::vector<std::string> all_vars = _parser.getPotHandle()->get_variable_names();
    _parser.checkUnidentifiedParams(all_vars, _enable_unused_check == ERROR_UNUSED);
  }

  if (isParamValid("error_override") || _error_overridden)
    _parser.checkOverriddenParams(true);
  else
    _parser.checkOverriddenParams(false);
}

void
MooseApp::executeExecutioner()
{
  // If ready to exit has been set, then just return
  if (_ready_to_exit)
    return;

  // run the simulation
  if (_executioner)
  {
#ifdef LIBMESH_HAVE_PETSC
    Moose::PetscSupport::petscSetupOutput(_command_line);
#endif
    _executioner->init();
    _executioner->execute();
  }
  else
    mooseError("No executioner was specified (go fix your input file)");
}

void
MooseApp::meshOnly(std::string mesh_file_name)
{
  /**
   * These actions should be the minimum set necessary to generate and output
   * a Mesh.
   */
  _action_warehouse.executeActionsWithAction("set_global_params");
  _action_warehouse.executeActionsWithAction("setup_mesh");
  _action_warehouse.executeActionsWithAction("prepare_mesh");
  _action_warehouse.executeActionsWithAction("add_mesh_modifier");
  _action_warehouse.executeActionsWithAction("setup_mesh_complete");

  // uniform refinement
  MooseMesh * mesh = _action_warehouse.mesh();
  MeshRefinement mesh_refinement(mesh->getMesh());
  mesh_refinement.uniformly_refine(mesh->uniformRefineLevel());

  // If no argument specified or if the argument following --mesh-only starts
  // with a dash, try to build an output filename based on the input mesh filename.
  if (mesh_file_name.empty() || (mesh_file_name.find('-') == 0))
  {
    mesh_file_name = _parser.getFileName();
    size_t pos = mesh_file_name.find_last_of('.');

    // Default to writing out an ExodusII mesh base on the input filename.
    mesh_file_name = mesh_file_name.substr(0, pos) + "_in.e";
  }

  // If we're writing an Exodus file, write the Mesh using its logical
  // element dimension rather than the spatial dimension, unless it's
  // a 1D Mesh.  One reason to prefer this approach is that sidesets
  // are displayed incorrectly for 2D triangular elements in both
  // Paraview and Cubit if num_dim==3 in the Exodus file. We do the
  // same thing in MOOSE's Exodus Output object, so we are mimicking
  // that behavior here.
  if (mesh_file_name.find(".e") + 2 == mesh_file_name.size())
  {
    ExodusII_IO exio(mesh->getMesh());
    if (mesh->getMesh().mesh_dimension() != 1)
      exio.use_mesh_dimension_instead_of_spatial_dimension(true);

    exio.write(mesh_file_name);
  }
  else
  {
    // Just write the file using the name requested by the user.
    mesh->getMesh().write(mesh_file_name);
  }

  // Since we are not going to create a problem the mesh
  // will not get cleaned up, so we'll do it here
  delete mesh;
  delete _action_warehouse.displacedMesh();
}

void
MooseApp::setCheckUnusedFlag(bool warn_is_error)
{
  _enable_unused_check = warn_is_error ? ERROR_UNUSED : WARN_UNUSED;
}

void
MooseApp::disableCheckUnusedFlag()
{
  _enable_unused_check = OFF;
}

void
MooseApp::setErrorOverridden()
{
  _error_overridden = true;
}

void
MooseApp::run()
{
  setupOptions();
  runInputFile();
  executeExecutioner();
}

void
MooseApp::setOutputPosition(Point p)
{

  _output_position_set = true;
  _output_position = p;
  _output_warehouse->meshChanged();

  if (_executioner != NULL)
    _executioner->parentOutputPositionChanged();
}

std::string
MooseApp::getFileName(bool stripLeadingPath) const
{
  return _parser.getFileName(stripLeadingPath);
}

OutputWarehouse &
MooseApp::getOutputWarehouse()
{
  if (_alternate_output_warehouse == NULL)
    return *_output_warehouse;
  else
    return *_alternate_output_warehouse;
}
