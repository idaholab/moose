/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include <cmath> // std::sinh and std::cosh
#include "Q2PMaterial.h"

template <>
InputParameters
validParams<Q2PMaterial>()
{
  InputParameters params = validParams<Material>();

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
    _gravity(declareProperty<RealVectorValue>("gravity"))
{
  if (isCoupled("perm_change") && (coupledComponents("perm_change") != LIBMESH_DIM * LIBMESH_DIM))
    mooseError(LIBMESH_DIM * LIBMESH_DIM,
               " components of perm_change must be given to a Q2PMaterial.  You supplied ",
               coupledComponents("perm_change"),
               "\n");

  _perm_change.resize(LIBMESH_DIM * LIBMESH_DIM);
  for (unsigned int i = 0; i < LIBMESH_DIM * LIBMESH_DIM; ++i)
    _perm_change[i] = (isCoupled("perm_change") ? &coupledValue("perm_change", i)
                                                : &_zero); // coupledValue returns a reference (an
                                                           // alias) to a VariableValue, and the &
                                                           // turns it into a pointer
}

void
Q2PMaterial::computeQpProperties()
{
  _porosity[_qp] = _material_por + _por_change[_qp];
  _porosity_old[_qp] = _material_por + _por_change_old[_qp];

  _permeability[_qp] = _material_perm;
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    for (unsigned int j = 0; j < LIBMESH_DIM; j++)
      _permeability[_qp](i, j) *= std::pow(10, (*_perm_change[LIBMESH_DIM * i + j])[_qp]);

  _gravity[_qp] = _material_gravity;
}
