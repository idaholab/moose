//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <cmath> // std::sinh and std::cosh
#include "Q2PMaterial.h"

registerMooseObject("RichardsApp", Q2PMaterial);

InputParameters
Q2PMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredRangeCheckedParam<Real>(
      "mat_porosity",
      "mat_porosity>=0 & mat_porosity<=1",
      "The porosity of the material.  Should be between 0 and 1.  Eg, 0.1");
  params.addCoupledVar("por_change",
                       0,
                       "An auxillary variable describing porosity changes.  "
                       "Porosity = mat_porosity + por_change.  If this is not "
                       "provided, zero is used.");
  params.addRequiredParam<RealTensorValue>("mat_permeability", "The permeability tensor (m^2).");
  params.addCoupledVar("perm_change",
                       "A list of auxillary variable describing permeability "
                       "changes.  There must be 9 of these (in 3D), corresponding "
                       "to the xx, xy, xz, yx, yy, yz, zx, zy, zz components "
                       "respectively (in 3D).  Permeability = "
                       "mat_permeability*10^(perm_change).");
  params.addRequiredParam<RealVectorValue>(
      "gravity",
      "Gravitational acceleration (m/s^2) as a vector pointing downwards.  Eg (0,0,-10)");
  return params;
}

Q2PMaterial::Q2PMaterial(const InputParameters & parameters)
  : Material(parameters),
    _material_por(getParam<Real>("mat_porosity")),
    _por_change(coupledValue("por_change")),
    _por_change_old(isCoupled("por_change") ? coupledValueOld("por_change") : _zero),
    _material_perm(getParam<RealTensorValue>("mat_permeability")),
    _material_gravity(getParam<RealVectorValue>("gravity")),
    _porosity_old(declareProperty<Real>("porosity_old")),
    _porosity(declareProperty<Real>("porosity")),
    _permeability(declareProperty<RealTensorValue>("permeability")),
    _gravity(declareProperty<RealVectorValue>("gravity")),
    _perm_change(isCoupled("perm_change")
                     ? coupledValues("perm_change")
                     : std::vector<const VariableValue *>(LIBMESH_DIM * LIBMESH_DIM, &_zero))
{
  if (isCoupled("perm_change") && (coupledComponents("perm_change") != LIBMESH_DIM * LIBMESH_DIM))
    mooseError(LIBMESH_DIM * LIBMESH_DIM,
               " components of perm_change must be given to a Q2PMaterial.  You supplied ",
               coupledComponents("perm_change"),
               "\n");
}

void
Q2PMaterial::computeQpProperties()
{
  _porosity[_qp] = _material_por + _por_change[_qp];
  _porosity_old[_qp] = _material_por + _por_change_old[_qp];

  _permeability[_qp] = _material_perm;
  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      _permeability[_qp](i, j) *= std::pow(10, (*_perm_change[LIBMESH_DIM * i + j])[_qp]);

  _gravity[_qp] = _material_gravity;
}
