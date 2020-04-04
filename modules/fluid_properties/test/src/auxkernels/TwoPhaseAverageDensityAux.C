//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoPhaseAverageDensityAux.h"
#include "TwoPhaseFluidProperties.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesTestApp", TwoPhaseAverageDensityAux);

InputParameters
TwoPhaseAverageDensityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Computes the average of the densities of the phases corresponding to "
                             "a 2-phase fluid properties object.");

  params.addRequiredCoupledVar("p", "Pressure");
  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredParam<UserObjectName>("fp_2phase", "2-phase fluid properties user object name");

  return params;
}

TwoPhaseAverageDensityAux::TwoPhaseAverageDensityAux(const InputParameters & parameters)
  : AuxKernel(parameters),

    _p(coupledValue("p")),
    _T(coupledValue("T")),
    _fp_2phase(getUserObject<TwoPhaseFluidProperties>("fp_2phase")),
    _fp_liquid(getUserObjectByName<SinglePhaseFluidProperties>(_fp_2phase.getLiquidName())),
    _fp_vapor(getUserObjectByName<SinglePhaseFluidProperties>(_fp_2phase.getVaporName()))
{
}

Real
TwoPhaseAverageDensityAux::computeValue()
{
  return 0.5 *
         (_fp_liquid.rho_from_p_T(_p[_qp], _T[_qp]) + _fp_vapor.rho_from_p_T(_p[_qp], _T[_qp]));
}
