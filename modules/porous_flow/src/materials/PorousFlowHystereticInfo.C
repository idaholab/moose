//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHystereticInfo.h"

registerMooseObject("PorousFlowApp", PorousFlowHystereticInfo);

InputParameters
PorousFlowHystereticInfo::validParams()
{
  InputParameters params = PorousFlowHystereticCapillaryPressure::validParams();
  params.addCoupledVar("pc_var",
                       0,
                       "Variable that represents capillary pressure.  Depending on info_required, "
                       "this may not be used to compute the info");
  params.addRequiredCoupledVar(
      "sat_var",
      "Variable that represent liquid saturation.  This is always needed to "
      "ensure the hysteretic order is computed correctly");
  MooseEnum property_enum("pc sat sat_given_pc dS_dPc_err dPc_dS_err d2S_dPc2_err d2Pc_dS2_err",
                          "pc");
  params.addParam<MooseEnum>(
      "info_required",
      property_enum,
      "The type of information required.  pc: capillary pressure given the saturation (pc_var is "
      "not used in this case).  sat: given the liquid saturation, compute the capillary pressure, "
      "then invert the relationship to yield liquid saturation again (pc_var is not used in this "
      "case).  This is useful to understand the non-invertibility of the hysteretic relationships. "
      " sat_given_pc: given the capillary pressure, compute the saturation.  dS_dPc_err: relative "
      "error in d(sat)/d(pc) calculation, ie (S(Pc + fd_eps) - S(Pc - fd_eps))/(2 * eps * S'(Pc)) "
      "- 1, where S' is the coded derivative (Pc is computed from sat in this case: pc_var is not "
      "used).  This is useful for checking derivatives.  dPc_dS_err: relative error in "
      "d(pc)/d(sat) calculation, ie (Pc(S + fd_eps) - Pc(S - fd_eps)) / (2 * eps * Pc'(S)) - 1, "
      "where Pc' is the coded derative (pc_var is not used in this case).  d2S_dPc2_err: relative "
      "error in d^2(sat)/d(pc)^2 (Pc is computed from sat in this case: pc_var is not used).  "
      "d2Pc_dS2_err: relative error in d^2(pc)/d(sat)^2.");
  params.addParam<Real>(
      "fd_eps",
      1E-8,
      "Small quantity used in computing the finite-difference approximations to derivatives");
  params.addClassDescription(
      "This Material computes capillary pressure or saturation, etc.  It is primarily "
      "of use when users desire to compute hysteretic quantities such as capillary pressure for "
      "visualisation purposes.  The "
      "result is written into PorousFlow_hysteretic_info_nodal or "
      "PorousFlow_hysteretic_info_qp (depending on the at_nodes flag).  It "
      "does not "
      "compute porepressure and should not be used in simulations that employ "
      "PorousFlow*PhaseHys* "
      "Materials.");
  return params;
}

PorousFlowHystereticInfo::PorousFlowHystereticInfo(const InputParameters & parameters)
  : PorousFlowHystereticCapillaryPressure(parameters),
    _pc(_nodal_material ? declareProperty<Real>("PorousFlow_hysteretic_capillary_pressure_nodal")
                        : declareProperty<Real>("PorousFlow_hysteretic_capillary_pressure_qp")),
    _info(_nodal_material ? declareProperty<Real>("PorousFlow_hysteretic_info_nodal")
                          : declareProperty<Real>("PorousFlow_hysteretic_info_qp")),
    _pc_val(_nodal_material ? coupledDofValues("pc_var") : coupledValue("pc_var")),
    _sat_val(_nodal_material ? coupledDofValues("sat_var") : coupledValue("sat_var")),
    _fd_eps(getParam<Real>("fd_eps")),
    _info_enum(getParam<MooseEnum>("info_required").getEnum<InfoTypeEnum>())
{
  // note that _num_phases must be positive, otherwise get a problem with using
  // PorousFlowHysteresisOrder, so _num_phases > 0 needn't be checked here
}

void
PorousFlowHystereticInfo::initQpStatefulProperties()
{
  PorousFlowHystereticCapillaryPressure::initQpStatefulProperties();
  _saturation[_qp][0] = _sat_val[_qp];
  _pc[_qp] = capillaryPressureQp(_sat_val[_qp]);
  computeQpInfo();
}

void
PorousFlowHystereticInfo::computeQpProperties()
{
  PorousFlowHystereticCapillaryPressure::computeQpProperties();
  _saturation[_qp][0] = _sat_val[_qp];
  _pc[_qp] = capillaryPressureQp(_sat_val[_qp]);
  computeQpInfo();
}

void
PorousFlowHystereticInfo::computeQpInfo()
{
  switch (_info_enum)
  {
    case InfoTypeEnum::PC:
      _info[_qp] = capillaryPressureQp(_sat_val[_qp]);
      break;
    case InfoTypeEnum::SAT:
    {
      const Real pc = capillaryPressureQp(_sat_val[_qp]);
      _info[_qp] = liquidSaturationQp(pc);
      break;
    }
    case InfoTypeEnum::SAT_GIVEN_PC:
      _info[_qp] = liquidSaturationQp(_pc_val[_qp]);
      break;
    case InfoTypeEnum::DS_DPC_ERR:
    {
      const Real pc = capillaryPressureQp(_sat_val[_qp]);
      const Real fd =
          0.5 * (liquidSaturationQp(pc + _fd_eps) - liquidSaturationQp(pc - _fd_eps)) / _fd_eps;
      _info[_qp] = relativeError(fd, dliquidSaturationQp(pc));
      break;
    }
    case InfoTypeEnum::DPC_DS_ERR:
    {
      const Real fd = 0.5 *
                      (capillaryPressureQp(_sat_val[_qp] + _fd_eps) -
                       capillaryPressureQp(_sat_val[_qp] - _fd_eps)) /
                      _fd_eps;
      _info[_qp] = relativeError(fd, dcapillaryPressureQp(_sat_val[_qp]));
      break;
    }
    case InfoTypeEnum::D2S_DPC2_ERR:
    {
      const Real pc = capillaryPressureQp(_sat_val[_qp]);
      const Real fd =
          0.5 * (dliquidSaturationQp(pc + _fd_eps) - dliquidSaturationQp(pc - _fd_eps)) / _fd_eps;
      _info[_qp] = relativeError(fd, d2liquidSaturationQp(pc));
      break;
    }
    case InfoTypeEnum::D2PC_DS2_ERR:
    {
      const Real fd = 0.5 *
                      (dcapillaryPressureQp(_sat_val[_qp] + _fd_eps) -
                       dcapillaryPressureQp(_sat_val[_qp] - _fd_eps)) /
                      _fd_eps;
      _info[_qp] = relativeError(fd, d2capillaryPressureQp(_sat_val[_qp]));
      break;
    }
  }
}

Real
PorousFlowHystereticInfo::relativeError(Real finite_difference, Real hand_coded)
{
  if (finite_difference == 0.0 && hand_coded == 0.0)
    return 0.0;
  else if (finite_difference > 0.0 && hand_coded == 0.0)
    return std::numeric_limits<Real>::max();
  else if (finite_difference < 0.0 && hand_coded == 0.0)
    return std::numeric_limits<Real>::min();
  return finite_difference / hand_coded - 1.0;
}
