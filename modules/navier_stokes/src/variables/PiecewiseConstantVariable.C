//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseConstantVariable.h"
#include "NS.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", PiecewiseConstantVariable);

InputParameters
PiecewiseConstantVariable::validParams()
{
  return INSFVVariable::validParams();
}

PiecewiseConstantVariable::PiecewiseConstantVariable(const InputParameters & params)
  : INSFVVariable(params)
{
}

bool
PiecewiseConstantVariable::isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                                      const Elem * const elem) const
{
  if (isDirichletBoundaryFace(fi, elem))
    return false;
  if (!isInternalFace(fi))
    return true;

  return !MooseUtils::relativeFuzzyEqual((*this)(Moose::ElemArg{&fi.elem(), false}),
                                         (*this)(Moose::ElemArg{fi.neighborPtr(), false}));
}
