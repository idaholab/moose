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

// STL includes
#include <ostream>

// MOOSE includes
#include "ConsoleUtils.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "SystemInfo.h"
#include "Executioner.h"
#include "Conversion.h"
#include "OutputWarehouse.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

namespace ConsoleUtils
{


std::string
outputFrameworkInformation(MooseApp & app)
{
  std::stringstream oss;
  oss << std::left;

  if (app.getSystemInfo() != NULL)
    oss << app.getSystemInfo()->getInfo();

  oss << std::left
      << "Parallelism:\n"
      << std::setw(console_field_width) << "  Num Processors: " << static_cast<std::size_t>(app.n_processors()) << '\n'
      << std::setw(console_field_width) << "  Num Threads: " << static_cast<std::size_t>(libMesh::n_threads()) << '\n'
      << '\n';

  return oss.str();
}


std::string
outputMeshInformation(FEProblem & problem, bool verbose)
{
  std::stringstream oss;
  oss << std::left;

  MooseMesh & moose_mesh = problem.mesh();
  MeshBase & mesh = moose_mesh.getMesh();

  if (verbose)
  {
    oss << "Mesh: " << '\n'
        << std::setw(console_field_width) << "  Distribution: " << (moose_mesh.isParallelMesh() ? "parallel" : "serial")
        << (moose_mesh.isDistributionForced() ? " (forced) " : "") << '\n'
        << std::setw(console_field_width) << "  Mesh Dimension: " << mesh.mesh_dimension() << '\n'
        << std::setw(console_field_width) << "  Spatial Dimension: " << mesh.spatial_dimension() << '\n';
  }

  oss << std::setw(console_field_width) << "  Nodes:" << '\n'
      << std::setw(console_field_width) << "    Total:" << mesh.n_nodes() << '\n'
      << std::setw(console_field_width) << "    Local:" << mesh.n_local_nodes() << '\n'
      << std::setw(console_field_width) << "  Elems:" << '\n'
      << std::setw(console_field_width) << "    Total:" << mesh.n_elem() << '\n'
      << std::setw(console_field_width) << "    Local:" << mesh.n_local_elem() << '\n';

  if (verbose)
  {

    oss << std::setw(console_field_width) << "  Num Subdomains: "       << static_cast<std::size_t>(mesh.n_subdomains()) << '\n'
        << std::setw(console_field_width) << "  Num Partitions: "       << static_cast<std::size_t>(mesh.n_partitions()) << '\n';
  if (problem.n_processors() > 1 && moose_mesh.partitionerName() != "")
    oss << std::setw(console_field_width) << "  Partitioner: "       << moose_mesh.partitionerName()
        << (moose_mesh.isPartitionerForced() ? " (forced) " : "")
        << '\n';
  }

  oss << '\n';

  return oss.str();
}


std::string
outputAuxiliarySystemInformation(FEProblem & problem)
{
  return outputSystemInformationHelper(problem.getAuxiliarySystem().system());
}


std::string
outputNonlinearSystemInformation(FEProblem & problem)
{
  return outputSystemInformationHelper(problem.getNonlinearSystem().system());
}


std::string
outputSystemInformationHelper(const System & system)
{
  std::stringstream oss;
  oss << std::left;

  if (system.n_dofs())
  {
    oss << std::setw(console_field_width) << "  Num DOFs: " << system.n_dofs() << '\n'
        << std::setw(console_field_width) << "  Num Local DOFs: " << system.n_local_dofs() << '\n';

    std::streampos begin_string_pos = oss.tellp();
    std::streampos curr_string_pos = begin_string_pos;
    oss << std::setw(console_field_width) << "  Variables: ";
    for (unsigned int vg=0; vg<system.n_variable_groups(); vg++)
    {
      const VariableGroup &vg_description (system.variable_group(vg));

      if (vg_description.n_variables() > 1) oss << "{ ";
      for (unsigned int vn=0; vn<vg_description.n_variables(); vn++)
      {
        oss << "\"" << vg_description.name(vn) << "\" ";
        curr_string_pos = oss.tellp();
        insertNewline(oss, begin_string_pos, curr_string_pos);
      }

      if (vg_description.n_variables() > 1) oss << "} ";
    }
    oss << '\n';

    begin_string_pos = oss.tellp();
    curr_string_pos = begin_string_pos;
    oss << std::setw(console_field_width) << "  Finite Element Types: ";
#ifndef LIBMESH_ENABLE_INFINITE_ELEMENTS
    for (unsigned int vg=0; vg<system.n_variable_groups(); vg++)
    {
      oss << "\""
          << libMesh::Utility::enum_to_string<FEFamily>(system.get_dof_map().variable_group(vg).type().family)
          << "\" ";
      curr_string_pos = oss.tellp();
      insertNewline(oss, begin_string_pos, curr_string_pos);
    }
    oss << '\n';
#else
    for (unsigned int vg=0; vg<system.n_variable_groups(); vg++)
    {
      oss << "\""
          << libMesh::Utility::enum_to_string<FEFamily>(system.get_dof_map().variable_group(vg).type().family)
          << "\", \""
          << libMesh::Utility::enum_to_string<FEFamily>(system.get_dof_map().variable_group(vg).type().radial_family)
          << "\" ";
      curr_string_pos = oss.tellp();
      insertNewline(oss, begin_string_pos, curr_string_pos);
    }
    oss << '\n';

    begin_string_pos = oss.tellp();
    curr_string_pos = begin_string_pos;
    oss << std::setw(console_field_width) << "  Infinite Element Mapping: ";
    for (unsigned int vg=0; vg<system.n_variable_groups(); vg++)
    {
      oss << "\""
          << libMesh::Utility::enum_to_string<InfMapType>(system.get_dof_map().variable_group(vg).type().inf_map)
          << "\" ";
      curr_string_pos = oss.tellp();
      insertNewline(oss, begin_string_pos, curr_string_pos);
    }
    oss << '\n';
#endif

    begin_string_pos = oss.tellp();
    curr_string_pos = begin_string_pos;
    oss << std::setw(console_field_width) << "  Approximation Orders: ";
    for (unsigned int vg=0; vg<system.n_variable_groups(); vg++)
    {
#ifndef LIBMESH_ENABLE_INFINITE_ELEMENTS
      oss << "\""
          << Utility::enum_to_string<Order>(system.get_dof_map().variable_group(vg).type().order)
          << "\" ";
#else
      oss << "\""
          << Utility::enum_to_string<Order>(system.get_dof_map().variable_group(vg).type().order)
          << "\", \""
          << Utility::enum_to_string<Order>(system.get_dof_map().variable_group(vg).type().radial_order)
          << "\" ";
#endif
      curr_string_pos = oss.tellp();
      insertNewline(oss, begin_string_pos, curr_string_pos);
    }
    oss << "\n\n";
  }

  return oss.str();
}


std::string
outputExecutionInformation(MooseApp & app, FEProblem & problem)
{

  std::stringstream oss;
  oss << std::left;

  Executioner * exec = app.actionWarehouse().executioner().get();

  oss << "Execution Information:\n"
      << std::setw(console_field_width) << "  Executioner: " << demangle(typeid(*exec).name()) << '\n';

  std::string time_stepper = exec->getTimeStepperName();
  if (time_stepper != "")
    oss << std::setw(console_field_width) << "  TimeStepper: " << time_stepper << '\n';

  oss << std::setw(console_field_width) << "  Solver Mode: " << Moose::stringify<Moose::SolveType>(problem.solverParams()._type) << '\n';

  const std::string & pc_desc = problem.getPreconditionerDescription();
  if (!pc_desc.empty())
    oss << std::setw(console_field_width) << "  Preconditioner: " << pc_desc << '\n';
  oss << '\n';

  return oss.str();
}


std::string
outputOutputInformation(MooseApp & app)
{
  std::stringstream oss;
  oss << std::left;

  const std::vector<Output *> & outputs = app.getOutputWarehouse().all();
  oss << "Outputs:\n";
  for (std::vector<Output *>::const_iterator it = outputs.begin(); it != outputs.end(); ++it)
  {
    // Display the "output_on" settings
    const MultiMooseEnum & output_on = (*it)->outputOn();
    oss << "  " << std::setw(console_field_width-2) << (*it)->name() <<  "\"" << output_on << "\"\n";

    // Display the advanced "output_on" settings, only if they are different from "output_on"
    if ((*it)->isAdvanced())
    {
      const OutputOnWarehouse & adv_on = (*it)->advancedOutputOn();
      for (std::map<std::string, MultiMooseEnum>::const_iterator adv_it = adv_on.begin(); adv_it != adv_on.end(); ++adv_it)
        if (output_on != adv_it->second)
          oss << "    " << std::setw(console_field_width-4) << adv_it->first + ":" <<  "\"" << adv_it->second << "\"\n";
    }
  }

  return oss.str();
}


std::string outputLegacyInformation(FEProblem & problem)
{
  std::stringstream oss;
  oss << std::left;

  if (problem.legacyUoAuxComputation() || problem.legacyUoInitialization())
  {
    oss << COLOR_RED << "LEGACY MODES ENABLED:\n";
    if (problem.legacyUoAuxComputation())
      oss << COLOR_RED << "  Computing EXEC_LINEAR AuxKernel types when any UserObject type is executed.\n";
    if (problem.legacyUoInitialization())
      oss << COLOR_RED << "  Computing all UserObjects during initial setup.\n";
  }
  oss << COLOR_DEFAULT;

  return oss.str();
}


void
insertNewline(std::stringstream &oss, std::streampos &begin, std::streampos &curr)
{
   if (curr - begin > console_line_length)
   {
     oss << "\n";
     begin = oss.tellp();
     oss << std::setw(console_field_width + 2) << "";  // "{ "
   }
}

} // ConsoleUtils namespace
