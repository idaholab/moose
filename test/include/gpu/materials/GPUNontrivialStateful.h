//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUMaterial.h"

class NotTriviallyCopyable
{
public:
  NotTriviallyCopyable() = default;
  NotTriviallyCopyable(const NotTriviallyCopyable &) {}
  NotTriviallyCopyable & operator=(const NotTriviallyCopyable &) { return *this; }
};

namespace Moose
{
namespace Kokkos
{
template <>
struct ArrayDeepCopy<NotTriviallyCopyable>
{
  static const bool value = true;
};
} // namespace Kokkos
} // namespace Moose

class KokkosNontrivialStateful final : public Moose::Kokkos::Material<KokkosNontrivialStateful>
{
public:
  static InputParameters validParams();

  KokkosNontrivialStateful(const InputParameters & parameters);

  KOKKOS_FUNCTION void computeQpProperties(const unsigned int /* qp */, Datum & /* datum */) const
  {
  }

protected:
  Moose::Kokkos::MaterialProperty<NotTriviallyCopyable> _prop;
  Moose::Kokkos::MaterialProperty<NotTriviallyCopyable> _old_prop;
};
