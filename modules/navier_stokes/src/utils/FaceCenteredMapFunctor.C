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

  RealTensorValue n_x_Sf;
  typename Moose::FunctorBase<T>::ValueType reconstructed_value;

  for (const auto side : make_range(elem_arg.elem->n_sides()))
  {
    const FaceInfo * const fi = _mesh.faceInfo(elem_arg.elem, side);
    const auto & normal = fi->normal();
    const auto area = fi->faceArea();
    typename Moose::FunctorBase<T>::ValueType face_value = this->evaluate(fi);

    n_x_Sf += Moose::outer_product(normal, normal) * area;
    reconstructed_value += normal * (face_value * normal * area);
  }

  return n_x_Sf.inverse() * reconstructed_value;
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

template <typename T, typename Map>
typename Moose::FunctorBase<T>::ValueType
FaceCenteredMapFunctor<T, Map>::evaluate(const FaceArg & face, unsigned int) const
{
  const FaceInfo * const fi = face.fi;
  return this->evaluate(fi);
}

template class FaceCenteredMapFunctor<ADRealVectorValue,
                                      std::unordered_map<dof_id_type, ADRealVectorValue>>;
