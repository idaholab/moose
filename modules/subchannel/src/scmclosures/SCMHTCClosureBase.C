//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMHTCClosureBase.h"
#include "SCM.h"

InputParameters
SCMHTCClosureBase::validParams()
{
  InputParameters params = SCMClosureBase::validParams();
  return params;
}

SCMHTCClosureBase::SCMHTCClosureBase(const InputParameters & parameters)
  : SCMClosureBase(parameters)
{
}

NusseltPreInfo
SCMHTCClosureBase::computeNusseltNumberPreInfo(const NusseltStruct & nusselt_args) const
{
  NusseltPreInfo info;
  info.Re = nusselt_args.Re;
  info.Pr = nusselt_args.Pr;

  const auto pitch = _subchannel_mesh.getPitch();
  Real D;
  const bool is_duct = (nusselt_args.i_pin == std::numeric_limits<unsigned int>::max());
  if (!is_duct)
  {
    auto Dpin_soln = SolutionHandle(_subproblem.getVariable(0, "Dpin"));
    const auto * pin_node = _subchannel_mesh.getPinNode(nusselt_args.i_pin, nusselt_args.iz);
    if (Dpin_soln(pin_node) > 0)
      D = Dpin_soln(pin_node);
    else
      mooseError(name(),
                 "The diameter of the pin is equal or smaller than zero, "
                 "please initialize the auxiliary variable Dpin.");
  }
  else
    D = _subchannel_mesh.getPinDiameter();

  info.poD = pitch / D;
  info.subch_type = _subchannel_mesh.getSubchannelType(nusselt_args.i_ch);

  info.laminar_Nu = (info.subch_type == EChannelType::CENTER)
                        ? 4.0
                        : (info.subch_type == EChannelType::EDGE ? 3.7 : 3.3);

  info.ReL = 300 * std::pow(10.0, 1.7 * (info.poD - 1.0));
  info.ReT = 1e4 * std::pow(10.0, 1.7 * (info.poD - 1.0));

  return info;
}
