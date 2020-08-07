//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/packing.h"
#include "libmesh/dense_vector.h"
#include "MooseTypes.h"

namespace libMesh
{
namespace Parallel
{

/**
 * This object is responsible for packing and unpacking data stored in a
 * std::shared_ptr<DenseVector<Real>>. It is also specific to the training process of
 * PODReducedBasisTrainer since it packs the global sample index and variable index together with
 * the vector.
 */
template <>
class Packing<std::tuple<unsigned int, unsigned int, std::shared_ptr<DenseVector<Real>>>>
{

public:
  typedef Real buffer_type;

  /// Getting the sizes of the packed objects using an iterator.
  static unsigned int packed_size(typename std::vector<Real>::const_iterator in);

  /// Getting the sizes of the packed objects using the object itself.
  static unsigned int packable_size(
      const std::tuple<unsigned int, unsigned int, std::shared_ptr<DenseVector<Real>>> & object,
      const void *);

  // Pack the objects on the sending process.
  template <typename Iter, typename Context>
  static void
  pack(const std::tuple<unsigned int, unsigned int, std::shared_ptr<DenseVector<Real>>> & object,
       Iter data_out,
       const Context *);

  // Unpack the object on the receiving process.
  template <typename BufferIter, typename Context>
  static std::tuple<unsigned int, unsigned int, std::shared_ptr<DenseVector<Real>>>
  unpack(BufferIter in, Context *);
};
} // namespace Parallel
} // namespace libMesh
