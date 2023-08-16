//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FaceCenteredMapFunctor.h"
#include "FaceInfo.h"
#include "FVUtils.h"
#include "libmesh/compare_types.h"
#include "libmesh/type_tensor.h"
#include "libmesh/tensor_tools.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/elem.h"
#include "libmesh/point.h"

namespace Moose
{
template <typename T, typename T2, typename std::enable_if<ScalarTraits<T>::value, int>::type = 0>
inline TypeVector<typename CompareTypes<T, T2>::supertype>
outer_product(const T & a, const TypeVector<T2> & b)
{
  TypeVector<typename CompareTypes<T, T2>::supertype> ret;
  for (const auto i : make_range(Moose::dim))
    ret(i) = a * b(i);

  return ret;
}

template <typename T, typename T2>
inline TypeTensor<typename CompareTypes<T, T2>::supertype>
outer_product(const TypeVector<T> & a, const TypeVector<T2> & b)
{
  return libMesh::outer_product(a, b);
}
}

template <typename T, typename Map>
typename FaceCenteredMapFunctor<T, Map>::ValueType
FaceCenteredMapFunctor<T, Map>::evaluate(const ElemArg & elem_arg, const StateArg &) const
{
  // The following reconstruction is based on Weller's method. For more information on this,
  // we recommend:
  //
  // Weller, Hilary. "Non-orthogonal version of the arbitrary polygonal C-grid and a new diamond
  // grid." Geoscientific Model Development 7.3 (2014): 779-797.
  //
  // and
  //
  // Aguerre, Horacio J., et al. "An oscillation-free flow solver based on flux reconstruction."
  // Journal of Computational Physics 365 (2018): 135-148.
  //
  // This basically reconstructs the cell value based on flux values as follows:
  //
  // $\left( \sum_f n_f \outer S_f \right)^{-1} \sum_f (\phi_f \cdot S_f)n_f$
  //
  // where $S_f$ is the surface normal vector, $n_f$ is the unit surface vector and $\phi_f$ is a
  // vector value on the field. Hence the restriction to vector values.

  using ValueType = typename FaceCenteredMapFunctor<T, Map>::ValueType;

  // Primitive type one rank below the type of the stored data (T). If we are storing a rank two
  // tensor, this is a vector, if we are storing a vector this is just a number
  using PrimitiveType = typename MetaPhysicL::ReplaceAlgebraicType<
      T,
      typename TensorTools::DecrementRank<typename MetaPhysicL::ValueType<T>::type>::type>::type;

  if constexpr (libMesh::TensorTools::TensorTraits<ValueType>::rank == 1)
  {
    const auto dim = _mesh.dimension();
    // The reason why these are DenseVector/Matrix is that when the mesh dimension is lower than 3,
    // we get singular matrixes if we try to invert TensorValues.
    DenseMatrix<PrimitiveType> n_x_Sf(dim, dim);
    DenseVector<PrimitiveType> sum_normal_flux(dim);

    const Elem * const elem = elem_arg.elem;

    for (const auto side : make_range(elem->n_sides()))
    {
      const Elem * const neighbor = elem->neighbor_ptr(side);

      // We need to check if the faceinfo belongs to the element or the neighbor. Based on that we
      // query the faceinfo and adjust the normal to point outward of the current cell
      const bool elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, neighbor);
      const FaceInfo * const fi = _mesh.faceInfo(
          elem_has_fi ? elem : neighbor, elem_has_fi ? side : neighbor->which_neighbor_am_i(elem));
      const Point & normal = elem_has_fi ? fi->normal() : Point(-fi->normal());

      const Point area_vector = normal * fi->faceArea();
      const ValueType face_value = this->evaluate(fi);

      const auto product = Moose::outer_product(normal, area_vector);

      const auto flux_contrib = normal * (face_value * area_vector);
      for (const auto i : make_range(dim))
      {
        sum_normal_flux(i) += flux_contrib(i);
        for (const auto j : make_range(dim))
          n_x_Sf(i, j) += product(i, j);
      }
    }

    // We do the inversion of the surface vector matrix here. It is symmetric
    // and small so we can do it using a Cholesky decomposition.
    DenseVector<PrimitiveType> dense_result(dim);
    n_x_Sf.cholesky_solve(sum_normal_flux, dense_result);

    ValueType result;
    for (const auto i : make_range(dim))
      result(i) = dense_result(i);

    return result;
  }
  else
    mooseError("Cell center reconstruction is not implemented!");
}

template <typename T, typename Map>
typename FaceCenteredMapFunctor<T, Map>::ValueType
FaceCenteredMapFunctor<T, Map>::evaluate(const FaceArg & face, const StateArg &) const
{
  return this->evaluate(face.fi);
}

template <typename T, typename Map>
typename FaceCenteredMapFunctor<T, Map>::ValueType
FaceCenteredMapFunctor<T, Map>::evaluate(const FaceInfo * fi) const
{
  try
  {
    return libmesh_map_find(*this, fi->id());
  }
  catch (libMesh::LogicError &)
  {
    if (!_sub_ids.empty() && !_sub_ids.count(fi->elem().subdomain_id()))
    {
      if (fi->neighborPtr() && !_sub_ids.count(fi->neighborPtr()->subdomain_id()))
        mooseError("Attempted to evaluate FaceCenteredMapFunctor '",
                   this->functorName(),
                   "' with an element subdomain id of '",
                   fi->elem().subdomain_id(),
                   fi->neighborPtr() ? " or neighbor subdomain id of '" +
                                           std::to_string(fi->neighborPtr()->subdomain_id()) + "'"
                                     : "",
                   "' but that subdomain id is not one of the subdomain ids the functor is "
                   "restricted to.");
    }
    else
      mooseError("Attempted access into FaceCenteredMapFunctor '",
                 this->functorName(),
                 "' with a key that does not yet exist in the map. Make sure to fill your "
                 "FaceCenteredMapFunctor for all elements you will attempt to access later.");

    return typename FaceCenteredMapFunctor<T, Map>::ValueType();
  }
}

template class FaceCenteredMapFunctor<ADRealVectorValue,
                                      std::unordered_map<dof_id_type, ADRealVectorValue>>;
template class FaceCenteredMapFunctor<RealVectorValue,
                                      std::unordered_map<dof_id_type, RealVectorValue>>;
