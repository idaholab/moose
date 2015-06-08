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
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "MooseInit.h"
#include "Executioner.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"
#include "PetscSupport.h"
#include "Conversion.h"
#include "CommandLine.h"
#include "InfixIterator.h"

// Regular expression includes
#include "pcrecpp.h"

// libMesh includes
#include "libmesh/mesh_refinement.h"
#include "libmesh/string_to_enum.h"

// System include for dynamic library methods
#include <dlfcn.h>

#define QUOTE(macro) stringifyName(macro)

template<>
InputParameters validParams<MooseApp>()
{
  InputParameters params;

  params.addCommandLineParam<std::string>("input_file", "-i <input_file>", "Specify an input file");
  params.addCommandLineParam<std::string>("mesh_only", "--mesh-only", "Setup and Output the input mesh only.");

  params.addCommandLineParam<bool>("show_input", "--show-input", false, "Shows the parsed input file before running the simulation.");
  params.addCommandLineParam<bool>("show_outputs", "--show-outputs", false, "Shows the output execution time information.");

  params.addCommandLineParam<bool>("no_color", "--no-color", false, "Disable coloring of all Console outputs.");

  params.addCommandLineParam<bool>("help", "-h --help", false, "Displays CLI usage statement.");

  params.addCommandLineParam<std::string>("dump", "--dump [search_string]", "Shows a dump of available input file syntax.");
  params.addCommandLineParam<std::string>("yaml", "--yaml", "Dumps input file syntax in YAML format.");
  params.addCommandLineParam<bool>("syntax", "--syntax", false, "Dumps the associated Action syntax paths ONLY");
  params.addCommandLineParam<bool>("check_input", "--check-input", false, "Check the input file (i.e. requires -i <filename>) and quit.");
  params.addCommandLineParam<bool>("list_constructed_objects", "--list-constructed-objects", false, "List all moose object type names constructed by the master app factory.");

  params.addCommandLineParam<unsigned int>("n_threads", "--n-threads=<n>", 1, "Runs the specified number of threads per process");

  params.addCommandLineParam<bool>("warn_unused", "-w --warn-unused", false, "Warn about unused input file options");
  params.addCommandLineParam<bool>("error_unused", "-e --error-unused", false, "Error when encountering unused input file options");
  params.addCommandLineParam<bool>("error_override", "-o --error-override", false, "Error when encountering overridden or parameters supplied multiple times");

  params.addCommandLineParam<bool>("parallel_mesh", "--parallel-mesh", false, "The libMesh Mesh underlying MooseMesh should always be a ParallelMesh");

  params.addCommandLineParam<unsigned int>("refinements", "-r <n>", 0, "Specify additional initial uniform refinements for automatic scaling");

  params.addCommandLineParam<std::string>("recover", "--recover [file_base]", "Continue the calculation.  If file_base is omitted then the most recent recovery file will be utilized");

  params.addCommandLineParam<bool>("half_transient", "--half-transient", false, "When true the simulation will only run half of its specified transient (ie half the timesteps).  This is useful for testing recovery and restart");

  // No default on these two options, they must not both be valid
  params.addCommandLineParam<bool>("trap_fpe", "--trap-fpe", "Enable Floating Point Exception handling in critical sections of code.  This is enabled automatically in DEBUG mode");
  params.addCommandLineParam<bool>("no_trap_fpe", "--no-trap-fpe", "Disable Floating Point Exception handling in critical sections of code when using DEBUG mode.");

  params.addCommandLineParam<bool>("error", "--error", false, "Turn all warnings into errors");

  params.addCommandLineParam<bool>("timing", "-t --timing", false, "Enable all performance logging for timing purposes. This will disable all screen output of performance logs for all Console objects.");

  // Legacy Flags
  params.addParam<bool>("use_legacy_uo_aux_computation", true, "Set to true to have MOOSE recompute *all* AuxKernel types every time *any* UserObject type is executed.\nThis behavoir is non-intuitive and will be removed late fall 2014, The default is controlled through MooseApp");
  params.addParam<bool>("use_legacy_uo_initialization", true, "Set to true to have MOOSE compute all UserObjects and Postprocessors during the initial setup phase of the problem recompute *all* AuxKernel types every time *any* UserObject type is executed.\nThis behavoir is non-intuitive and will be removed late fall 2014, The default is controlled through MooseApp");

  params.addPrivateParam<std::string>("_type");
  params.addPrivateParam<int>("_argc");
  params.addPrivateParam<char**>("_argv");
  params.addPrivateParam<MooseSharedPointer<CommandLine> >("_command_line");
  params.addPrivateParam<MooseSharedPointer<Parallel::Communicator> >("_comm");

  return params;
}

// Free function for removing cli flags
bool isFlag(const std::string s)
{
  return s.length() && s[0] == '-';
}

MooseApp::MooseApp(const std::string & name, InputParameters parameters) :
    ParallelObject(*parameters.get<MooseSharedPointer<Parallel::Communicator> >("_comm")), // Can't call getParam() before pars is set
    _name(name),
    _pars(parameters),
    _type(getParam<std::string>("_type")),
    _comm(getParam<MooseSharedPointer<Parallel::Communicator> >("_comm")),
    _output_position_set(false),
    _start_time_set(false),
    _start_time(0.0),
    _global_time_offset(0.0),
    _alternate_output_warehouse(NULL),
    _output_warehouse(new OutputWarehouse),
    _action_factory(*this),
    _action_warehouse(*this, _syntax, _action_factory),
    _parser(*this, _action_warehouse),
    _use_nonlinear(true),
    _enable_unused_check(WARN_UNUSED),
    _factory(*this),
    _error_overridden(false),
    _ready_to_exit(false),
    _initial_from_file(false),
    _parallel_mesh_on_command_line(false),
    _recover(false),
    _restart(false),
    _half_transient(false),
    _legacy_uo_aux_computation_default(getParam<bool>("use_legacy_uo_aux_computation")),
    _legacy_uo_initialization_default(getParam<bool>("use_legacy_uo_initialization")),
    _check_input(getParam<bool>("check_input"))
{
  if (isParamValid("_argc") && isParamValid("_argv"))
  {
    int argc = getParam<int>("_argc");
    char ** argv = getParam<char**>("_argv");

    _sys_info = MooseSharedPointer<SystemInfo>(new SystemInfo(argc, argv));
  }
  if (isParamValid("_command_line"))
    _command_line = getParam<MooseSharedPointer<CommandLine> >("_command_line");
  else
    mooseError("Valid CommandLine object required");
}

MooseApp::~MooseApp()
{
  _action_warehouse.clear();

  // MUST be deleted before _comm is destroyed!
  delete _output_warehouse;

#ifdef LIBMESH_HAVE_DLOPEN
  // Close any open dynamic libraries
  for (std::map<std::pair<std::string, std::string>, void *>::iterator it = _lib_handles.begin(); it != _lib_handles.end(); ++it)
    dlclose(it->second);
#endif
}

void
MooseApp::setupOptions()
{
  if (getParam<bool>("error_unused"))
    setCheckUnusedFlag(true);
  else if (getParam<bool>("warn_unused"))
    setCheckUnusedFlag(false);

  if (getParam<bool>("error_override"))
    setErrorOverridden();

  _parallel_mesh_on_command_line = getParam<bool>("parallel_mesh");
  _half_transient = getParam<bool>("half_transient");
  _pars.set<bool>("timing") = getParam<bool>("timing");

  if (isParamValid("trap_fpe") && isParamValid("no_trap_fpe"))
    mooseError("Cannot use both \"--trap-fpe\" and \"--no-trap-fpe\" flags.");
  if (isParamValid("trap_fpe"))
    Moose::_trap_fpe = true;
  else if (isParamValid("no_trap_fpe"))
    Moose::_trap_fpe = false;

  Moose::_warnings_are_errors = getParam<bool>("error");
  Moose::_color_console = !getParam<bool>("no_color");

  if (getParam<bool>("help"))
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
  else if (getParam<bool>("syntax"))
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
  else if (_input_filename != "" || isParamValid("input_file")) // They already specified an input filename
  {
    if (_input_filename == "")
      _input_filename = getParam<std::string>("input_file");

    if (isParamValid("recover"))
    {
      // We need to set the flag manually here since the recover parameter is a string type (takes an optional filename)
      _recover = true;

      // Get command line argument following --recover on command line
      std::string recover_following_arg = getParam<std::string>("recover");

      // If the argument following --recover is non-existent or begins with
      // a dash then we are going to eventually find the newest recovery file to use
      if (!(recover_following_arg.empty() || (recover_following_arg.find('-') == 0)))
        _recover_base = recover_following_arg;
    }

    _parser.parse(_input_filename);
    _action_warehouse.build();
  }
  else
  {
    if (_check_input)
      mooseError("You specified --check-input, but did not provide an input file. Add -i <inputfile> to your command line.");

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

  if (getParam<bool>("list_constructed_objects"))
  {
    // TODO: ask multiapps for their constructed objects
    std::vector<std::string> obj_list = _factory.getConstructedObjects();
    Moose::out << "**START OBJECT DATA**\n";
    for (unsigned int i = 0; i < obj_list.size(); ++i)
    {
      Moose::out << obj_list[i] << "\n";
    }
    Moose::out << "**END OBJECT DATA**\n" << std::endl;
    _ready_to_exit = true;
    return;
  }


  bool error_unused = getParam<bool>("error_unused") || _enable_unused_check == ERROR_UNUSED;
  bool warn_unused = getParam<bool>("warn_unused") || _enable_unused_check == WARN_UNUSED;

  if (error_unused || warn_unused)
  {
    // Check the input file parameters
    std::vector<std::string> all_vars = _parser.getPotHandle()->get_variable_names();
    _parser.checkUnidentifiedParams(all_vars, error_unused, true);

    // Only check CLI parameters on the main application
    // TODO: Add support for SubApp overrides and checks #4119
    if (_name == "main")
    {
      // Check the CLI parameters
      all_vars = _command_line->getPot()->get_variable_names();
      // Remove flags, they aren't "input" parameters
      all_vars.erase( std::remove_if(all_vars.begin(), all_vars.end(), isFlag), all_vars.end() );

      _parser.checkUnidentifiedParams(all_vars, error_unused, false);
    }
  }

  if (getParam<bool>("error_override") || _error_overridden)
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
    Moose::PetscSupport::petscSetupOutput(_command_line.get());
#endif
    _executioner->init();
    if (_check_input)
    {
      // Output to stderr, so it is easier for peacock to get the result
      Moose::err << "Syntax OK" << std::endl;
      return;
    }
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
  _action_warehouse.executeActionsWithAction("uniform_refine_mesh");
  _action_warehouse.executeActionsWithAction("setup_mesh_complete");

  MooseSharedPointer<MooseMesh> & mesh = _action_warehouse.mesh();

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

bool &
MooseApp::legacyUoAuxComputationDefault()
{
  return _legacy_uo_aux_computation_default;
}

bool &
MooseApp::legacyUoInitializationDefault()
{
  return _legacy_uo_initialization_default;
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

  if (_executioner.get() != NULL)
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

std::string
MooseApp::appNameToLibName(const std::string & app_name) const
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
MooseApp::libNameToAppName(const std::string & library_name) const
{
  std::string app_name(library_name);

  // Strip off the leading "lib" and trailing ".la"
  if (pcrecpp::RE("lib(.+?)(?:-\\w+)?\\.la").Replace("\\1", &app_name) == 0)
    mooseError("Invalid library name: " << app_name);

  return MooseUtils::underscoreToCamelCase(app_name, true);
}

void
MooseApp::dynamicAppRegistration(const std::string & app_name, std::string library_path)
{
  Parameters params;
  params.set<std::string>("app_name") = app_name;
  params.set<RegistrationType>("reg_type") = APPLICATION;
  params.set<std::string>("registration_method") = app_name + "__registerApps";
  params.set<std::string>("library_path") = library_path;

  dynamicRegistration(params);

  // At this point the application should be registered so check it
  if (!AppFactory::instance().isRegistered(app_name))
  {
    std::ostringstream oss;
    std::set<std::string> paths = getLoadedLibraryPaths();

    oss << "Unable to locate library for \"" << app_name << "\".\nWe attempted to locate the library \"" << appNameToLibName(app_name)
        << "\" in the following paths:\n\t";
    std::copy(paths.begin(), paths.end(), infix_ostream_iterator<std::string>(oss, "\n\t"));
    oss << "\n\nMake sure you have compiled the library and either set the \"library_path\" variable "
        << "in your input file or exported \"MOOSE_LIBRARY_PATH\".\n"
        << "Compiled in debug mode to see the list of libraries checked for dynamic loading methods.";
    mooseError(oss.str());
  }
}

void
MooseApp::dynamicObjectRegistration(const std::string & app_name, Factory * factory, std::string library_path)
{
  Parameters params;
  params.set<std::string>("app_name") = app_name;
  params.set<RegistrationType>("reg_type") = OBJECT;
  params.set<std::string>("registration_method") = app_name + "__registerObjects";
  params.set<std::string>("library_path") = library_path;

  params.set<Factory *>("factory") = factory;

  dynamicRegistration(params);
}

void
MooseApp::dynamicSyntaxAssociation(const std::string & app_name, Syntax * syntax, ActionFactory * action_factory, std::string library_path)
{
  Parameters params;
  params.set<std::string>("app_name") = app_name;
  params.set<RegistrationType>("reg_type") = SYNTAX;
  params.set<std::string>("registration_method") = app_name + "__associateSyntax";
  params.set<std::string>("library_path") = library_path;

  params.set<Syntax *>("syntax") = syntax;
  params.set<ActionFactory *>("action_factory") = action_factory;

  dynamicRegistration(params);
}

void
MooseApp::dynamicRegistration(const Parameters & params)
{
  // first convert the app name to a library name
  std::string library_name = appNameToLibName(params.get<std::string>("app_name"));

  // Create a vector of paths that we can search inside for libraries
  std::vector<std::string> paths;

  std::string library_path = params.get<std::string>("library_path");

  if (library_path != "")
    MooseUtils::tokenize(library_path, paths, 1, ":");

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
      loadLibraryAndDependencies(*path_it + '/' + library_name, params);
    else
      mooseWarning("Unable to open library file \"" << *path_it + '/' + library_name << "\". Double check for spelling errors.");
}

void
MooseApp::loadLibraryAndDependencies(const std::string & library_filename, const Parameters & params)
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
          loadLibraryAndDependencies(depend_library.as_string(), params);

        // There's only one line in the .la file containing the dependency libs so break after finding it
        break;
      }
    }
    handle.close();
  }

  std::string registration_method_name = params.get<std::string>("registration_method");
  // Time to load the library, First see if we've already loaded this particular dynamic library
  if (_lib_handles.find(std::make_pair(library_filename, registration_method_name)) == _lib_handles.end() && // make sure we haven't already loaded this library
      dl_lib_filename != "")                                                                                 // AND make sure we have a library name (we won't for static linkage)
  {
    std::pair<std::string, std::string> lib_name_parts = MooseUtils::splitFileName(library_filename);

    // Assemble the actual filename using the base path of the *.la file and the dl_lib_filename
    std::string dl_lib_full_path = lib_name_parts.first + '/' + dl_lib_filename;

#ifdef LIBMESH_HAVE_DLOPEN
    void * handle = dlopen(dl_lib_full_path.c_str(), RTLD_LAZY);
#else
    void * handle = NULL;
#endif

    if (!handle)
      mooseError("Cannot open library: " << dl_lib_full_path.c_str() << "\n");

    // get the pointer to the method in the library.  The dlsym()
    // function returns a null pointer if the symbol cannot be found,
    // we also explicitly set the pointer to NULL if dlsym is not
    // available.
#ifdef LIBMESH_HAVE_DLOPEN
    void * registration_method = dlsym(handle, registration_method_name.c_str());
#else
    void * registration_method = NULL;
#endif

    if (!registration_method)
    {
      // We found a dynamic library that doesn't have a dynamic
      // registration method in it. This shouldn't be an error, so
      // we'll just move on.
#ifdef DEBUG
      mooseWarning("Unable to find extern \"C\" method \"" << registration_method_name \
                   << "\" in library: " << dl_lib_full_path << ".\n" \
                   << "This doesn't necessarily indicate an error condition unless you believe that the method should exist in that library.\n");
#endif

#ifdef LIBMESH_HAVE_DLOPEN
      dlclose(handle);
#endif
    }
    else // registration_method is valid!
    {
      // TODO: Look into cleaning this up
      switch (params.get<RegistrationType>("reg_type"))
      {
      case APPLICATION:
      {
        typedef void (*register_app_t)();
        register_app_t *reg_ptr = reinterpret_cast<register_app_t *>( &registration_method );
        (*reg_ptr)();
        break;
      }
      case OBJECT:
      {
        typedef void (*register_app_t)(Factory *);
        register_app_t *reg_ptr = reinterpret_cast<register_app_t *>( &registration_method );
        (*reg_ptr)(params.get<Factory *>("factory"));
        break;
      }
      case SYNTAX:
      {
        typedef void (*register_app_t)(Syntax *, ActionFactory *);
        register_app_t *reg_ptr = reinterpret_cast<register_app_t *>( &registration_method );
        (*reg_ptr)(params.get<Syntax *>("syntax"), params.get<ActionFactory *>("action_factory"));
        break;
      }
      default:
        mooseError("Unhandled RegistrationTyep");
      }

      // Store the handle so we can close it later
      _lib_handles.insert(std::make_pair(std::make_pair(library_filename, registration_method_name), handle));
    }
  }
}

std::set<std::string>
MooseApp::getLoadedLibraryPaths() const
{
  // Return the paths but not the open file handles
  std::set<std::string> paths;
  for (std::map<std::pair<std::string, std::string>, void *>::const_iterator it = _lib_handles.begin(); it != _lib_handles.end(); ++it)
    paths.insert(it->first.first);

  return paths;
}
