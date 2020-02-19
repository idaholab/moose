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

#pragma once

#include "Reaction.h"

class MaterialReaction;

template <>
InputParameters validParams<MaterialReaction>();

class MaterialReaction : public Reaction
{
public:
  MaterialReaction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const MaterialProperty<Real> & _coeff;
};
