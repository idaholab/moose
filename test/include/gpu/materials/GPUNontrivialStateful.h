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
};

class GPUNontrivialStateful final : public GPUMaterial<GPUNontrivialStateful>
{
public:
  static InputParameters validParams();

  GPUNontrivialStateful(const InputParameters & parameters);

  KOKKOS_FUNCTION void computeQpProperties(const unsigned int /* qp */, Datum & /* datum */) const
  {
  }

protected:
  GPUMaterialProperty<NotTriviallyCopyable> _prop;
  GPUMaterialProperty<NotTriviallyCopyable> _old_prop;
};
