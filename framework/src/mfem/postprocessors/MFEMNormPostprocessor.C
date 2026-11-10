//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNormPostprocessor.h"

InputParameters
MFEMNormPostprocessor::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params.addParam<unsigned int>(
      "quadrature_order",
      "Order of the quadrature rule used on every element. Defaults to 2p+3 on each element, "
      "where p is the polynomial order of the variable's element.");
  return params;
}

MFEMNormPostprocessor::MFEMNormPostprocessor(const InputParameters & parameters)
  : MFEMPostprocessor(parameters), _irs{}
{
  if (isParamValid("quadrature_order"))
    for (const auto geom : make_range(int(mfem::Geometry::NumGeom)))
      _irs[geom] = &mfem::IntRules.Get(geom, getParam<unsigned int>("quadrature_order"));
}

const mfem::IntegrationRule **
MFEMNormPostprocessor::getIntegrationRules() const
{
  return isParamValid("quadrature_order") ? _irs.data() : nullptr;
}

#endif
