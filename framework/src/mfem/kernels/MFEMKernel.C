//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMKernel.h"
#include "MFEMProblem.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"

InputParameters
MFEMKernel::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("Kernel");
  params.addParam<VariableName>("variable",
                                "Variable labelling the weak form this kernel is added to");
  params.addParam<std::vector<SubdomainName>>("block",
                                              {},
                                              "The list of blocks (ids) that this "
                                              "object will be applied to. Leave empty to apply "
                                              "to all blocks.");
  return params;
}

MFEMKernel::MFEMKernel(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _test_var_name(getParam<VariableName>("variable")),
    _subdomain_names(getParam<std::vector<SubdomainName>>("block")),
    _subdomain_attributes(_subdomain_names.size())
{
  for (unsigned int i = 0; i < _subdomain_names.size(); ++i)
    _subdomain_attributes[i] = std::stoi(_subdomain_names[i]);
  mfem::ParMesh & mesh(getMFEMProblem().mesh().getMFEMParMesh());
  if (!_subdomain_attributes.IsEmpty())
    mfem::common::AttrToMarker(mesh.attributes.Max(), _subdomain_attributes, _subdomain_markers);
}

#endif
