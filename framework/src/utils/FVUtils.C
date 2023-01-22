//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableFV.h"
#include "FVUtils.h"

namespace Moose
{
namespace FV
{

bool
elemHasFaceInfo(const Elem & elem, const Elem * const neighbor)
{
  // The face info belongs to elem:
  //  * at all mesh boundaries (i.e. where there is no neighbor)
  //  * if the element faces a neighbor which is on a lower refinement level
  //  * if the element is active and it has a lower ID than its neighbor
  if (!neighbor)
    return true;
  else if (elem.level() != neighbor->level())
    return neighbor->level() < elem.level();
  else if (!neighbor->active())
    return false;
  else
    return elem.id() < neighbor->id();
}

template <typename OutputType>
std::tuple<const Elem *, const Elem *, bool>
determineElemOneAndTwo(const FaceInfo & fi, const MooseVariableFV<OutputType> & var)
{
  auto ft = fi.faceType(var.name());
  mooseAssert(ft == FaceInfo::VarFaceNeighbors::BOTH
                  ? var.hasBlocks(fi.elem().subdomain_id()) && fi.neighborPtr() &&
                        var.hasBlocks(fi.neighborPtr()->subdomain_id())
                  : true,
              "Finite volume variable " << var.name()
                                        << " does not exist on both sides of the face despite "
                                           "what the FaceInfo is telling us.");
  mooseAssert(ft == FaceInfo::VarFaceNeighbors::ELEM
                  ? var.hasBlocks(fi.elem().subdomain_id()) &&
                        (!fi.neighborPtr() || !var.hasBlocks(fi.neighborPtr()->subdomain_id()))
                  : true,
              "Finite volume variable " << var.name()
                                        << " does not exist on or only on the elem side of the "
                                           "face despite what the FaceInfo is telling us.");
  mooseAssert(ft == FaceInfo::VarFaceNeighbors::NEIGHBOR
                  ? fi.neighborPtr() && var.hasBlocks(fi.neighborPtr()->subdomain_id()) &&
                        !var.hasBlocks(fi.elem().subdomain_id())
                  : true,
              "Finite volume variable " << var.name()
                                        << " does not exist on or only on the neighbor side of the "
                                           "face despite what the FaceInfo is telling us.");

  const bool one_is_elem =
      ft == FaceInfo::VarFaceNeighbors::BOTH || ft == FaceInfo::VarFaceNeighbors::ELEM;
  const Elem * const elem_one = one_is_elem ? &fi.elem() : fi.neighborPtr();
  mooseAssert(elem_one, "This elem should be non-null!");
  const Elem * const elem_two = one_is_elem ? fi.neighborPtr() : &fi.elem();

  return std::make_tuple(elem_one, elem_two, one_is_elem);
}

template std::tuple<const Elem *, const Elem *, bool>
determineElemOneAndTwo(const FaceInfo & fi, const MooseVariableFV<Real> & var);
}
}
