/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                 Rattlesnake                   */
/*                                               */
/*    (c) 2017 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     Under Contract No. DE-AC07-05ID14517      */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#include "MaterialReaction.h"

#include "Material.h"

registerMooseObject("MooseApp", MaterialReaction);

template <>
InputParameters
validParams<MaterialReaction>()
{
  InputParameters params = validParams<Reaction>();
  params.addParam<MaterialPropertyName>(
      "coefficient", 1.0, "Name of the material property acting as reaction coefficient.");
  return params;
}

MaterialReaction::MaterialReaction(const InputParameters & parameters)
  : Reaction(parameters), _coeff(getMaterialProperty<Real>("coefficient"))
{
}

Real
MaterialReaction::computeQpResidual()
{
  return _coeff[_qp] * Reaction::computeQpResidual();
}

Real
MaterialReaction::computeQpJacobian()
{
  return _coeff[_qp] * Reaction::computeQpJacobian();
}
