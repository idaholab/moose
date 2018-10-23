//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluxLimiter.h"
#include "NonlinearSystem.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("PorousFlowApp", FluxLimiter);

template <>
InputParameters
validParams<FluxLimiter>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Kuzmin-Turek flux limiting stuff.  ONLY WORKS FOR 1D");
  params.addRequiredCoupledVar("u", "The variable that is being advected");
  MooseEnum flux_limiter_type("MinMod VanLeer MC superbee None", "MinMod");
  params.addParam<MooseEnum>("flux_limiter_type", flux_limiter_type, "Type of flux limiter to use");
  return params;
}

FluxLimiter::FluxLimiter(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    Coupleable(this, false),
    _u_nodal(getVar("u", 0)),
    _u_var_num(coupled("u", 0)),
    _flux_limiter_type(getParam<MooseEnum>("flux_limiter_type").getEnum<FluxLimiterTypeEnum>())
{
}

Real
FluxLimiter::val_at_node(const Node & node) const
{
  // This is fine:
  // return _u_nodal->getNodalValue(node);
  // This is more convoluted, but it is used below in pPlus, etc
  dof_id_type dof = node.dof_number(_u_nodal->sys().number(), _u_var_num, 0);
  const NumericVector<Real> & u = *_u_nodal->sys().currentSolution();
  return u(dof);
}

Real
FluxLimiter::rPlus(unsigned node_i) const
{
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;
  const Real p = pPlus(node_i);
  const Real q = qPlus(node_i);
  if (p == 0.0)
    // Comment after Eqn (49): if P=0 then there's no antidiffusion, so no need to remove it
    return 1.0;
  const Real r = q / p;
  return fluxLimiter(1.0, r);
}

Real
FluxLimiter::rMinus(unsigned node_i) const
{
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;
  const Real p = pMinus(node_i);
  if (p == 0.0)
    // Comment after Eqn (49): if P=0 then there's no antidiffusion, so no need to remove it
    return 1.0;
  const Real q = qMinus(node_i);
  const Real r = q / p;
  return fluxLimiter(1.0, r);
}

Real
FluxLimiter::pPlus(unsigned node_i) const
{
  const NumericVector<Real> & u = *_u_nodal->sys().currentSolution();
  // Assume that the dof is equal to node_i (and similarly for node_j, below)
  const Real u_i = u(node_i);

  Real result = 0.0;
  // Must sum over all nodes connected with node_i.
  // In the current 1D case, this is just node_i-1 and node_i+1
  if (node_i > 0)
  {
    const unsigned node_j = node_i - 1;
    result += std::min(0.0, kij(node_i, node_j)) * std::min(0.0, u(node_j) - u_i);
  }
  if (node_i < u.size() - 1)
  {
    const unsigned node_j = node_i + 1;
    result += std::min(0.0, kij(node_i, node_j)) * std::min(0.0, u(node_j) - u_i);
  }
  return result;
}

Real
FluxLimiter::pMinus(unsigned node_i) const
{
  const NumericVector<Real> & u = *_u_nodal->sys().currentSolution();
  // Assume that the dof is equal to node_i (and similarly for node_j, below)
  const Real u_i = u(node_i);

  Real result = 0.0;
  // Must sum over all nodes connected with node_i.
  // In the current 1D case, this is just node_i-1 and node_i+1
  if (node_i > 0)
  {
    const unsigned node_j = node_i - 1;
    result += std::min(0.0, kij(node_i, node_j)) * std::max(0.0, u(node_j) - u_i);
  }
  if (node_i < u.size() - 1)
  {
    const unsigned node_j = node_i + 1;
    result += std::min(0.0, kij(node_i, node_j)) * std::max(0.0, u(node_j) - u_i);
  }
  return result;
}

Real
FluxLimiter::qPlus(unsigned node_i) const
{
  const NumericVector<Real> & u = *_u_nodal->sys().currentSolution();
  // Assume that the dof is equal to node_i (and similarly for node_j, below)
  const Real u_i = u(node_i);

  Real result = 0.0;
  // Must sum over all nodes connected with node_i.
  // In the current 1D case, this is just node_i-1 and node_i+1
  if (node_i > 0)
  {
    const unsigned node_j = node_i - 1;
    result += std::max(0.0, kij(node_i, node_j)) * std::max(0.0, u(node_j) - u_i);
  }
  if (node_i < u.size() - 1)
  {
    const unsigned node_j = node_i + 1;
    result += std::max(0.0, kij(node_i, node_j)) * std::max(0.0, u(node_j) - u_i);
  }
  return result;
}

Real
FluxLimiter::qMinus(unsigned node_i) const
{
  const NumericVector<Real> & u = *_u_nodal->sys().currentSolution();
  // Assume that the dof is equal to node_i (and similarly for node_j, below)
  const Real u_i = u(node_i);

  Real result = 0.0;
  // Must sum over all nodes connected with node_i.
  // In the current 1D case, this is just node_i-1 and node_i+1
  if (node_i > 0)
  {
    const unsigned node_j = node_i - 1;
    result += std::max(0.0, kij(node_i, node_j)) * std::min(0.0, u(node_j) - u_i);
  }
  if (node_i < u.size() - 1)
  {
    const unsigned node_j = node_i + 1;
    result += std::max(0.0, kij(node_i, node_j)) * std::min(0.0, u(node_j) - u_i);
  }
  return result;
}

Real
FluxLimiter::kij(unsigned node_i, unsigned node_j) const
{
  if (node_i == node_j + 1)
    return 0.5;
  else if (node_i == node_j - 1)
    return -0.5;
  return 0.0;
}

Real
FluxLimiter::fluxLimiter(Real a, Real b) const
{
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;

  if ((a >= 0.0 && b <= 0.0) || (a <= 0.0 && b >= 0.0))
    return 0.0;
  const Real s = (a > 0.0 ? 1.0 : -1.0);

  const Real lal = std::abs(a);
  const Real lbl = std::abs(b);
  switch (_flux_limiter_type)
  {
    case FluxLimiterTypeEnum::MinMod:
      return s * std::min(lal, lbl);
    case FluxLimiterTypeEnum::VanLeer:
      return s * 2 * lal * lbl / (lal + lbl);
    case FluxLimiterTypeEnum::MC:
      return s * std::min(0.5 * std::abs(a + b), 2.0 * std::min(lal, lbl));
    case FluxLimiterTypeEnum::superbee:
      return s * std::max(std::min(2.0 * lal, lbl), std::min(lal, 2.0 * lbl));
    default:
      mooseError("Unimplemented FluxLimiter type");
  }
  return 0.0;
}
