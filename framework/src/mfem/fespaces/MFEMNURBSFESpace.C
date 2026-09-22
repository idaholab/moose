//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNURBSFESpace.h"

registerMooseObject("MooseApp", MFEMNURBSFESpace);

InputParameters
MFEMNURBSFESpace::validParams()
{
  InputParameters params = MFEMSimplifiedFESpace::validParams();
  params.addClassDescription("Class to construct a scalar finite element space of NURBS basis "
                             "functions, for isogeometric analysis (IGA). The mesh the space is "
                             "defined on must be a NURBS mesh.");
  return params;
}

MFEMNURBSFESpace::MFEMNURBSFESpace(const InputParameters & parameters)
  : MFEMSimplifiedFESpace(parameters)
{
}

std::string
MFEMNURBSFESpace::getFECName() const
{
  // NURBS collections are named solely by their order; unlike the other families they have
  // no basis or reference dimension to encode, as the mesh knot vectors supply both.
  return "NURBS" + std::to_string(_fec_order);
}

int
MFEMNURBSFESpace::getVDim() const
{
  return 1;
}

std::shared_ptr<mfem::ParFiniteElementSpace>
MFEMNURBSFESpace::buildFESpace() const
{
  if (!_pmesh.NURBSext)
    mooseError("A NURBS finite element space can only be built on a NURBS mesh. Either set the "
               "'file' parameter of the mesh to an MFEM NURBS mesh file, or, if the 'submesh' "
               "parameter is set, note that submeshes do not inherit the NURBS geometry of the "
               "mesh they are extracted from.");

  const mfem::Array<int> & mesh_orders = _pmesh.NURBSext->GetOrders();

  // MFEM can only degree elevate the knot vectors of the mesh, never reduce their order, so a
  // space coarser than the geometry would silently be built at the order of the geometry.
  if (_fec_order < mesh_orders.Max())
    paramError("fec_order",
               "Requested order ",
               _fec_order,
               " is lower than the order ",
               mesh_orders.Max(),
               " of the NURBS geometry of the mesh. NURBS finite element spaces must be at "
               "least of the order of the mesh they are defined on.");

  // A NURBS extension is only needed for superparametric spaces; passing a null extension builds
  // the isoparametric space on the extension owned by the mesh. The extension built here is
  // handed over to the fespace, which replaces it with a ParNURBSExtension matching the mesh
  // partitioning and takes ownership of it.
  mfem::NURBSExtension * fespace_ext = _fec_order > mesh_orders.Min()
                                           ? new mfem::NURBSExtension(_pmesh.NURBSext, _fec_order)
                                           : nullptr;

  return std::make_shared<mfem::ParFiniteElementSpace>(
      &_pmesh, fespace_ext, getFEC().get(), getVDim(), _ordering);
}

#endif
