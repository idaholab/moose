//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Nemesis.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

#include "libmesh/dof_map.h"
#include "libmesh/nemesis_io.h"

using namespace libMesh;

registerMooseObject("MooseApp", Nemesis);

InputParameters
Nemesis::validParams()
{
  InputParameters params = AdvancedOutput::validParams();
  params += AdvancedOutput::enableOutputTypes("scalar postprocessor input");
  params.addParam<bool>("write_hdf5", false, "Enables HDF5 output format for Nemesis files.");
  params.addClassDescription("Object for output data in the Nemesis (parallel ExodusII) format.");
  return params;
}

Nemesis::Nemesis(const InputParameters & parameters)
  : AdvancedOutput(parameters),
    _nemesis_io_ptr(nullptr),
    _file_num(declareRestartableData<unsigned int>("nemesis_file_num", 0)),
    _nemesis_num(declareRestartableData<unsigned int>("nemesis_num", 0)),
    _nemesis_initialized(false),
    _recovering(_app.isRecovering()),
    _nemesis_mesh_changed(declareRestartableData<bool>("nemesis_mesh_changed", true)),
    _write_hdf5(getParam<bool>("write_hdf5"))
{
}

void
Nemesis::initialSetup()
{
  AdvancedOutput::initialSetup();
}

void
Nemesis::meshChanged()
{
  // Do not delete the Nemesis_IO object if it has not been used; also there is no need to setup
  // the object in this case, so just return
  if (_nemesis_io_ptr != nullptr && !_nemesis_initialized)
    return;

  // Indicate to the Nemesis object that the mesh has changed
  _nemesis_mesh_changed = true;
}

void
Nemesis::outputSetup()
{
  if (_nemesis_io_ptr)
  {
    // Do nothing if the Nemesis_IO objects exists, but has not been initialized
    if (!_nemesis_initialized)
      return;

    // Do nothing if the mesh has not changed
    if (!_nemesis_mesh_changed)
      return;
  }

  // Create the new NemesisIO object
  _nemesis_io_ptr = std::make_unique<Nemesis_IO>(_problem_ptr->mesh().getMesh());
  _nemesis_initialized = false;

  if (_write_hdf5)
  {
#ifndef LIBMESH_HAVE_HDF5
    mooseError("Moose input requested HDF Nemesis output, but libMesh was built without HDF5.");
#endif

    // This is redundant unless the libMesh default changes
    _nemesis_io_ptr->set_hdf5_writing(true);
  }
  else
  {
    _nemesis_io_ptr->set_hdf5_writing(false);
  }

  if (_recovering && !_nemesis_mesh_changed && _nemesis_num > 0)
  {
    // Set the recovering flag to false so that this special case is not triggered again
    _recovering = false;

    // Set the append flag to true b/c on recover the file is being appended
    _nemesis_io_ptr->append(true);
  }
  else
  {
    // Increment file counter
    if (_nemesis_mesh_changed)
      _file_num++;

    // Disable file appending and reset nemesis file number count
    _nemesis_io_ptr->append(false);
    _nemesis_num = 1;
  }
}

void
Nemesis::outputPostprocessors()
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
Nemesis::outputScalarVariables()
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
Nemesis::output()
{
  outputSetup();

  // Clear the global variables (postprocessors and scalars)
  _global_names.clear();
  _global_values.clear();

  // Call the output methods
  AdvancedOutput::output();

  // Set up the whitelist of nodal variable names to write.
  _nemesis_io_ptr->set_output_variables(
      std::vector<std::string>(getNodalVariableOutput().begin(), getNodalVariableOutput().end()));

  // Write nodal data
  _nemesis_io_ptr->write_timestep(
      filename(), *_es_ptr, _nemesis_num, getOutputTime() + _app.getGlobalTimeOffset());
  _nemesis_initialized = true;

  // Write elemental data
  std::vector<std::string> elemental(getElementalVariableOutput().begin(),
                                     getElementalVariableOutput().end());
  _nemesis_io_ptr->set_output_variables(elemental);
  _nemesis_io_ptr->write_element_data(*_es_ptr);

  // Increment output call counter for the current file
  _nemesis_num++;

  // Write the global variables (populated by the output methods)
  if (!_global_values.empty())
    _nemesis_io_ptr->write_global_data(_global_values, _global_names);

  // Reset the mesh changed flag
  _nemesis_mesh_changed = false;
}

std::string
Nemesis::filename()
{
  // Append the .e extension on the base file name
  std::ostringstream output;
  output << _file_base << ".e";

  // Add the _000x extension to the file
  if (_file_num > 1)
    output << "-s" << std::setw(_padding) << std::setprecision(0) << std::setfill('0') << std::right
           << _file_num;

  // Return the filename
  return output.str();
}
