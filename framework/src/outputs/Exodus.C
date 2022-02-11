//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Exodus.h"

// Moose includes
#include "DisplacedProblem.h"
#include "ExodusFormatter.h"
#include "FEProblem.h"
#include "FileMesh.h"
#include "MooseApp.h"
#include "MooseVariableScalar.h"
#include "LockFile.h"

#include "libmesh/exodusII_io.h"

registerMooseObject("MooseApp", Exodus);

defineLegacyParams(Exodus);

InputParameters
Exodus::validParams()
{
  // Get the base class parameters
  InputParameters params = OversampleOutput::validParams();
  params +=
      AdvancedOutput::enableOutputTypes("nodal elemental scalar postprocessor reporter input");

  // Enable sequential file output (do not set default, the use_displace criteria relies on
  // isParamValid, see Constructor)
  params.addParam<bool>("sequence",
                        "Enable/disable sequential file output (enabled by default "
                        "when 'use_displace = true', otherwise defaults to false");

  // Select problem dimension for mesh output
  params.addDeprecatedParam<bool>("use_problem_dimension",
                                  "Use the problem dimension to the mesh output. "
                                  "Set to false when outputting lower dimensional "
                                  "meshes embedded in a higher dimensional space.",
                                  "Use 'output_dimension = problem_dimension' instead.");

  MooseEnum output_dimension("default 1 2 3 problem_dimension", "default");

  params.addParam<MooseEnum>(
      "output_dimension", output_dimension, "The dimension of the output file");

  params.addParamNamesToGroup("output_dimension", "Advanced");

  // Set the default padding to 3
  params.set<unsigned int>("padding") = 3;

  // Add description for the Exodus class
  params.addClassDescription("Object for output data in the Exodus II format");

  // Flag for overwriting at each timestep
  params.addParam<bool>("overwrite",
                        false,
                        "When true the latest timestep will overwrite the "
                        "existing file, so only a single timestep exists.");

  // Set outputting of the input to be on by default
  params.set<ExecFlagEnum>("execute_input_on") = EXEC_INITIAL;

  // Flag for outputting discontinuous data to Exodus
  params.addParam<bool>(
      "discontinuous", false, "Enables discontinuous output format for Exodus files.");

  // Need a layer of geometric ghosting for mesh serialization
  params.addRelationshipManager("MooseGhostPointNeighbors",
                                Moose::RelationshipManagerType::GEOMETRIC);

  // Return the InputParameters
  return params;
}

Exodus::Exodus(const InputParameters & parameters)
  : OversampleOutput(parameters),
    _exodus_initialized(false),
    _exodus_num(declareRestartableData<unsigned int>("exodus_num", 0)),
    _recovering(_app.isRecovering()),
    _exodus_mesh_changed(declareRestartableData<bool>("exodus_mesh_changed", true)),
    _sequence(isParamValid("sequence") ? getParam<bool>("sequence")
              : _use_displaced         ? true
                                       : false),
    _overwrite(getParam<bool>("overwrite")),
    _output_dimension(getParam<MooseEnum>("output_dimension").getEnum<OutputDimension>()),
    _discontinuous(getParam<bool>("discontinuous"))
{
  if (isParamValid("use_problem_dimension"))
  {
    auto use_problem_dimension = getParam<bool>("use_problem_dimension");

    if (use_problem_dimension)
      _output_dimension = OutputDimension::PROBLEM_DIMENSION;
    else
      _output_dimension = OutputDimension::DEFAULT;
  }
  // If user sets 'discontinuous = true' and 'elemental_as_nodal = false', issue an error that these
  // are incompatible states
  if (_discontinuous && parameters.isParamSetByUser("elemental_as_nodal") && !_elemental_as_nodal)
    mooseError(name(),
               ": Invalid parameters. 'elemental_as_nodal' set to false while 'discontinuous' set "
               "to true.");
  // At this point, if we have discontinuous ouput, we know the user hasn't explicitly set
  // 'elemental_as_nodal = false', so we can safely default it to true
  if (_discontinuous)
    _elemental_as_nodal = true;
}

void
Exodus::setOutputDimension(unsigned int /*dim*/)
{
  mooseDeprecated(
      "This method is no longer needed. We can determine output dimension programmatically");
}

void
Exodus::initialSetup()
{
  // Call base class setup method
  OversampleOutput::initialSetup();

  // The libMesh::ExodusII_IO will fail when it is closed if the object is created but
  // nothing is written to the file. This checks that at least something will be written.
  if (!hasOutput())
    mooseError("The current settings result in nothing being output to the Exodus file.");

  // Test that some sort of variable output exists (case when all variables are disabled but input
  // output is still enabled
  if (!hasNodalVariableOutput() && !hasElementalVariableOutput() && !hasPostprocessorOutput() &&
      !hasScalarOutput())
    mooseError("The current settings results in only the input file and no variables being output "
               "to the Exodus file, this is not supported.");
}

void
Exodus::meshChanged()
{
  // Maintain Oversample::meshChanged() functionality
  OversampleOutput::meshChanged();

  // Indicate to the Exodus object that the mesh has changed
  _exodus_mesh_changed = true;
}

void
Exodus::sequence(bool state)
{
  _sequence = state;
}

void
Exodus::outputSetup()
{
  if (_exodus_io_ptr)
  {
    // Do nothing if the ExodusII_IO objects exists, but has not been initialized
    if (!_exodus_initialized)
      return;

    // Do nothing if the output is using oversampling. In this case the mesh that is being output
    // has not been changed, so there is no need to create a new ExodusII_IO object
    if (_oversample || _change_position)
      return;

    // Do nothing if the mesh has not changed and sequential output is not desired
    if (!_exodus_mesh_changed && !_sequence)
      return;
  }

  auto serialize = [this](auto & moose_mesh)
  {
    auto & lm_mesh = moose_mesh.getMesh();
    // Exodus is serial output so that we have to gather everything to "zero".
    lm_mesh.gather_to_zero();
    // This makes the face information out-of-date on process 0 for distributed meshes, e.g.
    // elements will have neighbors that they didn't previously have
    if ((this->processor_id() == 0) && !lm_mesh.is_replicated())
      moose_mesh.faceInfoDirty();
  };
  serialize(_problem_ptr->mesh());

  // We need to do the same thing for displaced mesh to make them consistent.
  // In general, it is a good idea to make the reference mesh and the displaced mesh
  // consistent since some operations or calculations are already based on this assumption.
  // For example,
  // FlagElementsThread::onElement(const Elem * elem)
  //   if (_displaced_problem)
  //    _displaced_problem->mesh().elemPtr(elem->id())->set_refinement_flag((Elem::RefinementState)marker_value);
  // Here we assume that the displaced mesh and the reference mesh are identical except
  // coordinations.
  if (_problem_ptr->getDisplacedProblem())
    serialize(_problem_ptr->getDisplacedProblem()->mesh());

  // Create the ExodusII_IO object
  _exodus_io_ptr = std::make_unique<ExodusII_IO>(_es_ptr->get_mesh());
  _exodus_initialized = false;

  // Increment file number and set appending status, append if all the following conditions are met:
  //   (1) If the application is recovering (not restarting)
  //   (2) The mesh has NOT changed
  //   (3) An existing Exodus file exists for appending (_exodus_num > 0)
  //   (4) Sequential output is NOT desired
  if (_recovering && !_exodus_mesh_changed && _exodus_num > 0 && !_sequence)
  {
    // Set the recovering flag to false so that this special case is not triggered again
    _recovering = false;

    // Set the append flag to true b/c on recover the file is being appended
    _exodus_io_ptr->append(true);
  }
  else
  {
    // Increment file counter
    if (_exodus_mesh_changed || _sequence)
      _file_num++;

    // Disable file appending and reset exodus file number count
    _exodus_io_ptr->append(false);
    _exodus_num = 1;
  }

  setOutputDimensionInExodusWriter(*_exodus_io_ptr, *_mesh_ptr, _output_dimension);
}

void
Exodus::setOutputDimensionInExodusWriter(ExodusII_IO & exodus_io,
                                         const MooseMesh & mesh,
                                         OutputDimension output_dimension)
{
  switch (output_dimension)
  {
    case OutputDimension::DEFAULT:
      // If the mesh_dimension is 1, we need to write out as 3D.
      //
      // This works around an issue in Paraview where 1D meshes cannot
      // not be visualized correctly. Otherwise, write out based on the effectiveSpatialDimension.
      if (mesh.getMesh().mesh_dimension() == 1)
        exodus_io.write_as_dimension(3);
      else
        exodus_io.write_as_dimension(static_cast<int>(mesh.effectiveSpatialDimension()));
      break;

    case OutputDimension::ONE:
    case OutputDimension::TWO:
    case OutputDimension::THREE:
      exodus_io.write_as_dimension(static_cast<int>(output_dimension));
      break;

    case OutputDimension::PROBLEM_DIMENSION:
      exodus_io.use_mesh_dimension_instead_of_spatial_dimension(true);
      break;

    default:
      ::mooseError("Unknown output_dimension in Exodus writer");
  }
}

void
Exodus::outputNodalVariables()
{
  // Set the output variable to the nodal variables
  std::vector<std::string> nodal(getNodalVariableOutput().begin(), getNodalVariableOutput().end());
  _exodus_io_ptr->set_output_variables(nodal);

  // Write the data via libMesh::ExodusII_IO
  if (_discontinuous)
    _exodus_io_ptr->write_timestep_discontinuous(
        filename(), *_es_ptr, _exodus_num, time() + _app.getGlobalTimeOffset());
  else
    _exodus_io_ptr->write_timestep(
        filename(), *_es_ptr, _exodus_num, time() + _app.getGlobalTimeOffset());

  if (!_overwrite)
    _exodus_num++;

  // This satisfies the initialization of the ExodusII_IO object
  _exodus_initialized = true;
}

void
Exodus::outputElementalVariables()
{
  // Make sure the the file is ready for writing of elemental data
  if (!_exodus_initialized || !hasNodalVariableOutput())
    outputEmptyTimestep();

  // Write the elemental data
  std::vector<std::string> elemental(getElementalVariableOutput().begin(),
                                     getElementalVariableOutput().end());
  _exodus_io_ptr->set_output_variables(elemental);
  _exodus_io_ptr->write_element_data(*_es_ptr);
}

void
Exodus::outputPostprocessors()
{
  // List of desired postprocessor outputs
  const std::set<std::string> & pps = getPostprocessorOutput();

  // Append the postprocessor data to the global name value parameters; scalar outputs
  // also append these member variables
  for (const auto & name : pps)
  {
    _global_names.push_back(name);
    _global_values.push_back(_problem_ptr->getPostprocessorValueByName(name));
  }
}

void
Exodus::outputReporters()
{
  for (const auto & combined_name : getReporterOutput())
  {
    ReporterName r_name(combined_name);
    if (_reporter_data.hasReporterValue<Real>(r_name) &&
        !hasPostprocessorByName(r_name.getObjectName()))
    {
      const Real & value = _reporter_data.getReporterValue<Real>(r_name);
      _global_names.push_back(r_name.getValueName());
      _global_values.push_back(value);
    }
  }
}

void
Exodus::outputScalarVariables()
{
  // List of desired scalar outputs
  const std::set<std::string> & out = getScalarOutput();

  // Append the scalar to the global output lists
  for (const auto & out_name : out)
  {
    // Make sure scalar values are in sync with the solution vector
    // and are visible on this processor.  See TableOutput.C for
    // TableOutput::outputScalarVariables() explanatory comments

    MooseVariableScalar & scalar_var = _problem_ptr->getScalarVariable(0, out_name);
    scalar_var.reinit();
    VariableValue value(scalar_var.sln());

    const std::vector<dof_id_type> & dof_indices = scalar_var.dofIndices();
    const unsigned int n = dof_indices.size();
    value.resize(n);

    const DofMap & dof_map = scalar_var.sys().dofMap();
    for (unsigned int i = 0; i != n; ++i)
    {
      const processor_id_type pid = dof_map.dof_owner(dof_indices[i]);
      this->comm().broadcast(value[i], pid);
    }

    // If the scalar has a single component, output the name directly
    if (n == 1)
    {
      _global_names.push_back(out_name);
      _global_values.push_back(value[0]);
    }

    // If the scalar as many components add indices to the end of the name
    else
    {
      for (unsigned int i = 0; i < n; ++i)
      {
        std::ostringstream os;
        os << out_name << "_" << i;
        _global_names.push_back(os.str());
        _global_values.push_back(value[i]);
      }
    }
  }
}

void
Exodus::outputInput()
{
  // Format the input file
  ExodusFormatter syntax_formatter;
  syntax_formatter.printInputFile(_app.actionWarehouse());
  syntax_formatter.format();

  // Store the information
  _input_record = syntax_formatter.getInputFileRecord();
}

void
Exodus::output(const ExecFlagType & type)
{
  // Prepare the ExodusII_IO object
  outputSetup();
  LockFile lf(filename(), processor_id() == 0);

  // Adjust the position of the output
  if (_app.hasOutputPosition())
    _exodus_io_ptr->set_coordinate_offset(_app.getOutputPosition());

  // Clear the global variables (postprocessors and scalars)
  _global_names.clear();
  _global_values.clear();

  // Call the individual output methods
  AdvancedOutput::output(type);

  // Write the global variables (populated by the output methods)
  if (!_global_values.empty())
  {
    if (!_exodus_initialized)
      outputEmptyTimestep();
    _exodus_io_ptr->write_global_data(_global_values, _global_names);
  }

  // Write the input file record if it exists and the output file is initialized
  if (!_input_record.empty() && _exodus_initialized)
  {
    _exodus_io_ptr->write_information_records(_input_record);
    _input_record.clear();
  }

  // Reset the mesh changed flag
  _exodus_mesh_changed = false;

  // It is possible to have an empty file created with the following scenario. By default the
  // 'execute_on_input' flag is setup to run on INITIAL. If the 'execute_on' is set to FINAL
  // but the simulation stops early (e.g., --half-transient) the Exodus file is created but there
  // is no data in it, because of the initial call to write the input data seems to create the file
  // but doesn't actually write the data into the solution/mesh is also supplied to the IO object.
  // Then if --recover is used this empty file fails to open for appending.
  //
  // The code below will delete any empty files that exist. Another solution is to set the
  // 'execute_on_input' flag to NONE.
  std::string current = filename();
  if (processor_id() == 0 && MooseUtils::checkFileReadable(current, false, false) &&
      (MooseUtils::fileSize(current) == 0))
  {
    int err = std::remove(current.c_str());
    if (err != 0)
      mooseError("MOOSE failed to remove the empty file ", current);
  }
}

std::string
Exodus::filename()
{
  // Append the .e extension on the base file name
  std::ostringstream output;
  output << _file_base + ".e";

  // Add the -s00x extension to the file
  if (_file_num > 1)
    output << "-s" << std::setw(_padding) << std::setprecision(0) << std::setfill('0') << std::right
           << _file_num;

  // Return the filename
  return output.str();
}

void
Exodus::outputEmptyTimestep()
{
  // Write a timestep with no variables
  _exodus_io_ptr->set_output_variables(std::vector<std::string>());
  _exodus_io_ptr->write_timestep(
      filename(), *_es_ptr, _exodus_num, time() + _app.getGlobalTimeOffset());

  if (!_overwrite)
    _exodus_num++;

  _exodus_initialized = true;
}

void
Exodus::clear()
{
  _exodus_io_ptr.reset();
}
