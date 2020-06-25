//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StatefulAuxLowerD.h"

registerMooseObject("MooseTestApp", StatefulAuxLowerD);

InputParameters
StatefulAuxLowerD::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("coupled_variable", "The heat flux variable");
  return params;
}

StatefulAuxLowerD::StatefulAuxLowerD(const InputParameters & parameters)
  : AuxKernel(parameters), _flux(coupledValueLower("coupled_variable"))
{
  if (!this->_bnd)
    mooseError("StatefulAuxLowerD can only be applied to boundaries (set boundary = something).");
}

Real
StatefulAuxLowerD::computeValue()
{
  Real coef;
  Real flux = _flux[_qp];

  if (std::abs(flux) > TOLERANCE * TOLERANCE)
    coef = 1 / std::sqrt(std::abs(flux));
  else
    coef = 0.0;

  return coef;
}
