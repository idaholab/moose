//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledVarThresholdElementSubdomainModifier.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", CoupledVarThresholdElementSubdomainModifier);

InputParameters
CoupledVarThresholdElementSubdomainModifier::validParams()
{
  InputParameters params = ThresholdElementSubdomainModifier::validParams();
  params.addClassDescription("Modify the element subdomain ID if a coupled variable satisfies the "
                             "criterion for the threshold (above, equal, or below)");
  params.addRequiredCoupledVar("coupled_var",
                               "Coupled variable whose value is used in the criterion");
  return params;
}

CoupledVarThresholdElementSubdomainModifier::CoupledVarThresholdElementSubdomainModifier(
    const InputParameters & parameters)
  : ThresholdElementSubdomainModifier(parameters), _v(coupledValue("coupled_var"))
{
}

Real
CoupledVarThresholdElementSubdomainModifier::computeValue()
{
  Real avg_val = 0;

  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    avg_val += _v[qp] * _JxW[qp] * _coord[qp];
  avg_val /= _current_elem_volume;

  return avg_val;
}
