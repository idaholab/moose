//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVPorosityVariable.h"
#include "NS.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", PINSFVPorosityVariable);

InputParameters
PINSFVPorosityVariable::validParams()
{
  return INSFVVariable::validParams();
}

PINSFVPorosityVariable::PINSFVPorosityVariable(const InputParameters & params)
  : INSFVVariable(params)
{
}

bool
PINSFVPorosityVariable::isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                                   const Elem * const elem) const
{
  if (isDirichletBoundaryFace(fi, elem))
    return false;
  if (_ssf_faces.count(std::make_pair(&fi, elem)))
    return true;
  if (!isInternalFace(fi))
    return true;

  return !MooseUtils::relativeFuzzyEqual((*this)(Moose::ElemArg{&fi.elem(), false}),
                                         (*this)(Moose::ElemArg{fi.neighborPtr(), false}));
}
