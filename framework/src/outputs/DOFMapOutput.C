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
#include "DOFMapOutput.h"
#include "FEProblem.h"
#include "KernelBase.h"
#include "MooseApp.h"
#include "Moose.h"

// libMesh includes
#include "libmesh/fe.h"

template<>
InputParameters validParams<DOFMapOutput>()
{
  // Get the parameters from the base class
  InputParameters params = validParams<FileOutput>();
  params += Output::enableOutputTypes("system_information");

  // Screen and file output toggles
  params.addParam<bool>("output_screen", false, "Output to the screen");
  params.addParam<bool>("output_file", true, "Output to the file");
  params.addParam<std::string>("system_name", "nl0", "System to output");
  return params;
}

DOFMapOutput::DOFMapOutput(const std::string & name, InputParameters parameters) :
    FileOutput(name, parameters),
    _write_file(getParam<bool>("output_file")),
    _write_screen(getParam<bool>("output_screen")),
    _system_name(getParam<std::string>("system_name")),
    _mesh(_mci_feproblem.mesh()),
    _console_buffer(_app.getOutputWarehouse().consoleBuffer())
{
  // Set output coloring
  if (Moose::_color_console)
  {
    char * term_env = getenv("TERM");
    if (term_env)
    {
      std::string term(term_env);
      if (term != "xterm-256color" && term != "xterm")
        Moose::_color_console = false;
    }
  }
}

DOFMapOutput::~DOFMapOutput()
{
  // Write the file output stream
  writeStreamToFile();
}

void
DOFMapOutput::initialSetup()
{
  // If file output is desired, wipe out the existing file if not recovering
  if (!_app.isRecovering())
    writeStreamToFile(false);

  // Output the system information
  if (_system_information && _allow_output)
    outputSystemInformation();
}

std::string
DOFMapOutput::filename()
{
  return _file_base + ".json";
}

void
DOFMapOutput::writeStreamToFile(bool append)
{
  if (!_write_file)
    return;

  // Create the stream
  std::ofstream output;

  // Open the file
  if (append)
    output.open(filename().c_str(), std::ios::app | std::ios::out);
  else
    output.open(filename().c_str(), std::ios::trunc);

  // Write contents of file output stream and close the file
  output << _file_output_stream.str();
  output.close();

  // Clear the file output stream
  _file_output_stream.str("");
}

template<typename T>
std::string
DOFMapOutput::join(const T & begin, const T & end, const char* const delim)
{
  std::ostringstream os;
  for (T it = begin; it != end; ++it)
    os << (it != begin ? delim : "") << *it;
  return os.str();
}

void
DOFMapOutput::write(std::string message)
{
  // Do nothing if the message is empty, writing empty strings messes with multiapp indenting
  if (message.empty())
    return;

  // Write the message to file
  if (_write_file)
    _file_output_stream << message << std::endl;

  // Write message to the screen
  if (_write_screen)
    Moose::out << message;
}

void
DOFMapOutput::mooseConsoleOutput(const std::string & message)
{
  // Write the messages
  write(message);

  // Flush the stream to the screen
  Moose::out << std::flush;
}

void
DOFMapOutput::outputSystemInformation()
{
  // Don't build this information if nothing is to be written
  if (!_write_screen && !_write_file)
    return;

  std::stringstream oss;

  // Get the DOF Map through the equation system
  const System & sys = _problem_ptr->es().get_system(_system_name); // TransientNonlinearImplicit
  const DofMap & dof_map = sys.get_dof_map();

  // fetch the KernelWarehouse through the NonlinearSystem
  NonlinearSystem & nl = _problem_ptr->getNonlinearSystem();
  const KernelWarehouse & kernels = nl.getKernelWarehouse(0);

  // get a set of all subdomains
  const std::set<SubdomainID> & subdomains = _mesh.meshSubdomains();

  bool first = true;
  oss << '[';
  for (unsigned int vg = 0; vg < dof_map.n_variable_groups(); ++vg)
  {
    const VariableGroup &vg_description (dof_map.variable_group(vg));
    for (unsigned int vn = 0; vn < vg_description.n_variables(); ++vn)
    {
      unsigned int var = vg_description.number(vn);

      if (!first)
        oss << ", ";
      first = false;

      oss << "{\"var\": \"" << vg_description.name(vn) << "\", \"subdomains\": [";
      for (std::set<SubdomainID>::const_iterator sd = subdomains.begin(); sd != subdomains.end(); ++sd)
      {
        oss << (sd != subdomains.begin() ? ", " : "") << "{\"id\": " << *sd << ", \"kernels\": [";

        // build a list of all kernels in the current subdomain
        nl.updateActiveKernels(*sd, 0);
        const std::vector<KernelBase *> & active_kernels = kernels.activeVar(var);
        for (unsigned i = 0; i<active_kernels.size(); ++i)
          oss << (i>0 ? ", \"" : "\"") << active_kernels[i]->name() << '"';

        oss << "], \"dofs\": [";

        // get the list of unique DOFs for this variable
        std::set<dof_id_type> dofs;
        for (unsigned int i = 0; i < _mesh.nElem(); ++i)
          if (_mesh.elem(i)->subdomain_id() == *sd)
          {
            std::vector<dof_id_type> di;
            dof_map.dof_indices(_mesh.elem(i), di, var);
            dofs.insert(di.begin(), di.end());
          }
        oss << join(dofs.begin(), dofs.end(), ", ") << "]}";
      }
      oss << "]}";
    }
  }
  oss << "]\n";

  // Output the information
  write(oss.str());
}
