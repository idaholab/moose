//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MultiAppMFEMShapeEvaluationTransfer.h"

registerMooseObject("MooseApp", MultiAppMFEMShapeEvaluationTransfer);

InputParameters
MultiAppMFEMShapeEvaluationTransfer::validParams()
{
  InputParameters params = MFEMMultiAppTransfer::validParams();
  params.addClassDescription("Transfers variable values from one MFEM application to another using "
                             "shape function evaluations.");
  return params;
}

MultiAppMFEMShapeEvaluationTransfer::MultiAppMFEMShapeEvaluationTransfer(
    InputParameters const & params)
  : MFEMMultiAppTransfer(params)
{
  checkValidTransferProblemTypes<MFEMProblem, MFEMProblem>();
}

MFEMProblem &
MultiAppMFEMShapeEvaluationTransfer::getActiveFromProblem()
{
  MFEMProblem * mfem_from_problem =
      dynamic_cast<MFEMProblem *>(&MFEMMultiAppTransfer::getActiveFromProblem());
  if (!mfem_from_problem)
    mooseError("Transfer source problem is not an MFEM problem");
  return *mfem_from_problem;
}

MFEMProblem &
MultiAppMFEMShapeEvaluationTransfer::getActiveToProblem()
{
  MFEMProblem * mfem_to_problem =
      dynamic_cast<MFEMProblem *>(&MFEMMultiAppTransfer::getActiveToProblem());
  if (!mfem_to_problem)
    mooseError("Transfer destination problem is not an MFEM problem");
  return *mfem_to_problem;
}

void
MultiAppMFEMShapeEvaluationTransfer::transferVariables(bool is_target_local)
{
  auto transformTargetPointsToSourceFrame =
      [this](mfem::Vector & point_coordinates, const unsigned int dimension)
  {
    mooseAssert(dimension == 2 || dimension == 3, "Target finite element space must be 2D or 3D");

    const auto n_points = point_coordinates.Size() / dimension;
    for (const auto i : make_range(n_points))
    {
      Point point_in_target_frame;
      switch (dimension)
      {
        case 3:
          point_in_target_frame(2) = point_coordinates[i + 2 * n_points];
          [[fallthrough]];
        case 2:
          point_in_target_frame(1) = point_coordinates[i + n_points];
          [[fallthrough]];
        case 1:
          point_in_target_frame(0) = point_coordinates[i];
      }
      const auto point_in_source_frame = mapPointToActiveSourceFrame(point_in_target_frame);
      point_coordinates[i] = point_in_source_frame(0);
      point_coordinates[i + n_points] = point_in_source_frame(1);
      if (dimension == 3)
        point_coordinates[i + 2 * n_points] = point_in_source_frame(2);
    }
  };

  // Get GridFunction from problem by name. For complex variables, return the real component.
  auto getGridFunction = [&](MFEMProblem & problem,
                             const std::string & name,
                             bool & is_complex) -> mfem::ParGridFunction &
  {
    if (problem.getProblemData().gridfunctions.Has(name))
    {
      is_complex = false;
      return *problem.getGridFunction(name);
    }
    if (problem.getProblemData().cmplx_gridfunctions.Has(name))
    {
      is_complex = true;
      return problem.getProblemData().cmplx_gridfunctions.Get(name)->real();
    }
    mooseError("No real or complex variable named '", name, "' found.");
  };

  for (const auto v : make_range(numToVar()))
  {
    bool is_from_complex{false};
    mfem::ParGridFunction & from_gf =
        getGridFunction(getActiveFromProblem(), getFromVarName(v), is_from_complex);
    mfem::ParFiniteElementSpace & from_pfespace = *from_gf.ParFESpace();
    from_pfespace.GetParMesh()->EnsureNodes();

    bool is_to_complex{false};
    // Vector to store position coords of nodes/interpolation pts
    mfem::Vector vxyz;
    // Ordering of node coordinates x,y,z
    mfem::Ordering::Type point_ordering = mfem::Ordering::Type::byNODES;
    // Ordering of (vector) GridFunction data
    mfem::Ordering::Type to_gf_ordering = mfem::Ordering::Type::byVDIM;
    // Vector to store GridFunction values at interp. pts
    mfem::Vector interp_vals;

    // Get points from target GridFunction on local rank
    if (is_target_local)
    {
      mfem::ParGridFunction & to_gf =
          getGridFunction(getActiveToProblem(), getToVarName(v), is_to_complex);
      mfem::ParFiniteElementSpace & to_pfespace = *to_gf.ParFESpace();
      to_pfespace.GetParMesh()->EnsureNodes();
      // Generate list of points where the grid function will be evaluated
      _mfem_projector.extractNodePositions(to_pfespace, vxyz, point_ordering);
      // Evaluate source grid function at target points
      const int dim = to_pfespace.GetParMesh()->Dimension();
      transformTargetPointsToSourceFrame(vxyz, dim);
      // Update ordering for interpolation
      to_gf_ordering = to_pfespace.GetOrdering();
    }

    // Evaluate source grid function at target points.
    // Interpolation must be performed over all source ranks.
    _mfem_interpolator.Setup(*from_gf.ParFESpace()->GetParMesh());
    _mfem_interpolator.SetDefaultInterpolationValue(getMFEMOutOfMeshValue());
    _mfem_interpolator.Interpolate(vxyz, from_gf, interp_vals, point_ordering, to_gf_ordering);

    // Project interpolated field onto target GridFunction data on the local rank
    if (is_target_local)
    {
      mfem::ParGridFunction & to_gf =
          getGridFunction(getActiveToProblem(), getToVarName(v), is_to_complex);
      mfem::ParFiniteElementSpace & to_pfespace = *to_gf.ParFESpace();
      _mfem_projector.projectNodalValues(interp_vals, to_gf_ordering, to_gf);
      if (is_to_complex)
      {
        // Get remaining imaginary component of destination GridFunction
        mfem::ParGridFunction & to_gf_im =
            getActiveToProblem().getProblemData().cmplx_gridfunctions.Get(getToVarName(v))->imag();
        if (is_from_complex)
        {
          mfem::ParGridFunction & from_gf_im = getActiveFromProblem()
                                                   .getProblemData()
                                                   .cmplx_gridfunctions.Get(getFromVarName(v))
                                                   ->imag();
          _mfem_interpolator.Interpolate(
              vxyz, from_gf_im, interp_vals, point_ordering, to_pfespace.GetOrdering());
          _mfem_projector.projectNodalValues(interp_vals, to_pfespace.GetOrdering(), to_gf_im);
        }
        else // Transfer from real variable to complex variable, so imag component zero
          to_gf_im = 0.0;
      }
    }
  }
}

#endif
