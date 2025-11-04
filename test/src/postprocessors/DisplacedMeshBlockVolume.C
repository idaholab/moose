//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DisplacedMeshBlockVolume.h"

#include "DisplacedProblem.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", DisplacedMeshBlockVolume);

InputParameters
DisplacedMeshBlockVolume::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  return params;
}

DisplacedMeshBlockVolume::DisplacedMeshBlockVolume(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _mesh(_fe_problem.getDisplacedProblem()->mesh())
{
}

void
DisplacedMeshBlockVolume::meshDisplaced()
{
  _volume = 0;
  for (const auto & elem : _mesh.getMesh().active_local_element_ptr_range())
    _volume += elem->volume();
  gatherSum(_volume);

  _console << "Displaced mesh block volume " << _volume << std::endl;
}
