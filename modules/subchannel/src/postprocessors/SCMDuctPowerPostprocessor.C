//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMDuctPowerPostprocessor.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "SCM.h"
#include "SubChannel1PhaseProblem.h"

registerMooseObject("SubChannelApp", SCMDuctPowerPostprocessor);

InputParameters
SCMDuctPowerPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Calculates the power that goes into the coolant from the duct "
                             "based on aux variable duct_heat_flux");
  return params;
}

SCMDuctPowerPostprocessor::SCMDuctPowerPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(SCM::getConstMesh<SubChannelMesh>(_fe_problem.mesh())),
    _value(0)
{
}

void
SCMDuctPowerPostprocessor::execute()
{
  const auto scm_problem = dynamic_cast<SubChannel1PhaseProblem *>(&_fe_problem);
  const auto nz = _mesh.getNumOfAxialCells();
  const auto n_channels = _mesh.getNumOfChannels();
  if (!scm_problem)
    mooseError("SCMDuctPowerPostprocessor can only be used within a subchannel 1phase problem.");
  auto power = 0.0;
  for (unsigned int iz = 1; iz < nz + 1; iz++)
    for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
      power += scm_problem->computeAddedHeatDuct(i_ch, iz);
  _value = power;
}

Real
SCMDuctPowerPostprocessor::getValue() const
{
  return _value;
}
