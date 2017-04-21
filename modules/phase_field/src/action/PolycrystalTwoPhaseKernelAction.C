/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PolycrystalTwoPhaseKernelAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template <>
InputParameters
validParams<PolycrystalTwoPhaseKernelAction>()
{
  InputParameters params = validParams<PolycrystalKernelAction>();
  params.addClassDescription("Set up Two-Phase polycrystal kernels");
  params.addRequiredParam<unsigned int>(
      "second_phase_op_num", "specifies the total number of grains/variants of the second phase");
  params.addParam<Real>(
      "en_ratio", 1.0, "Ratio of surface energy to GB energy, e.g., interphase energy");
  params.addParam<Real>(
      "second_phase_en_ratio", 1.0, "Ratio of second-phase to parent-phase GB energy");
  params.addParam<Real>("mob_ratio", 1.0, "Ratio of surface energy to GB mobility");
  return params;
}

PolycrystalTwoPhaseKernelAction::PolycrystalTwoPhaseKernelAction(const InputParameters & params)
  : PolycrystalKernelAction(params),
    _phase_num(2),
    _second_phase_op_num(getParam<unsigned int>("second_phase_op_num")),
    _en_ratio(getParam<Real>("en_ratio")),
    _second_phase_en_ratio(getParam<Real>("second_phase_en_ratio")),
    _mob_ratio(getParam<Real>("mob_ratio"))
{
}
