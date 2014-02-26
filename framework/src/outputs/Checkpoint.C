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
#include <sys/stat.h>

// Moose includes
#include "Checkpoint.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/checkpoint_io.h"

template<>
InputParameters validParams<Checkpoint>()
{
  // Get the parameters from the base classes
  InputParameters params = validParams<OutputBase>();
  params += validParams<FileOutputInterface>();

  // Typical checkpoint options
  params.addParam<unsigned int>("num_files", 2, "Number of the restart files to save");
  params.addParam<std::string>("suffix", "cp", "This will be appended to the file_base to create the directory name for checkpoint files.");

  // Advanced settings
  params.addParam<bool>("binary", true, "Toggle the output of binary files");
  params.addParamNamesToGroup("binary", "Advanced");

  // Checkpoint files always output everything, so suppress the toggles
  params.suppressParameter<bool>("output_nodal_variables");
  params.suppressParameter<bool>("output_elemental_variables");
  params.suppressParameter<bool>("output_scalar_variables");
  params.suppressParameter<bool>("output_postprocessors");

  return params;
}

Checkpoint::Checkpoint(const std::string & name, InputParameters & parameters) :
    OutputBase(name, parameters),
    FileOutputInterface(name, parameters),
    _num_files(getParam<unsigned int>("num_files")),
    _suffix(getParam<std::string>("suffix")),
    _binary(getParam<bool>("binary")),
    _restartable_data(_problem_ptr->getRestartableData()),
    _recoverable_data(_problem_ptr->getRecoverableData()),
    _material_property_storage(_problem_ptr->getMaterialPropertyStorage()),
    _material_property_io(MaterialPropertyIO(*_problem_ptr)),
    _restartable_data_io(RestartableDataIO(*_problem_ptr))
{
}

Checkpoint::~Checkpoint()
{
}

std::string
Checkpoint::filename()
{
  // Get the time step with correct zero padding
  std::ostringstream output;
  output << std::setw(_padding)
         << std::setprecision(0)
         << std::setfill('0')
         << std::right
         << _t_step;
  return output.str();
}

std::string
Checkpoint::directory()
{
  return _file_base + "_" + _suffix;
}

void
Checkpoint::output()
{

  // Start the performance log
  Moose::perf_log.push("output()", "Checkpoint");

  // Create the output directory
  std::string cp_dir = directory();
  mkdir(cp_dir.c_str(),  S_IRWXU | S_IRGRP);

  // Create the output filename
  std::string current_file = cp_dir + "/" + filename();

  // Create the libMesh Checkpoint_IO object
  MeshBase & mesh = _es_ptr->get_mesh();
  CheckpointIO io(mesh, _binary);

  // Set renumbering flag (renumber if adaptivity is on)
  bool renumber = false;
  if(_problem_ptr->adaptivity().isOn())
    renumber = true;

  // Create checkpoint file structure
  CheckpointFileNames current_file_struct;
  if (_binary)
  {
    current_file_struct.checkpoint = current_file + "_mesh.cpr";
    current_file_struct.system = current_file + ".xdr";
  }
  else
  {
    current_file_struct.checkpoint = current_file + "_mesh.cpa";
    current_file_struct.system = current_file + ".xda";
  }
  current_file_struct.restart = current_file + ".rd";
  current_file_struct.material = current_file + ".msmp";

  // Write the checkpoint file
  io.write(current_file_struct.checkpoint);

  // Write the xdr
  _es_ptr->write(current_file_struct.system, libMeshEnums::ENCODE, EquationSystems::WRITE_DATA | EquationSystems::WRITE_ADDITIONAL_DATA | EquationSystems::WRITE_PARALLEL_FILES, renumber);

  // Write the restartable data
  _restartable_data_io.writeRestartableData(current_file_struct.restart, _restartable_data, _recoverable_data);

  // Write the material property data
  if (_material_property_storage.hasStatefulProperties())
    _material_property_io.write(current_file_struct.material);

  // Remove old checkpoint files
  updateCheckpointFiles(current_file_struct);

  // Stop the logging
  Moose::perf_log.pop("output()", "Checkpoint");
}

void
Checkpoint::updateCheckpointFiles(CheckpointFileNames file_struct)
{

  // Update the list of stored files
  _file_names.push_back(file_struct);

  // Remove un-wanted files
  if (_file_names.size() > _num_files)
  {
    // Extract the filenames to be removed
    CheckpointFileNames delete_files = _file_names[0];

    // Remove these filenames from the list
    _file_names.erase(_file_names.begin());

    // Get thread and proc information
    unsigned int n_threads = libMesh::n_threads();
    processor_id_type proc_id = libMesh::processor_id();

    // Delete checkpoint files (_mesh.cpr)
    remove(delete_files.checkpoint.c_str());

    // Delete the system files (xdr and xdr.0000, ...)
    remove(delete_files.system.c_str());
    {
      std::ostringstream oss;
      oss << delete_files.system
          << "." << std::setw(4)
          << std::setprecision(0)
          << std::setfill('0')
          << proc_id;
      remove( oss.str().c_str() );
    }

    // Remove material property files
    remove(delete_files.material.c_str());

    // Remove the material files
    {
      std::ostringstream oss;
      oss << delete_files.material << '-' << proc_id;
      remove(oss.str().c_str());
    }

    // Remove the restart files (rd)
    {
      std::ostringstream oss;
      oss << delete_files.restart << "-" << proc_id;
      remove(oss.str().c_str());
    }
  }
}

void
Checkpoint::outputNodalVariables()
{
  mooseError("Invalid for Checkpoint output type");
}

void
Checkpoint::outputElementalVariables()
{
  mooseError("Invalid for Checkpoint output type");
}

void
Checkpoint::outputScalarVariables()
{
  mooseError("Invalid for Checkpoint output type");
}

void
Checkpoint::outputPostprocessors()
{
  mooseError("Invalid for Checkpoint output type");
}
