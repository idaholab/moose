//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVFDataKernel.h"

InputParameters
INSFVFDataKernel::validParams()
{
  auto params = FVElementalKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  // We need two ghost layers for the Moukalled body-force correction to the Rhie-Chow velocity
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

INSFVFDataKernel::INSFVFDataKernel(const InputParameters & params)
  : FVElementalKernel(params), INSFVMomentumResidualObject(*this)
{
  std::vector<std::string> tagging_params = {
      "vector_tags", "matrix_tags", "extra_vector_tags", "extra_matrix_tags"};
  for (const auto & tparam : tagging_params)
    if (params.isParamSetByUser(tparam))
      paramError(tparam,
                 "Tagging parameters have no effect if set on an 'INSFVFDataKernel'. Please set '",
                 tparam,
                 "' on the associated Rhie-Chow user-object '",
                 _rc_uo.name(),
                 "' instead.");

  _rc_uo.hasFData(blockRestricted() ? blockIDs() : _mesh.meshSubdomains());
}
