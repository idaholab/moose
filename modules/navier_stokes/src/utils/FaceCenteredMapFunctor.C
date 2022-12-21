//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FaceCenteredMapFunctor.h"

template <typename T, typename Map>
typename Moose::FunctorBase<T>::ValueType
FaceCenteredMapFunctor<T, Map>::evaluate(const ElemArg & elem_arg, unsigned int) const
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

  using ValueType = typename Moose::FunctorBase<T>::ValueType;
  using PrimitiveType = typename MetaPhysicL::ReplaceAlgebraicType<
      T,
      typename TensorTools::DecrementRank<typename MetaPhysicL::ValueType<T>::type>::type>::type;

  if constexpr (libMesh::TensorTools::TensorTraits<ValueType>::rank == 1)
  {
    const auto dim = _mesh.dimension();
    // The reason why this is a
    DenseMatrix<PrimitiveType> n_x_Sf(dim, dim);
    DenseVector<PrimitiveType> sum_normal_flux(dim);

    const Elem * elem = elem_arg.elem;

    for (const auto side : make_range(elem->n_sides()))
    {
      const Elem * neighbor = elem->neighbor_ptr(side);
      const bool elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, neighbor);
      const FaceInfo * const fi = _mesh.faceInfo(
          elem_has_fi ? elem : neighbor, elem_has_fi ? side : neighbor->which_neighbor_am_i(elem));
      const Point & normal = elem_has_fi ? fi->normal() : Point(-fi->normal());
      const Real area = fi->faceArea();

      ValueType face_value = this->evaluate(fi);

      const auto product = Moose::outer_product(normal, normal * area);
      const auto flux_contrib = normal * (face_value * normal * area);
      for (auto i : make_range(dim))
      {
        sum_normal_flux(i) += flux_contrib(i);
        for (auto j : make_range(dim))
          n_x_Sf(i, j) += product(i, j);
      }
    }

    DenseVector<PrimitiveType> dense_result(dim);
    n_x_Sf.lu_solve(sum_normal_flux, dense_result);

    ValueType result;
    for (auto i : make_range(dim))
      result(i) = dense_result(i);

    return result;
  }
  else
    mooseError("Cell center reconstruction is not implemented!");
}

template <typename T, typename Map>
typename Moose::FunctorBase<T>::ValueType
FaceCenteredMapFunctor<T, Map>::evaluate(const FaceArg & face, unsigned int) const
{
  const FaceInfo * const fi = face.fi;
  return this->evaluate(fi);
}

template <typename T, typename Map>
typename Moose::FunctorBase<T>::ValueType
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

    return typename Moose::FunctorBase<T>::ValueType();
  }
}

template class FaceCenteredMapFunctor<ADRealVectorValue,
                                      std::unordered_map<dof_id_type, ADRealVectorValue>>;
template class FaceCenteredMapFunctor<RealVectorValue,
                                      std::unordered_map<dof_id_type, RealVectorValue>>;
