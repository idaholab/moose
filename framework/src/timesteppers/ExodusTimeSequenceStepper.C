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

#include "ExodusTimeSequenceStepper.h"
#include "MooseUtils.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/exodusII_io.h"

template <>
InputParameters
validParams<ExodusTimeSequenceStepper>()
{
  InputParameters params = validParams<TimeSequenceStepperBase>();
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
