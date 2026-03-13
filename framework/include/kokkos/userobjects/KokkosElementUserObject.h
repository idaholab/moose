//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosUserObject.h"
#include "KokkosElementReducer.h"

#include "MaterialPropertyInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TransientInterface.h"
#include "RandomInterface.h"
#include "ElementIDInterface.h"

namespace Moose::Kokkos
{

class ElementUserObject : public UserObject,
                          public ElementReducer,
                          public ::MaterialPropertyInterface,
                          public ::CoupleableMooseVariableDependencyIntermediateInterface,
                          public ::TransientInterface,
                          public ::RandomInterface,
                          public ::ElementIDInterface
{
public:
  static InputParameters validParams();

  ElementUserObject(const InputParameters & parameters);

  /**
   * Copy constructor for parallel dispatch
   */
  ElementUserObject(const ElementUserObject & object);

  virtual void compute() override;

  /**
   * The parallel computation entry function called by Kokkos
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(DefaultLoop, const ThreadID tid, const Derived & object) const;

  /**
   * Shim for hook method that can be leveraged to implement static polymorphism
   */
  template <typename Derived>
  KOKKOS_FUNCTION void executeShim(const Derived & object, Datum & datum) const
  {
    object.execute(datum);
  }

  using ElementReducer::operator();
  using ElementReducer::executeShim;

protected:
  virtual void dispatch();
};

template <typename Derived>
KOKKOS_FUNCTION void
ElementUserObject::operator()(DefaultLoop, const ThreadID tid, const Derived & object) const
{
  auto elem = kokkosBlockElementID(tid);

  Datum datum(elem, libMesh::invalid_uint, kokkosAssembly(), kokkosSystems());

  object.executeShim(object, datum);
}

} // namespace Moose::Kokkos
