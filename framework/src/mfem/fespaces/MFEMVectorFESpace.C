//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMVectorFESpace.h"

registerMooseObject("MooseApp", MFEMVectorFESpace);

InputParameters
MFEMVectorFESpace::validParams()
{
  InputParameters params = MFEMSimplifiedFESpace::validParams();
  params.addClassDescription(
      "Convenience class to construct vector finite element spaces, abstracting away some of the "
      "mathematical complexity of specifying the dimensions.");
  MooseEnum fec_types("H1 ND RT L2", "H1", true);
  params.addParam<MooseEnum>("fec_type", fec_types, "Specifies the family of FE shape functions.");
  params.addParam<std::string>("closed_basis",
                               "GaussLobatto",
                               "Specifies the closed quadrature basis used for vector elements.");
  params.addParam<std::string>("open_basis",
                               "GaussLegendre",
                               "Specifies the open quadrature basis used for vector elements.");
  params.addParam<int>("range_dim",
                       0,
                       "The number of components of the vectors in reference space. Zero "
                       "(the default) means it will be the same as the problem dimension. "
                       "Note that MFEM does not currently support 2D vectors in 1D space "
                       "for ND and RT elements.");
  return params;
}

MFEMVectorFESpace::MFEMVectorFESpace(const InputParameters & parameters)
  : MFEMSimplifiedFESpace(parameters),
    _fec_type(parameters.get<MooseEnum>("fec_type")),
    _range_dim(parameters.get<int>("range_dim"))
{
}

std::string
MFEMVectorFESpace::getFECName() const
{
  const int pdim = getProblemDim();
  std::string actual_type = _fec_type;
  if ((_fec_type == "ND" || _fec_type == "RT") && _range_dim != 0 && _range_dim != pdim)
  {
    if (_range_dim != 3)
      mooseError("No  " + _fec_type + " finite element collection available for " +
                 std::to_string(_range_dim) + "D vectors in " + std::to_string(pdim) + "D space.");
    actual_type += "_R" + std::to_string(pdim) + "D";
  }

<<<<<<< HEAD
  return actual_type + "_" + std::to_string(pdim) + "D_P" + std::to_string(_fec_order);
=======
  char cb = mfem::BasisType::GetChar(getBasis(getParam<std::string>("closed_basis")));
  std::string closed_basis(1, cb);
  char ob = mfem::BasisType::GetChar(getBasis(getParam<std::string>("open_basis")));
  std::string open_basis(1, ob);
  std::string basis = (closed_basis + open_basis == "Gg" ? "" : "@" + closed_basis + open_basis);

  return actual_type + basis + "_" + std::to_string(pdim) + "D_P" + std::to_string(_fec_order);
>>>>>>> 6164145e86 (Fix default basis choice MFEM errors)
}

int
MFEMVectorFESpace::getVDim() const
{
  if (_fec_type == "H1" || _fec_type == "L2")
  {
    return _range_dim == 0 ? getProblemDim() : _range_dim;
  }
  else
  {
    return 1;
  }
}

#endif
