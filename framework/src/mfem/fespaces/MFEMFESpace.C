//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMFESpace.h"
#include "MFEMProblem.h"

InputParameters
MFEMFESpace::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMFESpace");
  MooseEnum ordering("NODES VDIM", "VDIM", false);
  params.addParam<MooseEnum>("ordering", ordering, "Ordering style to use for vector DoFs.");
  params.addParam<int>("vdim", 1, "The number of degrees of freedom per basis function.");
  params.addParam<std::string>("submesh",
                               "Submesh to define the FESpace on. Leave blank to use base mesh.");
  return params;
}

MFEMFESpace::MFEMFESpace(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _ordering(parameters.get<MooseEnum>("ordering")),
    _pmesh(
        parameters.isParamValid("submesh")
            ? getMFEMProblem().getProblemData().submeshes.GetRef(getParam<std::string>("submesh"))
            : const_cast<mfem::ParMesh &>(getMFEMProblem().mesh().getMFEMParMesh()))
{
}

void
MFEMFESpace::buildFEC() const
{
  _fec = std::shared_ptr<mfem::FiniteElementCollection>(
      mfem::FiniteElementCollection::New(getFECName().c_str()));
}

void
MFEMFESpace::buildFESpace() const
{
  _fespace =
      std::make_shared<mfem::ParFiniteElementSpace>(&_pmesh, getFEC().get(), getVDim(), _ordering);
}

#endif
