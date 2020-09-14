//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadratureMaterial.h"

registerMooseObject("MooseTestApp", QuadratureMaterial);

InputParameters
QuadratureMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<std::string>("property_name",
                                       "The desired name for the Material Property.");
  return params;
}

QuadratureMaterial::QuadratureMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop_name(getParam<std::string>("property_name")),
    _mat_prop(declareProperty<Real>(_prop_name))
{
  if (!_bnd)
  {
    std::set<SubdomainID> blocks;
    if (blockRestricted())
      blocks = blockIDs();
    else
      blocks = _mesh.meshSubdomains();

    for (auto & blk : blockIDs())
      _fe_problem.bumpAllQRuleOrder(EIGHTH, blk);
  }
}

void
QuadratureMaterial::computeQpProperties()
{
  if (_qrule->get_order() < 8)
    mooseError("This material expect quadrature order to be higher than 8");
  _mat_prop[_qp] = _qp;
}
