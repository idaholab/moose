/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "ViscosityIC.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("SubChannelApp", ViscosityIC);

InputParameters
ViscosityIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription(
      "Computes viscosity of coolant based on initial conditions of pressure and temperature");
  params.addRequiredCoupledVar("T", "Temperature [K]");
  params.addRequiredParam<Real>("p", "Pressure [Pa]");
  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object name");
  params.addClassDescription("Computes viscosity from specified pressure and temperature");
  return params;
}

ViscosityIC::ViscosityIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _T(coupledValue("T")),
    _P(getParam<Real>("p")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
ViscosityIC::value(const Point & /*p*/)
{
  return _fp.mu_from_p_T(_P, _T[_qp]);
}
