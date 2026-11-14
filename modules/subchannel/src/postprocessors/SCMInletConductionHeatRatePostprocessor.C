//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMInletConductionHeatRatePostprocessor.h"
#include "FEProblemBase.h"
#include "SolutionHandle.h"
#include "MooseMesh.h"
#include "SCM.h"
#include "SinglePhaseFluidProperties.h"
#include "SubChannel1PhaseProblem.h"
#include "TriSubChannel1PhaseProblem.h"

registerMooseObject("SubChannelApp", SCMInletConductionHeatRatePostprocessor);

InputParameters
SCMInletConductionHeatRatePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Calculates the heat rate $[W]$ that axial conduction carries out of the assembly through "
      "the inlet, against the flow. It closes the energy balance between the pin and duct heat "
      "rates and the enthalpy flow rate rise of SCMTHPowerPostprocessor.");
  return params;
}

SCMInletConductionHeatRatePostprocessor::SCMInletConductionHeatRatePostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(SCM::getConstMesh<SubChannelMesh>(_fe_problem.mesh())),
    _value(0)
{
  // The explicit solve conducts temperature with its own stencil, which differs from the implicit
  // enthalpy stencil this postprocessor evaluates, so its result would not be the solver's flux
  const auto scm_problem = dynamic_cast<const SubChannel1PhaseProblem *>(&_fe_problem);
  if (scm_problem && !scm_problem->isImplicit())
    mooseError("SCMInletConductionHeatRatePostprocessor supports only the implicit solve "
               "('implicit = true').");
}

void
SCMInletConductionHeatRatePostprocessor::execute()
{
  const auto scm_problem = dynamic_cast<SubChannel1PhaseProblem *>(&_fe_problem);
  if (!scm_problem)
    mooseError("SCMInletConductionHeatRatePostprocessor can only be used within a subchannel "
               "1phase problem.");

  // Only the triangular assembly assembles axial heat conduction. A quadrilateral assembly has no
  // conduction term, so none of its heat leaves through the inlet this way.
  if (!dynamic_cast<TriSubChannel1PhaseProblem *>(&_fe_problem))
  {
    _value = 0.0;
    return;
  }

  const auto n_channels = _mesh.getNumOfChannels();
  const auto & z_grid = _mesh.getZGrid();
  const auto * fp = scm_problem->getSinglePhaseFluidProperties();
  auto h_soln = SolutionHandle(_fe_problem.getVariable(0, "h"));
  auto T_soln = SolutionHandle(_fe_problem.getVariable(0, "T"));
  auto P_soln = SolutionHandle(_fe_problem.getVariable(0, "P"));
  auto S_soln = SolutionHandle(_fe_problem.getVariable(0, "S"));
  const Real P_out = scm_problem->getOutletPressure();

  // The thermal diffusivity k / cp of the enthalpy equation, evaluated as the solver does
  auto diffusivity = [&](const Node * node)
  {
    const Real p = P_soln(node) + P_out;
    const Real T = T_soln(node);
    return fp->k_from_p_T(p, T) / fp->cp_from_p_T(p, T);
  };

  // The conduction flux of the first cell, with the same face interpolation the enthalpy assembly
  // uses. A positive value leaves the assembly, since the first node is hotter than the prescribed
  // inlet.
  Real heat_rate = 0.0;
  for (const auto i_ch : make_range(n_channels))
  {
    const auto * node_in = _mesh.getChannelNode(i_ch, 0);
    const auto * node_out = _mesh.getChannelNode(i_ch, 1);
    const Real dz = z_grid[1] - z_grid[0];
    const Real S_face = scm_problem->interpolateFaceValue(S_soln(node_out), S_soln(node_in));
    const Real diff_face =
        scm_problem->interpolateFaceValue(diffusivity(node_out), diffusivity(node_in));
    heat_rate += diff_face * S_face * (h_soln(node_out) - h_soln(node_in)) / dz;
  }

  _value = heat_rate;
}

void
SCMInletConductionHeatRatePostprocessor::finalize()
{
  // The subchannel solution is only available on processor 0
  _communicator.broadcast(_value);
}

Real
SCMInletConductionHeatRatePostprocessor::getValue() const
{
  return _value;
}
