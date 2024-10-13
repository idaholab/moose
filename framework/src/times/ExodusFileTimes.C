//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExodusFileTimes.h"
#include "MooseUtils.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/exodusII_io.h"

registerMooseObject("MooseApp", ExodusFileTimes);

InputParameters
ExodusFileTimes::validParams()
{
  InputParameters params = Times::validParams();
  params.addClassDescription("Import times from one or more Exodus files.");
  params.addRequiredParam<std::vector<FileName>>("files", "Exodus file(s) with the times");
  // File is loaded only on zeroth process
  params.set<bool>("auto_broadcast") = true;

  return params;
}

ExodusFileTimes::ExodusFileTimes(const InputParameters & parameters) : Times(parameters)
{
  // Reading exodus files on all ranks could be expensive
  const auto & times_files = getParam<std::vector<FileName>>("files");

  if (processor_id() == 0)
  {
    for (const auto p_file_it : index_range(times_files))
    {
      // Check that the required file exists
      MooseUtils::checkFileReadable(times_files[p_file_it]);

      // dummy mesh
      ReplicatedMesh mesh(_communicator);

      libMesh::ExodusII_IO exodusII_io(mesh);
      exodusII_io.read(times_files[p_file_it]);
      auto & times = exodusII_io.get_time_steps();

      for (const auto & d : times)
        _times.push_back(d);
    }
  }

  // Sort all the times, but also broadcast to other ranks
  finalize();
}
