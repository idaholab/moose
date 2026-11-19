//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FaceCenteredMapFunctor.h"
#include "FaceInfo.h"
#include "libmesh/elem.h"

template <typename T, typename Map>
typename FaceCenteredMapFunctor<T, Map>::ValueType
FaceCenteredMapFunctor<T, Map>::evaluate(const FaceInfo * fi) const
{
  auto it = this->find(fi->id());
  if (it == this->end())
  {
    if (!this->_sub_ids.empty() && !this->_sub_ids.count(fi->elem().subdomain_id()))
    {
      if (fi->neighborPtr() && !this->_sub_ids.count(fi->neighborPtr()->subdomain_id()))
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
                 "' for face id ",
                 fi->id(),
                 " with elem subdomain id ",
                 fi->elem().subdomain_id(),
                 fi->neighborPtr() ? " and neighbor subdomain id " : "",
                 fi->neighborPtr() ? std::to_string(fi->neighborPtr()->subdomain_id()) : "",
                 " with a key that does not yet exist in the map. Make sure to fill your "
                 "FaceCenteredMapFunctor for all elements you will attempt to access later.");

    return typename FaceCenteredMapFunctor<T, Map>::ValueType();
  }

  return it->second;
}

template class FaceCenteredMapFunctor<ADRealVectorValue,
                                      std::unordered_map<dof_id_type, ADRealVectorValue>>;
template class FaceCenteredMapFunctor<RealVectorValue,
                                      std::unordered_map<dof_id_type, RealVectorValue>>;
template class FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>;
