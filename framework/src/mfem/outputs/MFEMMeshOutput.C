//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMeshOutput.h"

registerMooseObject("MooseApp", MFEMMeshOutput);

InputParameters
MFEMMeshOutput::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addClassDescription("Output for controlling MFEMMeshOutput inherited data.");
  params.addParam<std::string>("submesh",
                               "Submesh to output variables on. Leave blank to use base mesh.");
  MooseEnum ordering("NONE HILBERT GECKO", "NONE", false);
  params.addParam<MooseEnum>("ordering", ordering, "Whether to reorder the elements of the mesh. Options are NONE to do nothing, HILBERT to perform a spatial sort on the elements so theyappriximately follow the Hilbert curve, and GECKO to use the Gecko library to order elements for increased memory coherence.");
  params.addParam<int>("precision", 16, "Number of digits to use with ASCII output.");
  // FIXME: Allow users to specify precision
  return params;
}

MFEMMeshOutput::MFEMMeshOutput(const InputParameters & parameters)
  : FileOutput(parameters),
    _pmesh(parameters.isParamValid("submesh")
           ? static_cast<MFEMProblem *>(_problem_ptr)->getProblemData().submeshes.GetRef(getParam<std::string>("submesh"))
           : static_cast<MFEMProblem *>(_problem_ptr)->mfemParMesh()),
    _ordering(getParam<MooseEnum>("ordering")),
    _precision(getParam<int>("precision"))
{
}

std::string MFEMMeshOutput::filename() {
  std::ostringstream output;
  output << _file_base << ".mesh";

  // Add the _000x extension to the file
  if (_file_num > 1)
    output << "-s" << std::setw(_padding) << std::setprecision(0) << std::setfill('0') << std::right
           << _file_num;

  // Return the filename
  return output.str();
}

void
MFEMMeshOutput::output()
{
  constexpr int save_rank = 0;
  mfem::Mesh serial_mesh = _pmesh.GetSerialMesh(save_rank);

  if (_ordering > 0) {
    mfem::Array<int> new_order;
    if (_ordering == 1) {
      serial_mesh.GetHilbertElementOrdering(new_order);
    }
    else {
      // FIXME: Add support for various Gecko element ordering configs
      serial_mesh.GetGeckoElementOrdering(new_order);
    }
    serial_mesh.ReorderElements(new_order);
  }

  if (processor_id() == save_rank) {
    serial_mesh.Save(filename().c_str(), _precision);
  }
}

#endif
