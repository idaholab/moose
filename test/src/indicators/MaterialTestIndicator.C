//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialTestIndicator.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

registerMooseObject("MooseTestApp", MaterialTestIndicator);

InputParameters
MaterialTestIndicator::validParams()
{
  InputParameters params = Indicator::validParams();
  params += MaterialPropertyInterface::validParams();
  params.addParam<MaterialPropertyName>(
      "property", "The name of the material property to use for an indicator.");
  return params;
}

MaterialTestIndicator::MaterialTestIndicator(const InputParameters & parameters)
  : Indicator(parameters),
    _property(getMaterialProperty<Real>("property")),
    _qrule(_assembly.qRule()),
    _indicator_var(dynamic_cast<MooseVariable &>(_sys.getVariable(_tid, name())))
{
}

void
MaterialTestIndicator::computeIndicator()
{
  Real min = std::numeric_limits<Real>::max();
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    min = std::min(min, _property[qp]);
  _indicator_var.setNodalValue(min);
}
