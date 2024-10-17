//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "DOFMapOutput.h"
#include "FEProblem.h"
#include "KernelBase.h"
#include "MooseApp.h"
#include "Moose.h"
#include "Conversion.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"

#include "libmesh/fe.h"

// compiler includes (for type demangling)
#include <cxxabi.h>
#include <fstream>

using namespace libMesh;

registerMooseObjectAliased("MooseApp", DOFMapOutput, "DOFMap");

InputParameters
DOFMapOutput::validParams()
{
  // Get the parameters from the base class
  InputParameters params = FileOutput::validParams();
  params.addClassDescription("Output degree-of-freedom (DOF) map.");

  // Screen and file output toggles
  params.addParam<bool>("output_screen", false, "Output to the screen");
  params.addParam<bool>("output_file", true, "Output to the file");
  params.addParam<std::string>("system_name", "nl0", "System to output");

  // By default this only executes on the initial timestep
  params.set<ExecFlagEnum>("execute_on", true) = EXEC_INITIAL;

  params.addParam<NonlinearSystemName>(
      "nl_sys", "nl0", "The nonlinear system that we should output information for.");
  return params;
}

DOFMapOutput::DOFMapOutput(const InputParameters & parameters)
  : FileOutput(parameters),
    _write_file(getParam<bool>("output_file")),
    _write_screen(getParam<bool>("output_screen")),
    _system_name(getParam<std::string>("system_name")),
    _mesh(_problem_ptr->mesh()),
    _nl_sys_num(_problem_ptr->nlSysNum(getParam<NonlinearSystemName>("nl_sys")))
{
}

std::string
DOFMapOutput::filename()
{
  if (_file_num > 0)
    return _file_base + "_" + Moose::stringify(_file_num) + ".json";
  else
    return _file_base + ".json";
}

std::string
DOFMapOutput::demangle(const std::string & name)
{
#if defined(LIBMESH_HAVE_GCC_ABI_DEMANGLE)
  return libMesh::demangle(name.c_str());
#else
  // at least remove leading digits
  std::string demangled(name);
  while (demangled.length() && demangled[0] >= '0' && demangled[0] <= '9')
    demangled.erase(0, 1);

  return demangled;
#endif
}

void
DOFMapOutput::writeStreamToFile(bool /*append*/)
{
  // Create the stream
  std::ofstream output;

  // Open the file and write contents of file output stream and close the file
  output.open(filename().c_str(), std::ios::trunc);
  if (output.fail())
    mooseError("Unable to open file ", filename());

  output << _file_output_stream.str();
  output.close();

  // Clear the file output stream
  _file_output_stream.str("");
  _file_num++;
}

template <typename T>
std::string
DOFMapOutput::join(const T & begin, const T & end, const char * const delim)
{
  std::ostringstream os;
  for (T it = begin; it != end; ++it)
    os << (it != begin ? delim : "") << *it;
  return os.str();
}

void
DOFMapOutput::output()
{
  // Don't build this information if nothing is to be written
  if (!_write_screen && !_write_file)
    return;

  std::stringstream oss;

  // Get the DOF Map through the equation system
  const System & sys = _problem_ptr->es().get_system(_system_name); // TransientNonlinearImplicit
  const DofMap & dof_map = sys.get_dof_map();

  // fetch the KernelWarehouse through the NonlinearSystem
  NonlinearSystemBase & _nl = _problem_ptr->getNonlinearSystemBase(_nl_sys_num);
  auto & kernels = _nl.getKernelWarehouse();

  // get a set of all subdomains
  const std::set<SubdomainID> & subdomains = _mesh.meshSubdomains();

  bool first = true;
  oss << "{\"ndof\": " << sys.n_dofs() << ", \"demangled\": ";
#if defined(LIBMESH_HAVE_GCC_ABI_DEMANGLE)
  oss << "true";
#else
  oss << "false";
#endif
  oss << ", \"vars\": [";
  for (unsigned int vg = 0; vg < dof_map.n_variable_groups(); ++vg)
  {
    const VariableGroup & vg_description(dof_map.variable_group(vg));
    for (unsigned int vn = 0; vn < vg_description.n_variables(); ++vn)
    {
      unsigned int var = vg_description.number(vn);

      if (!first)
        oss << ", ";
      first = false;

      oss << "{\"name\": \"" << vg_description.name(vn) << "\", \"subdomains\": [";
      for (std::set<SubdomainID>::const_iterator sd = subdomains.begin(); sd != subdomains.end();
           ++sd)
      {
        oss << (sd != subdomains.begin() ? ", " : "") << "{\"id\": " << *sd << ", \"kernels\": [";

        // if this variable has active kernels output them
        if (kernels.hasActiveVariableBlockObjects(var, *sd))
        {
          const auto & active_kernels = kernels.getActiveVariableBlockObjects(var, *sd);
          for (unsigned i = 0; i < active_kernels.size(); ++i)
          {
            KernelBase & kb = *(active_kernels[i].get());
            oss << (i > 0 ? ", " : "") << "{\"name\": \"" << kb.name() << "\", \"type\": \""
                << demangle(typeid(kb).name()) << "\"}";
          }
        }
        oss << "], \"dofs\": [";

        // get the list of unique DOFs for this variable
        // Start by looking at local DOFs
        std::set<dof_id_type> dofs;
        ConstElemRange * active_local_elems = _mesh.getActiveLocalElementRange();
        for (const auto & elem : *active_local_elems)
        {
          if (elem->subdomain_id() == *sd)
          {
            std::vector<dof_id_type> di;
            dof_map.dof_indices(elem, di, var);
            dofs.insert(di.begin(), di.end());
          }
        }

        // Then collect DOFs from other processors.  On a distributed
        // mesh they may know about DOFs we can't even see.
        _communicator.set_union(dofs);

        oss << join(dofs.begin(), dofs.end(), ", ") << "]}";
      }
      oss << "]}";
    }
  }
  oss << "]}" << std::endl;

  // Write the message to file stream
  if (_write_file)
    _file_output_stream << oss.str() << std::endl;

  // Write message to the screen
  if (_write_screen)
    _console << oss.str() << std::flush;

  // Write the actual file
  if (_write_file)
    writeStreamToFile();
}
