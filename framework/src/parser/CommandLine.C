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

#include <iomanip>

#include "CommandLine.h"
#include "MooseInit.h"
#include "Parser.h"

CommandLine::CommandLine(int argc, char *argv[]) :
    _get_pot(new GetPot(argc, argv))
{
}

CommandLine::~CommandLine()
{
  delete _get_pot;
}

void
CommandLine::addOption(const std::string & name, Option cli_opt)
{
  for (unsigned int i = 0; i < cli_opt.cli_syntax.size(); i++)
  {
    std::string stx = cli_opt.cli_syntax[i];
    cli_opt.cli_switch.push_back(stx.substr(0, stx.find_first_of(" ")));
  }

  _cli_options[name] = cli_opt;
}

bool
CommandLine::search(const std::string &option_name)
{
  std::map<std::string, Option>::iterator pos = _cli_options.find(option_name);
  if (pos != _cli_options.end())
  {
    for (unsigned int i=0; i<pos->second.cli_switch.size(); ++i)
    {
      if (_get_pot->search(pos->second.cli_switch[i]))
        return true;
    }

    if (pos->second.required)
    {
      printUsage();
      mooseError("Required parameter: " << option_name << " missing");
    }
  }
  else
    mooseError("Unrecognized option name");

  return false;
}

void
CommandLine::printUsage() const
{
  // Grab the first item out of argv
  std::string command((*_get_pot)[0]);
  command.substr(command.find_last_of("/\\")+1);

  std::cout << "\nUsage: " << command << " [<options>]\n\n"
            << "Options:\n" << std::left;

  for (std::map<std::string, Option>::const_iterator i = _cli_options.begin(); i != _cli_options.end(); ++i)
  {
    std::stringstream oss;
    for (unsigned int j = 0; j < i->second.cli_syntax.size(); ++j)
    {
      if (j) oss << " | ";
      oss << i->second.cli_syntax[j];
    }
    std::cout << "  " << std::setw(50) << oss.str() << i->second.description << "\n";
  }

  std::cout << "\nSolver Options:\n"
            << "  See solver manual for details (Petsc or Trilinos)\n";
}

void
CommandLine::buildVarsSet()
{
  for(const char* var; (var = _get_pot->next_nominus()) != NULL; )
  {
    std::vector<std::string> name_value_pairs;
    Parser::tokenize(var, name_value_pairs, 0, "=");
    _command_line_vars.insert(name_value_pairs[0]);
  }
}

bool
CommandLine::isVariableOnCommandLine(const std::string &name) const
{
  return _command_line_vars.find(name) != _command_line_vars.end();
}

bool
CommandLine::haveVariable(const std::string & name)
{
  return _get_pot->have_variable(name);
}

void
CommandLine::print(const char * prefix, std::ostream & out_stream, unsigned int skip_count)
{
  _get_pot->print(prefix, out_stream, skip_count);
}
