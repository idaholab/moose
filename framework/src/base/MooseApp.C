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

#include "MooseApp.h"
#include "MooseSyntax.h"
#include "MooseInit.h"
#include "Executioner.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<MooseApp>()
{
  InputParameters params;

  params.addCommandLineParam<std::string>("input_file", "-i", "Specify an input file");
  params.addCommandLineParam<std::string>("mesh_only", "--mesh-only", "Setup and Output the input mesh only.");

  params.addCommandLineParam<bool>("show_input", "--show-input", "Shows the parsed input file before running the simulation.");

  params.addCommandLineParam<bool>("help", "-h --help", "Displays CLI usage statement.");

  params.addCommandLineParam<bool>("dump", "--dump", "Shows a dump of available input file syntax.");
  params.addCommandLineParam<bool>("yaml", "--yaml", "Dumps input file syntax in YAML format.");
  params.addCommandLineParam<bool>("syntax", "--syntax", "Dumps the associated Action syntax paths ONLY");

  params.addCommandLineParam<unsigned int>("n_threads", "--n-threads", 1, "Runs the specified number of threads (Intel TBB) per process");

  params.addCommandLineParam<bool>("warn_unused", "-w --warn-unused", "Warn about unused input file options");
  params.addCommandLineParam<bool>("error_unused", "-e --error-unused", "Error when encounting unused input file options");
  params.addCommandLineParam<bool>("error_override", "-o --error-override", "Error when encountering overriden or parameters supplied multipled times");

  params.addCommandLineParam<unsigned int>("refinements", "-r", 0, "Specify additional initial uniform refinements for automatic scaling");

  params.addPrivateParam<int>("_argc");
  params.addPrivateParam<char**>("_argv");

  return params;
}

MooseApp::MooseApp(const std::string & name, InputParameters parameters):
    _name(name),
    _pars(parameters),
    _command_line(NULL),
    _action_factory(*this),
    _action_warehouse(*this, _syntax, _action_factory),
    _parser(*this, _action_warehouse),
    _executioner(NULL),
    _sys_info(NULL),
    _enable_unused_check(WARN_UNUSED),
    _factory(*this),
    _error_overridden(false),
    _ready_to_exit(false)
{
  if(isParamValid("_argc") && isParamValid("_argv"))
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

  if (isParamValid("help"))
  {
    _command_line->printUsage();
    _ready_to_exit = true;
  }
  else if (isParamValid("dump"))
  {
    _parser.initSyntaxFormatter(Parser::INPUT_FILE, true);
    _parser.buildFullTree("");
    _ready_to_exit = true;
  }
  else if (isParamValid("yaml"))
  {
    _parser.initSyntaxFormatter(Parser::YAML, true);
    _parser.buildFullTree("");
    _ready_to_exit = true;
  }
  else if (isParamValid("syntax"))
  {
    std::multimap<std::string, Syntax::ActionInfo> syntax = _syntax.getAssociatedActions();
    std::cout << "**START SYNTAX DATA**\n";
    for (std::multimap<std::string, Syntax::ActionInfo>::iterator it = syntax.begin(); it != syntax.end(); ++it)
    {
      std::cout << it->first << "\n";
    }
    std::cout << "**END SYNTAX DATA**\n" << std::endl;
  }
  else if(_input_filename != "") // They already specified an input filename
  {
    _parser.parse(_input_filename);
    _action_warehouse.build();
    return;
  }
  else if (isParamValid("input_file"))
  {
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

  // Print the input file syntax if requested
  if (isParamValid("show_input"))
  {
    _action_warehouse.printInputFile(std::cout);
  }

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
    _executioner->execute();
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
  _action_warehouse.executeActionsWithAction("read_mesh");
  _action_warehouse.executeActionsWithAction("add_extra_nodeset");
  _action_warehouse.executeActionsWithAction("setup_mesh");
  _action_warehouse.executeActionsWithAction("add_mesh_modifier");
  _action_warehouse.executeActionsWithAction("setup_mesh_complete");

  if (mesh_file_name == "")
  {
    mesh_file_name = _parser.getFileName();
    size_t pos = mesh_file_name.find_last_of('.');

    // Default to writing out an ExodusII mesh base on the input filename.
    mesh_file_name = mesh_file_name.substr(0,pos) + "_in.e";
  }

  _action_warehouse.mesh()->getMesh().write(mesh_file_name);

  // Since we are not going to create a problem the mesh
  // will not get cleaned up, so we'll do it here
  delete _action_warehouse.mesh();
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

std::string
MooseApp::getSysInfo()
{
  if(_sys_info)
    return _sys_info->getInfo();
  else
    return "";
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

std::string
MooseApp::getFileName(bool stripLeadingPath) const
{
  return _parser.getFileName(stripLeadingPath);
}
