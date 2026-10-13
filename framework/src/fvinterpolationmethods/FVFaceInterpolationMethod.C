//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFaceInterpolationMethod.h"

Real
FVFaceInterpolationMethod::interpolate(const Moose::FunctorBase<Real> & functor,
                                       const FaceInfo & face,
                                       const Moose::StateArg & state) const
{
  mooseAssert(face.neighborPtr(),
              "Face interpolation with a Moose functor requires a two-sided internal face.");

  const Real elem_value = functor(Moose::ElemArg{face.elemPtr(), false}, state);
  const Real neighbor_value = functor(Moose::ElemArg{face.neighborPtr(), false}, state);
  return interpolate(face, elem_value, neighbor_value);
}
