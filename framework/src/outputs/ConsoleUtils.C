//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ConsoleUtils.h"

#include "AuxiliarySystem.h"
#include "Conversion.h"
#include "Executioner.h"
#include "MoosePreconditioner.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "NonlinearSystem.h"
#include "OutputWarehouse.h"
#include "SystemInfo.h"
#include "Checkpoint.h"
#include "InputParameterWarehouse.h"
#include "Registry.h"
#include "CommandLine.h"

#include <filesystem>

#include "libmesh/string_to_enum.h"
#include "libmesh/simple_range.h"

using namespace libMesh;

namespace ConsoleUtils
{

std::string
indent(unsigned int spaces)
{
  return std::string(spaces, ' ');
}

std::string
outputFrameworkInformation(const MooseApp & app)
{
  std::stringstream oss;
  oss << std::left;

  if (app.getSystemInfo() != NULL)
    oss << app.getSystemInfo()->getInfo();

  oss << "Input File(s):\n";
  for (const auto & entry : app.getInputFileNames())
    oss << "  " << std::filesystem::absolute(entry).c_str() << "\n";
  oss << "\n";

  const auto & cl = std::as_const(*app.commandLine());
  // We skip the 0th argument of the main app, i.e., the name used to invoke the program
  const auto cl_range =
      as_range(std::next(cl.getEntries().begin(), app.multiAppLevel() == 0), cl.getEntries().end());

  std::stringstream args_oss;
  for (const auto & entry : cl_range)
    if (!entry.hit_param && !entry.subapp_name && entry.name != "-i")
      args_oss << "  " << cl.formatEntry(entry) << "\n";
  if (args_oss.str().size())
    oss << "Command Line Argument(s):\n" << args_oss.str() << "\n";

  std::stringstream input_args_oss;
  for (const auto & entry : cl_range)
    if (entry.hit_param && !entry.subapp_name)
      input_args_oss << "  " << cl.formatEntry(entry) << "\n";
  if (input_args_oss.str().size())
    oss << "Command Line Input Argument(s):\n" << input_args_oss.str() << "\n";

  const auto checkpoints = app.getOutputWarehouse().getOutputs<Checkpoint>();
  if (checkpoints.size())
  {
    oss << std::left << "Checkpoint:\n";
    oss << checkpoints[0]->checkpointInfo().str();
    oss << std::endl;
  }

  oss << std::left << "Parallelism:\n"
      << std::setw(console_field_width)
      << "  Num Processors: " << static_cast<std::size_t>(app.n_processors()) << '\n'
      << std::setw(console_field_width)
      << "  Num Threads: " << static_cast<std::size_t>(libMesh::n_threads()) << std::endl;

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
    bool forced = moose_mesh.isParallelTypeForced();
    bool pre_split = moose_mesh.isSplit();

    // clang-format off
    oss << "\nMesh: " << '\n'
        << std::setw(console_field_width)
        << "  Parallel Type: " << (moose_mesh.isDistributedMesh() ? "distributed" : "replicated")
        << (forced || pre_split ? " (" : "")
        << (forced ? "forced" : "")
        << (forced && pre_split ? ", " : "")
        << (pre_split ? "pre-split" : "")
        << (forced || pre_split ? ")" : "")
        << '\n'
        << std::setw(console_field_width) << "  Mesh Dimension: " << mesh.mesh_dimension() << '\n'
        << std::setw(console_field_width) << "  Spatial Dimension: " << mesh.spatial_dimension()
        << '\n';
    // clang-format on
  }

  if (mesh.n_processors() > 1)
  {
    dof_id_type nnodes = mesh.n_nodes();
    dof_id_type nnodes_local = mesh.n_local_nodes();
    oss << std::setw(console_field_width) << "  Nodes:" << '\n'
        << std::setw(console_field_width) << "    Total:" << nnodes << '\n';
    oss << std::setw(console_field_width) << "    Local:" << nnodes_local << '\n';
    dof_id_type min_nnodes = nnodes_local, max_nnodes = nnodes_local;
    mesh.comm().min(min_nnodes);
    mesh.comm().max(max_nnodes);
    if (mesh.processor_id() == 0)
      oss << std::setw(console_field_width) << "    Min/Max/Avg:" << min_nnodes << '/' << max_nnodes
          << '/' << nnodes / mesh.n_processors() << '\n';
  }
  else
    oss << std::setw(console_field_width) << "  Nodes:" << mesh.n_nodes() << '\n';

  if (mesh.n_processors() > 1)
  {
    dof_id_type nelems = mesh.n_active_elem();
    dof_id_type nelems_local = mesh.n_active_local_elem();
    oss << std::setw(console_field_width) << "  Elems:" << '\n'
        << std::setw(console_field_width) << "    Total:" << nelems << '\n';
    oss << std::setw(console_field_width) << "    Local:" << nelems_local << '\n';
    dof_id_type min_nelems = nelems_local, max_nelems = nelems_local;
    mesh.comm().min(min_nelems);
    mesh.comm().max(max_nelems);
    if (mesh.processor_id() == 0)
      oss << std::setw(console_field_width) << "    Min/Max/Avg:" << min_nelems << '/' << max_nelems
          << '/' << nelems / mesh.n_processors() << '\n';
  }
  else
    oss << std::setw(console_field_width) << "  Elems:" << mesh.n_active_elem() << '\n';

  if (verbose)
  {

    oss << std::setw(console_field_width)
        << "  Num Subdomains: " << static_cast<std::size_t>(mesh.n_subdomains()) << '\n';
    if (mesh.n_processors() > 1)
    {
      oss << std::setw(console_field_width)
          << "  Num Partitions: " << static_cast<std::size_t>(mesh.n_partitions()) << '\n'
          << std::setw(console_field_width) << "  Partitioner: " << moose_mesh.partitionerName()
          << (moose_mesh.isPartitionerForced() ? " (forced) " : "") << '\n';
      if (mesh.skip_partitioning())
        oss << std::setw(console_field_width) << "  Skipping all partitioning!" << '\n';
      else if (mesh.skip_noncritical_partitioning())
        oss << std::setw(console_field_width) << "  Skipping noncritical partitioning!" << '\n';
    }
  }

  oss << std::endl;

  return oss.str();
}

std::string
outputAuxiliarySystemInformation(FEProblemBase & problem)
{
  return outputSystemInformationHelper(problem.getAuxiliarySystem().system());
}

std::string
outputSystemInformationHelper(std::stringstream & oss, System & system)
{
  oss << std::left;

  if (system.n_dofs())
  {
    oss << std::setw(console_field_width) << "  Num DOFs: " << system.n_dofs() << '\n'
        << std::setw(console_field_width) << "  Num Local DOFs: " << system.n_local_dofs() << '\n';

    if (system.n_constrained_dofs())
    {
      oss << std::setw(console_field_width)
          << "  Num Constrained DOFs: " << system.n_constrained_dofs() << '\n'
          << std::setw(console_field_width)
          << "  Local Constrained DOFs: " << system.n_local_constrained_dofs() << '\n';
    }

    std::streampos begin_string_pos = oss.tellp();
    std::streampos curr_string_pos = begin_string_pos;
    oss << std::setw(console_field_width) << "  Variables: ";
    for (unsigned int vg = 0; vg < system.n_variable_groups(); vg++)
    {
      const VariableGroup & vg_description(system.variable_group(vg));

      if (vg_description.n_variables() > 1)
        oss << "{ ";
      if (vg_description.n_variables() > 10)
      {
        // when the number of variables in this group is larger than 10, we only output the first
        // and the last 5 variable names
        for (unsigned int vn = 0; vn < 5; vn++)
        {
          oss << "\"" << vg_description.name(vn) << "\" ";
          curr_string_pos = oss.tellp();
          insertNewline(oss, begin_string_pos, curr_string_pos);
        }
        oss << "... ";
        curr_string_pos = oss.tellp();
        insertNewline(oss, begin_string_pos, curr_string_pos);
        for (unsigned int vn = vg_description.n_variables() - 5; vn < vg_description.n_variables();
             vn++)
        {
          oss << "\"" << vg_description.name(vn) << "\" ";
          curr_string_pos = oss.tellp();
          insertNewline(oss, begin_string_pos, curr_string_pos);
        }
      }
      else
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
      oss << "\""
          << libMesh::Utility::enum_to_string<FEFamily>(
                 system.get_dof_map().variable_group(vg).type().family)
          << "\" ";
      curr_string_pos = oss.tellp();
      insertNewline(oss, begin_string_pos, curr_string_pos);
    }
    oss << '\n';
#else
    for (unsigned int vg = 0; vg < system.n_variable_groups(); vg++)
    {
      oss << "\""
          << libMesh::Utility::enum_to_string<FEFamily>(
                 system.get_dof_map().variable_group(vg).type().family)
          << "\", \""
          << libMesh::Utility::enum_to_string<FEFamily>(
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
      oss << "\""
          << libMesh::Utility::enum_to_string<InfMapType>(
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
          << "\", \""
          << Utility::enum_to_string<Order>(
                 system.get_dof_map().variable_group(vg).type().radial_order)
          << "\" ";
#endif
      curr_string_pos = oss.tellp();
      insertNewline(oss, begin_string_pos, curr_string_pos);
    }
    oss << "\n" << std::endl;
  }

  return oss.str();
}

std::string
outputNonlinearSystemInformation(FEProblemBase & problem, const unsigned int nl_sys_num)
{
  std::stringstream oss;
  oss << std::left;

  return outputSystemInformationHelper(oss, problem.getNonlinearSystemBase(nl_sys_num).system());
}

std::string
outputSystemInformationHelper(System & system)
{
  std::stringstream oss;

  return outputSystemInformationHelper(oss, system);
}

std::string
outputRelationshipManagerInformation(const MooseApp & app)
{
  std::stringstream oss;
  oss << std::left;

  auto info_strings = app.getRelationshipManagerInfo();
  if (info_strings.size())
  {
    for (const auto & info_pair : info_strings)
      oss << std::setw(console_field_width)
          << "  " + MooseUtils::underscoreToCamelCase(MooseUtils::toLower(info_pair.first), true) +
                 ":"
          << info_pair.second << '\n';
    oss << std::endl;
  }

  return oss.str();
}

std::string
outputExecutionInformation(const MooseApp & app, FEProblemBase & problem)
{

  std::stringstream oss;
  oss << std::left;

  Executioner * exec = app.getExecutioner();

  oss << "Execution Information:\n"
      << std::setw(console_field_width) << "  Executioner: " << exec->type() << '\n';

  std::string time_stepper = exec->getTimeStepperName();
  if (time_stepper != "")
    oss << std::setw(console_field_width) << "  TimeStepper: " << time_stepper << '\n';
  const auto time_integrator_names = exec->getTimeIntegratorNames();
  if (!time_integrator_names.empty())
    oss << std::setw(console_field_width)
        << "  TimeIntegrator(s): " << MooseUtils::join(time_integrator_names, ", ") << '\n';

  oss << std::setw(console_field_width) << "  Solver Mode: " << problem.solverTypeString() << '\n';

  const std::string & pc_desc = problem.getPetscOptions().pc_description;
  if (!pc_desc.empty())
    oss << std::setw(console_field_width) << "  PETSc Preconditioner: " << pc_desc << '\n';

  for (const auto i : make_range(problem.numNonlinearSystems()))
  {
    MoosePreconditioner const * mpc = problem.getNonlinearSystemBase(i).getPreconditioner();
    if (mpc)
    {
      oss << std::setw(console_field_width)
          << "  MOOSE Preconditioner" +
                 (problem.numNonlinearSystems() > 1 ? (" " + std::to_string(i)) : "") + ": "
          << mpc->getParam<std::string>("_type");
      if (mpc->name().find("_moose_auto") != std::string::npos)
        oss << " (auto)";
      oss << '\n';
    }
    oss << std::endl;
  }

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
              << "\"" << adv_it.second << "\"" << std::endl;
    }
  }

  return oss.str();
}

std::string
outputPreSMOResidualInformation()
{
  std::stringstream oss;
  oss << std::left;

  oss << COLOR_BLUE;
  oss << "Executioner/use_pre_smo_residual is set to true. The pre-SMO residual will be evaluated "
         "at the beginning of each time step before executing objects that could modify the "
         "solution, such as preset BCs, predictors, correctors, constraints, and certain user "
         "objects. The pre-SMO residuals will be prefixed with * and will be used in the relative "
         "convergence check.\n";
  oss << COLOR_DEFAULT;

  return oss.str();
}

std::string
outputLegacyInformation(MooseApp & app)
{
  std::stringstream oss;
  oss << std::left;

  if (app.parameters().get<bool>("use_legacy_material_output"))
  {
    oss << COLOR_RED << "LEGACY MODES ENABLED:" << COLOR_DEFAULT << '\n';
    oss << " This application uses the legacy material output option: material properties are "
           "output only on TIMESTEP_END, not INITIAL. To remove this message, set "
           "'use_legacy_material_output' to false in this application. If there are gold output "
           "files that contain material property output for which output occurs on INITIAL, then "
           "these will generate diffs due to zero values being stored, and these tests should be "
           "re-golded.\n"
        << COLOR_DEFAULT << std::endl;
  }

  if (app.parameters().get<bool>("use_legacy_initial_residual_evaluation_behavior"))
  {
    oss << COLOR_RED << "LEGACY MODES ENABLED:" << COLOR_DEFAULT << '\n';
    oss << " This application uses the legacy initial residual evaluation behavior. The legacy "
           "behavior performs an often times redundant residual evaluation before the solution "
           "modifying objects are executed prior to the initial (0th nonlinear iteration) residual "
           "evaluation. The new behavior skips that redundant residual evaluation unless the "
           "parameter Executioner/use_pre_smo_residual is set to true. To remove this message and "
           "enable the new behavior, set the parameter "
           "'use_legacy_initial_residual_evaluation_behavior' to false in *App.C. Some tests that "
           "rely on the side effects of the legacy behavior may fail/diff and should be "
           "re-golded.\n"
        << COLOR_DEFAULT << std::endl;
  }

  return oss.str();
}

std::string
outputDataFilePaths()
{
  std::stringstream oss;
  oss << "Data File Paths:\n";
  for (const auto & [name, path] : Registry::getDataFilePaths())
    oss << "  " << name << ": " << path << "\n";
  return oss.str() + "\n";
}

std::string
outputDataFileParams(MooseApp & app)
{
  std::map<std::string, std::string> values; // for A-Z sort
  for (const auto & object_name_params_pair : app.getInputParameterWarehouse().getInputParameters())
  {
    const auto & params = object_name_params_pair.second;
    for (const auto & name_value_pair : *params)
    {
      const auto & name = name_value_pair.first;
      if (const auto path = params->queryDataFileNamePath(name))
        if (params->getHitNode(name))
          values.emplace(params->paramFullpath(name), path->path);
    }
  }

  std::stringstream oss;
  oss << "Data File Parameters:\n";
  for (const auto & [param, value] : values)
    oss << "  " << param << " = " << value << "\n";
  return oss.str() + '\n';
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

std::string
formatString(std::string message, const std::string & prefix)
{
  MooseUtils::indentMessage(prefix, message, COLOR_DEFAULT, true, " ");
  std::stringstream stream;
  std::streampos start = stream.tellp();
  stream << message;
  std::streampos end = stream.tellp();
  insertNewline(stream, start, end);
  auto formatted_string = stream.str();
  // no need to end with a line break
  if (formatted_string.back() == '\n')
    formatted_string.pop_back();
  return formatted_string;
}

std::string
mooseObjectVectorToString(const std::vector<MooseObject *> & objs, const std::string & sep /*=""*/)
{
  std::string object_names = "";
  if (objs.size())
  {
    // Gather all the object names
    std::vector<std::string> names;
    names.reserve(objs.size());
    for (const auto & obj : objs)
    {
      mooseAssert(obj, "Trying to print a null object");
      names.push_back(obj->name());
    }

    object_names = MooseUtils::join(names, sep);
  }
  return object_names;
}

} // ConsoleUtils namespace
