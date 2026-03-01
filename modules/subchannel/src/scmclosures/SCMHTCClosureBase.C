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
  : SCMClosureBase(parameters), _Dpin_soln(_subproblem.getVariable(0, "Dpin"))
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
    const auto * pin_node = _subchannel_mesh.getPinNode(nusselt_args.i_pin, nusselt_args.iz);
    if (_Dpin_soln(pin_node) > 0)
      D = _Dpin_soln(pin_node);
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
                        ? 3.73
                        : (info.subch_type == EChannelType::EDGE ? 3.59 : 3.52);
  /// transient range Re limits-Updated-Cheng-Todreas 2018
  info.ReL = 320 * std::pow(10.0, (info.poD - 1.0));
  info.ReT = 1e4 * std::pow(10.0, 0.7 * (info.poD - 1.0));

  return info;
}

Real
SCMHTCClosureBase::computeHTC(const FrictionStruct & friction_args,
                              const NusseltStruct & nusselt_args,
                              const Real k) const
{
  // Compute HTC
  auto Nu = computeNusseltNumber(friction_args, nusselt_args);
  auto Dh_i = 4.0 * friction_args.S / friction_args.w_perim;
  return Nu * k / Dh_i;
}
