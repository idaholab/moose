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
#include "ConsoleUtils.h"

#include "AuxiliarySystem.h"
#include "Conversion.h"
#include "Executioner.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "OutputWarehouse.h"
#include "SystemInfo.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

namespace ConsoleUtils
{

std::string
indent(unsigned int spaces)
{
  return std::string(spaces, ' ');
}

std::string
outputFrameworkInformation(MooseApp & app)
{
  std::stringstream oss;
  oss << std::left;

  if (app.getSystemInfo() != NULL)
    oss << app.getSystemInfo()->getInfo();

  oss << std::left << "Parallelism:\n"
      << std::setw(console_field_width)
      << "  Num Processors: " << static_cast<std::size_t>(app.n_processors()) << '\n'
      << std::setw(console_field_width)
      << "  Num Threads: " << static_cast<std::size_t>(libMesh::n_threads()) << '\n'
      << '\n';

  return oss.str();
}

std::string
outputMeshInformation(FEProblemBase & problem, bool verbose)
{
  std::stringstream oss;
  oss << std::left;

  MooseMesh & moose_mesh = problem.mesh();
  MeshBase & mesh = moose_mesh.getMesh();

  if (verbose)
  {
    oss << "Mesh: " << '\n'
        << std::setw(console_field_width)
        << "  Parallel Type: " << (moose_mesh.isDistributedMesh() ? "distributed" : "replicated")
        << (moose_mesh.isParallelTypeForced() ? " (forced) " : "") << '\n'
        << std::setw(console_field_width) << "  Mesh Dimension: " << mesh.mesh_dimension() << '\n'
        << std::setw(console_field_width) << "  Spatial Dimension: " << mesh.spatial_dimension()
        << '\n';
  }

  oss << std::setw(console_field_width) << "  Nodes:" << '\n'
      << std::setw(console_field_width) << "    Total:" << mesh.n_nodes() << '\n'
      << std::setw(console_field_width) << "    Local:" << mesh.n_local_nodes() << '\n'
      << std::setw(console_field_width) << "  Elems:" << '\n'
      << std::setw(console_field_width) << "    Total:" << mesh.n_active_elem() << '\n'
      << std::setw(console_field_width) << "    Local:" << mesh.n_active_local_elem() << '\n';

  if (verbose)
  {

    oss << std::setw(console_field_width)
        << "  Num Subdomains: " << static_cast<std::size_t>(mesh.n_subdomains()) << '\n'
        << std::setw(console_field_width)
        << "  Num Partitions: " << static_cast<std::size_t>(mesh.n_partitions()) << '\n';
    if (problem.n_processors() > 1)
      oss << std::setw(console_field_width) << "  Partitioner: " << moose_mesh.partitionerName()
          << (moose_mesh.isPartitionerForced() ? " (forced) " : "") << '\n';
  }

  oss << '\n';

  return oss.str();
}

std::string
outputAuxiliarySystemInformation(FEProblemBase & problem)
{
  return outputSystemInformationHelper(problem.getAuxiliarySystem().system());
}

std::string
outputNonlinearSystemInformation(FEProblemBase & problem)
{
  return outputSystemInformationHelper(problem.getNonlinearSystemBase().system());
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
    for (unsigned int vg = 0; vg < system.n_variable_groups(); vg++)
    {
      const VariableGroup & vg_description(system.variable_group(vg));

      if (vg_description.n_variables() > 1)
        oss << "{ ";
      for (unsigned int vn = 0; vn < vg_description.n_variables(); vn++)
      {
        oss << "\"" << vg_description.name(vn) << "\" ";
        curr_string_pos = oss.tellp();
        insertNewline(oss, begin_string_pos, curr_string_pos);
      }

      if (vg_description.n_variables() > 1)
        oss << "} ";
    }
    oss << '\n';

    begin_string_pos = oss.tellp();
    curr_string_pos = begin_string_pos;
    oss << std::setw(console_field_width) << "  Finite Element Types: ";
#ifndef LIBMESH_ENABLE_INFINITE_ELEMENTS
    for (unsigned int vg = 0; vg < system.n_variable_groups(); vg++)
    {
      oss << "\"" << libMesh::Utility::enum_to_string<FEFamily>(
                         system.get_dof_map().variable_group(vg).type().family)
          << "\" ";
      curr_string_pos = oss.tellp();
      insertNewline(oss, begin_string_pos, curr_string_pos);
    }
    oss << '\n';
#else
    for (unsigned int vg = 0; vg < system.n_variable_groups(); vg++)
    {
      oss << "\"" << libMesh::Utility::enum_to_string<FEFamily>(
                         system.get_dof_map().variable_group(vg).type().family)
          << "\", \"" << libMesh::Utility::enum_to_string<FEFamily>(
                             system.get_dof_map().variable_group(vg).type().radial_family)
          << "\" ";
      curr_string_pos = oss.tellp();
      insertNewline(oss, begin_string_pos, curr_string_pos);
    }
    oss << '\n';

    begin_string_pos = oss.tellp();
    curr_string_pos = begin_string_pos;
    oss << std::setw(console_field_width) << "  Infinite Element Mapping: ";
    for (unsigned int vg = 0; vg < system.n_variable_groups(); vg++)
    {
      oss << "\"" << libMesh::Utility::enum_to_string<InfMapType>(
                         system.get_dof_map().variable_group(vg).type().inf_map)
          << "\" ";
      curr_string_pos = oss.tellp();
      insertNewline(oss, begin_string_pos, curr_string_pos);
    }
    oss << '\n';
#endif

    begin_string_pos = oss.tellp();
    curr_string_pos = begin_string_pos;
    oss << std::setw(console_field_width) << "  Approximation Orders: ";
    for (unsigned int vg = 0; vg < system.n_variable_groups(); vg++)
    {
#ifndef LIBMESH_ENABLE_INFINITE_ELEMENTS
      oss << "\""
          << Utility::enum_to_string<Order>(system.get_dof_map().variable_group(vg).type().order)
          << "\" ";
#else
      oss << "\""
          << Utility::enum_to_string<Order>(system.get_dof_map().variable_group(vg).type().order)
          << "\", \"" << Utility::enum_to_string<Order>(
                             system.get_dof_map().variable_group(vg).type().radial_order)
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
outputExecutionInformation(MooseApp & app, FEProblemBase & problem)
{

  std::stringstream oss;
  oss << std::left;

  Executioner * exec = app.getExecutioner();

  oss << "Execution Information:\n"
      << std::setw(console_field_width) << "  Executioner: " << demangle(typeid(*exec).name())
      << '\n';

  std::string time_stepper = exec->getTimeStepperName();
  if (time_stepper != "")
    oss << std::setw(console_field_width) << "  TimeStepper: " << time_stepper << '\n';

  oss << std::setw(console_field_width)
      << "  Solver Mode: " << Moose::stringify<Moose::SolveType>(problem.solverParams()._type)
      << '\n';

  const std::string & pc_desc = problem.getPetscOptions().pc_description;
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

  const std::vector<Output *> outputs = app.getOutputWarehouse().getOutputs<Output>();
  oss << "Outputs:\n";
  for (const auto & out : outputs)
  {
    // Display the "execute_on" settings
    const MultiMooseEnum & execute_on = out->executeOn();
    oss << "  " << std::setw(console_field_width - 2) << out->name() << "\"" << execute_on
        << "\"\n";

    // Display the advanced "execute_on" settings, only if they are different from "execute_on"
    if (out->isAdvanced())
    {
      const OutputOnWarehouse & adv_on = out->advancedExecuteOn();
      for (const auto & adv_it : adv_on)
        if (execute_on != adv_it.second)
          oss << "    " << std::setw(console_field_width - 4) << adv_it.first + ":"
              << "\"" << adv_it.second << "\"\n";
    }
  }

  return oss.str();
}

void
insertNewline(std::stringstream & oss, std::streampos & begin, std::streampos & curr)
{
  if (curr - begin > console_line_length)
  {
    oss << "\n";
    begin = oss.tellp();
    oss << std::setw(console_field_width + 2) << ""; // "{ "
  }
}

} // ConsoleUtils namespace
