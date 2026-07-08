//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMFESpaceHierarchy.h"
#include "MFEMFESpace.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMFESpaceHierarchy);

InputParameters
MFEMFESpaceHierarchy::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params.registerBase("MFEMFESpaceHierarchy");
  params.registerSystemAttributeName("MFEMFESpaceHierarchy");
  params.addClassDescription(
      "Builds a mfem::ParFiniteElementSpaceHierarchy from a base FESpace by applying a "
      "sequence of h-refinements ('h') and/or p-refinements (integer order strings).");
  params.addRequiredParam<MFEMFESpaceName>("fespace",
                                           "Name of the base (coarsest-level) FESpace object.");
  params.addRequiredParam<std::vector<std::string>>(
      "refinements",
      "Ordered sequence of refinements. Each entry is 'h' for uniform h-refinement or "
      "an integer string for p-refinement to that polynomial order (must be strictly "
      "increasing across p-entries).");
  return params;
}

MFEMFESpaceHierarchy::MFEMFESpaceHierarchy(const InputParameters & parameters)
  : MFEMObject(parameters)
{
  buildHierarchy();
}

void
MFEMFESpaceHierarchy::buildHierarchy()
{
  const auto & fespace_name = getParam<MFEMFESpaceName>("fespace");
  const auto & refinements = getParam<std::vector<std::string>>("refinements");

  if (refinements.empty())
    paramError("refinements", "must not be empty.");

  const auto & base_fespace_object =
      getMFEMProblem().getMFEMObject<MFEMFESpace>("MFEMFESpace", fespace_name);
  _is_scalar = base_fespace_object.isScalar();
  _is_vector = base_fespace_object.isVector();

  // Retrieve the base (coarsest) ParFiniteElementSpace from ProblemData.
  auto base_fespace = base_fespace_object.getFESpace();
  mfem::ParMesh * base_mesh = base_fespace->GetParMesh();
  const int vdim = base_fespace->GetVDim();
  const int ordering = base_fespace->GetOrdering();

  // Construct the hierarchy with the coarse level.
  // MOOSE owns the mesh and fespace, so pass ownM=false, ownFES=false.
  _hierarchy = std::make_shared<mfem::ParFiniteElementSpaceHierarchy>(
      base_mesh, base_fespace.get(), /*ownM=*/false, /*ownFES=*/false);

  for (const auto & entry : refinements)
  {
    if (entry == "h")
      _hierarchy->AddUniformlyRefinedLevel(vdim, ordering);
    else
    {
      // p-refinement: parse the target polynomial order.
      int new_order = 0;
      try
      {
        new_order = std::stoi(entry);
      }
      catch (const std::exception &)
      {
        paramError("refinements",
                   "entry '",
                   entry,
                   "' is neither 'h' nor a valid integer polynomial order.");
      }

      if (new_order < 1)
        paramError("refinements", "p-refinement order must be >= 1, got ", new_order, ".");

      // Derive the new FEC name from the current finest level's FEC.
      const std::string current_fec_name = _hierarchy->GetFinestFESpace().FEColl()->Name();
      const auto pos = current_fec_name.rfind("_P");
      if (pos == std::string::npos)
        paramError("refinements",
                   "cannot infer polynomial order from FEC name '",
                   current_fec_name,
                   "' - expected a '_P<N>' suffix.");

      const int current_order = std::stoi(current_fec_name.substr(pos + 2));
      if (new_order <= current_order)
        paramError("refinements",
                   "p-refinement orders must be strictly increasing; requested order ",
                   new_order,
                   " is not greater than the current finest level's order ",
                   current_order,
                   ".");

      const std::string new_fec_name =
          current_fec_name.substr(0, pos + 2) + std::to_string(new_order);

      auto new_fec = std::unique_ptr<mfem::FiniteElementCollection>(
          mfem::FiniteElementCollection::New(new_fec_name.c_str()));
      _hierarchy->AddOrderRefinedLevel(new_fec.get(), vdim, ordering);
      _level_fecs.push_back(std::move(new_fec));
    }
  }
}
#endif
