//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExodusTimeSequenceStepper.h"
#include "MooseUtils.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/exodusII_io.h"

using namespace libMesh;

registerMooseObject("MooseApp", ExodusTimeSequenceStepper);

InputParameters
ExodusTimeSequenceStepper::validParams()
{
  InputParameters params = TimeSequenceStepperBase::validParams();
  params.addRequiredParam<MeshFileName>(
      "mesh",
      "The name of the mesh file to extract the time sequence from (must be an exodusII file).");
  params.addClassDescription("Solves the Transient problem at a sequence of time points taken from "
                             "a specified exodus file.");
  return params;
}

ExodusTimeSequenceStepper::ExodusTimeSequenceStepper(const InputParameters & parameters)
  : TimeSequenceStepperBase(parameters), _mesh_file(getParam<MeshFileName>("mesh"))
{
  // Read the Exodus file on processor 0
  std::vector<Real> times;
  if (processor_id() == 0)
  {
    // Check that the required file exists
    MooseUtils::checkFileReadable(_mesh_file);

    // dummy mesh
    ReplicatedMesh mesh(_communicator);

    ExodusII_IO exodusII_io(mesh);
    exodusII_io.read(_mesh_file);
    times = exodusII_io.get_time_steps();
  }

  // distribute timestep list
  unsigned int num_steps = times.size();
  _communicator.broadcast(num_steps);
  times.resize(num_steps);
  _communicator.broadcast(times);

  setupSequence(times);
}
