//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <tuple>

namespace libMesh
{
class Elem;
class QBase;
}

namespace Moose
{
struct ElemArg;
struct ElemFromFaceArg;
struct FaceArg;
struct SingleSidedFaceArg;
using ElemQpArg = std::tuple<const libMesh::Elem *, unsigned int, const libMesh::QBase *>;
using ElemSideQpArg =
    std::tuple<const libMesh::Elem *, unsigned int, unsigned int, const libMesh::QBase *>;
template <typename T>
class FunctorBase;
class FunctorEnvelopeBase;
template <typename T>
class FunctorEnvelope;
template <typename T>
using Functor = FunctorEnvelope<T>;
template <typename T>
class ConstantFunctor;
template <typename T>
class NullFunctor;
}
