//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "FaceInfo.h"
#include "CellCenteredMapFunctor.h"
#include "MathFVUtils.h"
#include "libmesh/elem.h"

#include <unordered_map>
#include <utility>

namespace Moose
{
namespace FV
{
/**
 * Takes an input functor that can be evaluated at faces, typically by linearly interpolating
 * between adjacent cell center values, and then creates an output functor whose cell-center
 * evaluations will correspond to weighted averages of the input functor's surrounding face
 * evaluations
 * @param output_functor the output functor
 * @param input_functor the input functor
 * @param num_int_recs the total number of interpolation and reconstruction operations to perform.
 * If this number is greater than 1, then this function will recurse
 * @param weight_with_sf when reconstructing the cell center value, decides whether the face values
 * (and maybe gradients) are weighted with the surface vector. If this is false, then the weights
 * are simply unity
 * @param faces the mesh faces we will be looping over for the interpolations and reconstructions
 */
template <typename T, typename Map>
void
interpolateReconstruct(CellCenteredMapFunctor<T, Map> & output_functor,
                       const Moose::FunctorBase<T> & input_functor,
                       const unsigned int num_int_recs,
                       const bool weight_with_sf,
                       const std::vector<const FaceInfo *> & faces,
                       const Moose::StateArg & time)
{
  if (!num_int_recs)
    return;

  std::unordered_map<dof_id_type, std::pair<T, Real>> elem_to_num_denom;

  for (const auto * const face : faces)
  {
    mooseAssert(face, "This must be non-null");
    const Real weight = weight_with_sf ? face->faceArea() * face->faceCoord() : 1;
    const Moose::FaceArg face_arg{
        face,
        Moose::FV::LimiterType::CentralDifference,
        true,
        false,
        input_functor.hasFaceSide(*face, true)
            ? (input_functor.hasFaceSide(*face, false) ? nullptr : face->elemPtr())
            : face->neighborPtr(),
        nullptr};
    auto face_value = input_functor(face_arg, time);
    std::pair<T, Real> * neighbor_pair = nullptr;
    if (face->neighborPtr() && face->neighborPtr() != libMesh::remote_elem)
    {
      neighbor_pair = &elem_to_num_denom[face->neighbor().id()];
      neighbor_pair->first += face_value * weight;
      neighbor_pair->second += weight;
    }
    auto & elem_pair = elem_to_num_denom[face->elem().id()];
    elem_pair.first += std::move(face_value) * weight;
    elem_pair.second += weight;
  }

  for (const auto & pair_ : elem_to_num_denom)
  {
    const auto & data_pair = pair_.second;
    output_functor[pair_.first] = data_pair.first / data_pair.second;
  }

  interpolateReconstruct(
      output_functor, output_functor, num_int_recs - 1, weight_with_sf, faces, time);
}
}
}
