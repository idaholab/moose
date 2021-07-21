//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <utility>

class FaceInfo;
namespace libMesh
{
class Elem;
}
namespace Moose
{
namespace FV
{
class Limiter;
}
}

template <typename T>
class FunctorInterface
{
public:
  T operator()(const libMesh::Elem * const & elem) const = 0;
  T operator()(const std::pair<const FaceInfo *, const Moose::FV::Limiter *> face) const = 0;
};
