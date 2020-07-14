//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

namespace libMesh
{
namespace Parallel
{

template <>
class Packing<std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>>
{

public:
  typedef Real buffer_type;

  static unsigned int packed_size(typename std::vector<Real>::const_iterator in);

  static unsigned int packable_size(
      const std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>> & object,
      const void *);

  template <typename Iter, typename Context>
  static void
  pack(const std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>> & object,
       Iter data_out,
       const Context *);

  template <typename BufferIter, typename Context>
  static std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>
  unpack(BufferIter in, Context *);
};
} // namespace Parallel
} // namespace libMesh
