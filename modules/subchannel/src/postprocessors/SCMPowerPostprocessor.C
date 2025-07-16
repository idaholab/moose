//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMPowerPostprocessor.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "SCM.h"
#include "SubChannel1PhaseProblem.h"

registerMooseObject("SubChannelApp", SCMPowerPostprocessor);

InputParameters
SCMPowerPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Calculates the total power of the assembly based on the distribution of variable q_prime");
  return params;
}

SCMPowerPostprocessor::SCMPowerPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(SCM::getConstMesh<SubChannelMesh>(_fe_problem.mesh())),
    _value(0)
{
}

void
SCMPowerPostprocessor::execute()
{
  const auto scm_problem = dynamic_cast<SubChannel1PhaseProblem *>(&_fe_problem);
  const auto nz = _mesh.getNumOfAxialCells();
  const auto n_channels = _mesh.getNumOfChannels();
  if (!scm_problem)
    mooseError("SCMPowerPostprocessor can only be used within a subchannel 1phase problem.");
  auto power = 0.0;
  for (unsigned int iz = 1; iz < nz + 1; iz++)
    for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
      power += scm_problem->computeAddedHeatPin(i_ch, iz);
  _value = power;
}

Real
SCMPowerPostprocessor::getValue() const
{
  return _value;
}
