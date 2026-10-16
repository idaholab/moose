//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DittusBoelterHTCInterface.h"

#include "FEProblemBase.h"
#include "Function.h"
#include "MooseObject.h"
#include "Numerics.h"
#include "SinglePhaseFluidProperties.h"

InputParameters
DittusBoelterHTCInterface::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");
  params.addRequiredParam<FunctionName>("reference_pressure", "System reference pressure [Pa]");
  params.addRequiredParam<FunctionName>("flow_area",
                                        "Cross-sectional area of the fluid flow channel [m^2]");

  return params;
}

DittusBoelterHTCInterface::DittusBoelterHTCInterface(const MooseObject * moose_object)
  : _fp_dbi(getFluidPropertiesOrError(moose_object)),
    _P_ref_dbi(getFunctionByParam(moose_object, "reference_pressure")),
    _flow_area_dbi(getFunctionByParam(moose_object, "flow_area"))
{
}

THREAD_ID
DittusBoelterHTCInterface::threadID(const MooseObject * const moose_object)
{
  const auto & params = moose_object->parameters();
  return params.have_parameter<THREAD_ID>("_tid") ? params.get<THREAD_ID>("_tid") : 0;
}

const SinglePhaseFluidProperties &
DittusBoelterHTCInterface::getFluidPropertiesOrError(const MooseObject * const moose_object)
{
  auto & fe_problem =
      *moose_object->parameters().getCheckedPointerParam<FEProblemBase *>("_fe_problem_base");
  const auto & uo_name = moose_object->getParam<UserObjectName>("fluid_properties");

  if (!fe_problem.hasUserObject(uo_name))
    moose_object->paramError(
        "fluid_properties", "No user object with the name '", uo_name, "' exists.");

  const auto * const fp = dynamic_cast<const SinglePhaseFluidProperties *>(
      &fe_problem.getUserObjectBase(uo_name, threadID(moose_object)));
  if (!fp)
    moose_object->paramError("fluid_properties",
                             "The user object '",
                             uo_name,
                             "' is not a SinglePhaseFluidProperties object.");

  return *fp;
}

const Function &
DittusBoelterHTCInterface::getFunctionByParam(const MooseObject * const moose_object,
                                              const std::string & param_name)
{
  auto & fe_problem =
      *moose_object->parameters().getCheckedPointerParam<FEProblemBase *>("_fe_problem_base");

  // Not guarded by hasFunction(), which is false for a parameter given as a plain number or
  // an expression; getFunction() builds the corresponding function on demand
  return fe_problem.getFunction(moose_object->getParam<FunctionName>(param_name),
                                threadID(moose_object));
}

ADReal
DittusBoelterHTCInterface::wallHeatTransferCoefficient(const Real t) const
{
  using std::abs, std::pow;

  const auto & m = massFlowRate();

  // Upwind the segment inlet temperature on the flow direction
  const auto T_in = 1.0 / 2.0 * (1 - abs(m) / m) * outletFluidTemperature() +
                    1.0 / 2.0 * (1 + abs(m) / m) * inletFluidTemperature();

  // Properties at the bulk film temperature of the segment
  const auto T_film = (fluidTemperature() + T_in) / 2;
  const auto P_ref = _P_ref_dbi.value(t, Point());
  const auto mu = _fp_dbi.mu_from_p_T(P_ref, T_film);
  const auto cp = _fp_dbi.cp_from_p_T(P_ref, T_film);
  const auto k = _fp_dbi.k_from_p_T(P_ref, T_film);

  const auto A = _flow_area_dbi.value(t, Point());
  const auto D_h = 4.0 * A / wettedPerimeter(t);
  // Formed from the mass flux G = rho * vel, which the segment's mass flow rate gives
  // directly, so no density evaluation is needed
  const auto G = abs(m) / A;
  const auto Re = G * D_h / mu;
  const auto Pr = THM::Prandtl(cp, mu, k);
  const auto Nu = 0.023 * pow(Re, 0.8) * pow(Pr, 0.4);

  return THM::wallHeatTransferCoefficient(Nu, k, D_h);
}
