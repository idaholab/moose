//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledVarSubdomainModifier.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", CoupledVarSubdomainModifier);

InputParameters
CoupledVarSubdomainModifier::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();
  params.addClassDescription(
      "Sets the element subdomain ID equal to that defined by the coupled_var");
  params.addRequiredCoupledVar("coupled_var",
                               "Coupled variable whose value sets the subdomain ID.  The value of "
                               "coupled_var rounded to the nearest unsigned integer is used");
  return params;
}

CoupledVarSubdomainModifier::CoupledVarSubdomainModifier(const InputParameters & parameters)
  : ElementSubdomainModifier(parameters), _v(coupledValue("coupled_var"))
{
}

SubdomainID
CoupledVarSubdomainModifier::computeSubdomainID() const
{
  Real avg_val = 0;

  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    avg_val += _v[qp] * _JxW[qp] * _coord[qp];
  avg_val /= _current_elem_volume;

  if (avg_val <= 0)
    return 0;
  else if (avg_val >= std::numeric_limits<SubdomainID>::max())
    return std::numeric_limits<SubdomainID>::max() - 1;
  return static_cast<SubdomainID>(std::lround(avg_val));
}
