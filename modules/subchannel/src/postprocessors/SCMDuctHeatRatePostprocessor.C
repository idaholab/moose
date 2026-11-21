//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMDuctHeatRatePostprocessor.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "SCM.h"
#include "SubChannel1PhaseProblem.h"

registerMooseObject("SubChannelApp", SCMDuctHeatRatePostprocessor);

InputParameters
SCMDuctHeatRatePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Calculates the heat flow that goes into the coolant through the duct "
                             "based on aux variable duct_heat_flux");
  return params;
}

SCMDuctHeatRatePostprocessor::SCMDuctHeatRatePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(SCM::getConstMesh<SubChannelMesh>(_fe_problem.mesh())),
    _scm_problem(SCM::getConstProblem<SubChannel1PhaseProblem>(_fe_problem)),
    _value(0)
{
}

void
SCMDuctHeatRatePostprocessor::execute()
{
  const auto nz = _mesh.getNumOfAxialCells();
  const auto n_channels = _mesh.getNumOfChannels();
  auto power = 0.0;
  for (unsigned int iz = 1; iz < nz + 1; iz++)
    for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
      power += _scm_problem.getAddedHeatDuct(i_ch, iz);
  _value = power;
}

void
SCMDuctHeatRatePostprocessor::finalize()
{
  // The subchannel solution is only available on processor 0
  _communicator.broadcast(_value);
}

Real
SCMDuctHeatRatePostprocessor::getValue() const
{
  return _value;
}
