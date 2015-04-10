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
#include "SimulationInformation.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "Executioner.h"
#include "Conversion.h"

// libMesh includes
#include "libmesh/string_to_enum.h"


template<>
InputParameters validParams<SimulationInformation>()
{
  // Get the base class parameters
  InputParameters params = validParams<BasicOutput<Output> >();
  params.set<MultiMooseEnum>("output_on") = "initial";

  // Add MultiMooseEnum for controlling what is output
  MultiMooseEnum info("framework mesh aux nonlinear execution output", "framework mesh aux nonlinear execution");
  params.addParam<MultiMooseEnum>("info", info, "List of information types to display ('framework', 'mesh', 'aux', 'nonlinear', 'execution', 'output')");

  return params;
}


SimulationInformation::SimulationInformation(const std::string & name, InputParameters parameters) :
    BasicOutput<Output>(name, parameters),
    _flags(getParam<MultiMooseEnum>("info"))
{
  // If --show-outputs is used, enable it
  if (_app.getParam<bool>("show_outputs"))
    _flags.push_back("output");
}


void
SimulationInformation::init()
{
  // If output_on = 'initial' perform the output
  if (shouldOutput(EXEC_INITIAL))
    output(EXEC_INITIAL);

  // Sets the _initialized flag
  Output::init();
}


void
SimulationInformation::output(const ExecFlagType & type)
{
  // SimulationInformation performs the initial output during the
  // call to init() so that the information comes out earlier, so
  // do not output it again if it has already been called
  if (type == EXEC_INITIAL && _initialized)
    return;

  // Perform the various outputs
  if (_flags.contains("framework"))
    outputFrameworkInformation();

  if (_flags.contains("mesh"))
    outputMeshInformation();

  if (_flags.contains("aux"))
    outputAuxiliarySystemInformation();

  if (_flags.contains("nonlinear"))
    outputNonlinearSystemInformation();

  if (_flags.contains("execution"))
    outputExecutionInformation();

  if (_flags.contains("output"))
    outputOutputInformation();

  _console << "\n\n" << std::flush;
}


void
SimulationInformation::outputFrameworkInformation()
{
  std::stringstream oss;
  oss << std::left;

  if (_app.getSystemInfo() != NULL)
    oss << _app.getSystemInfo()->getInfo();

  if (_problem_ptr->legacyUoAuxComputation() || _problem_ptr->legacyUoInitialization())
  {
    oss << "LEGACY MODES ENABLED:\n";
    if (_problem_ptr->legacyUoAuxComputation())
      oss << "  Computing EXEC_LINEAR AuxKernel types when any UserObject type is executed.\n";
    if (_problem_ptr->legacyUoInitialization())
      oss << "  Computing all UserObjects during initial setup.\n";
  }

  oss << std::left << '\n'
      << "Parallelism:\n"
      << std::setw(_field_width) << "  Num Processors: " << static_cast<std::size_t>(n_processors()) << '\n'
      << std::setw(_field_width) << "  Num Threads: " << static_cast<std::size_t>(n_threads()) << '\n'
      << '\n';

  _console << oss.str();
}


void
SimulationInformation::outputMeshInformation()
{
  std::stringstream oss;
  oss << std::left;

  MooseMesh & moose_mesh = _problem_ptr->mesh();
  MeshBase & mesh = moose_mesh.getMesh();

  oss << "Mesh: " << '\n'
      << std::setw(_field_width) << "  Distribution: " << (moose_mesh.isParallelMesh() ? "parallel" : "serial")
      << (moose_mesh.isDistributionForced() ? " (forced) " : "") << '\n'
      << std::setw(_field_width) << "  Mesh Dimension: " << mesh.mesh_dimension() << '\n'
      << std::setw(_field_width) << "  Spatial Dimension: " << mesh.spatial_dimension() << '\n'
      << std::setw(_field_width) << "  Nodes:" << '\n'
      << std::setw(_field_width) << "    Total:" << mesh.n_nodes() << '\n'
      << std::setw(_field_width) << "    Local:" << mesh.n_local_nodes() << '\n'
      << std::setw(_field_width) << "  Elems:" << '\n'
      << std::setw(_field_width) << "    Total:" << mesh.n_elem() << '\n'
      << std::setw(_field_width) << "    Local:" << mesh.n_local_elem() << '\n'
      << std::setw(_field_width) << "  Num Subdomains: "       << static_cast<std::size_t>(mesh.n_subdomains()) << '\n'
      << std::setw(_field_width) << "  Num Partitions: "       << static_cast<std::size_t>(mesh.n_partitions()) << '\n';
  if (n_processors() > 1 && moose_mesh.partitionerName() != "")
    oss << std::setw(_field_width) << "  Partitioner: "       << moose_mesh.partitionerName()
        << (moose_mesh.isPartitionerForced() ? " (forced) " : "")
        << '\n';
  oss << '\n';

  _console << oss.str();
}


void
SimulationInformation::outputAuxiliarySystemInformation()
{
  std::string output = outputSystemInformationHelper(_problem_ptr->getAuxiliarySystem().system());
  if (!output.empty())
    _console << "Auxiliary System:" << '\n' << output;
}


void
SimulationInformation::outputNonlinearSystemInformation()
{
  std::string output = outputSystemInformationHelper(_problem_ptr->getNonlinearSystem().system());
  if (!output.empty())
    _console << "Nonlinear System:" << '\n' << output;
}


void
SimulationInformation::outputExecutionInformation()
{

  std::stringstream oss;
  oss << std::left;

  Executioner * exec = _app.actionWarehouse().executioner().get();

  oss << "Execution Information:\n"
      << std::setw(_field_width) << "  Executioner: " << demangle(typeid(*exec).name()) << '\n';

  std::string time_stepper = exec->getTimeStepperName();
  if (time_stepper != "")
    oss << std::setw(_field_width) << "  TimeStepper: " << time_stepper << '\n';

  oss << std::setw(_field_width) << "  Solver Mode: " << Moose::stringify<Moose::SolveType>(_problem_ptr->solverParams()._type) << '\n';

  const std::string & pc_desc = _problem_ptr->getPreconditionerDescription();
  if (!pc_desc.empty())
    oss << std::setw(_field_width) << "  Preconditioner: " << pc_desc << '\n';
  oss << '\n';

  _console << oss.str();
}


void
SimulationInformation::outputOutputInformation()
{
  std::stringstream oss;
  oss << std::left;

  const std::vector<Output *> & outputs = _app.getOutputWarehouse().all();
  oss << "Outputs:\n";
  for (std::vector<Output *>::const_iterator it = outputs.begin(); it != outputs.end(); ++it)
  {
    // Display the "output_on" settings
    const MultiMooseEnum & output_on = (*it)->outputOn();
    oss << "  " << std::setw(_field_width-2) << (*it)->name() <<  "\"" << output_on << "\"\n";

    // Display the advanced "output_on" settings, only if they are different from "output_on"
    if ((*it)->isAdvanced())
    {
      const OutputOnWarehouse & adv_on = (*it)->advancedOutputOn();
      for (std::map<std::string, MultiMooseEnum>::const_iterator adv_it = adv_on.begin(); adv_it != adv_on.end(); ++adv_it)
        if (output_on != adv_it->second)
          oss << "    " << std::setw(_field_width-4) << adv_it->first + ":" <<  "\"" << adv_it->second << "\"\n";
    }
  }

  _console << oss.str();
}


std::string
SimulationInformation::outputSystemInformationHelper(const System & system)
{
  std::stringstream oss;
  oss << std::left;

  if (system.n_dofs())
  {
    oss << std::setw(_field_width) << "  Num DOFs: " << system.n_dofs() << '\n'
        << std::setw(_field_width) << "  Num Local DOFs: " << system.n_local_dofs() << '\n';

    std::streampos begin_string_pos = oss.tellp();
    std::streampos curr_string_pos = begin_string_pos;
    oss << std::setw(_field_width) << "  Variables: ";
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
    oss << std::setw(_field_width) << "  Finite Element Types: ";
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
    oss << std::setw(_field_width) << "  Infinite Element Mapping: ";
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
    oss << std::setw(_field_width) << "  Approximation Orders: ";
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


void
SimulationInformation::insertNewline(std::stringstream &oss, std::streampos &begin, std::streampos &curr)
{
   if (curr - begin > _line_length)
   {
     oss << "\n";
     begin = oss.tellp();
     oss << std::setw(_field_width + 2) << "";  // "{ "
   }
}
