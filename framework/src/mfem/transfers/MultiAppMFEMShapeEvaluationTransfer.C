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

void
MultiAppMFEMShapeEvaluationTransfer::transferVariables()
{
  // Get GridFunction from problem by name. For complex variables, return the real component.
  auto getGridFunction = [&](MFEMProblem & problem,
                             const std::string & name,
                             bool & is_complex) -> mfem::ParGridFunction &
  {
    if (problem.getProblemData().gridfunctions.Has(name))
    {
      is_complex = false;
      return *problem.getProblemData().gridfunctions.Get(name);
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
    bool is_from_complex;
    bool is_to_complex;
    mfem::ParGridFunction & from_gf =
        getGridFunction(getActiveFromProblem(), getFromVarName(v), is_from_complex);
    mfem::ParGridFunction & to_gf =
        getGridFunction(getActiveToProblem(), getToVarName(v), is_to_complex);

    mfem::ParFiniteElementSpace & from_pfespace = *from_gf.ParFESpace();
    mfem::ParFiniteElementSpace & to_pfespace = *to_gf.ParFESpace();

    from_pfespace.GetParMesh()->EnsureNodes();
    to_pfespace.GetParMesh()->EnsureNodes();

    // Generate list of points where the grid function will be evaluated
    mfem::Vector vxyz;
    mfem::Ordering::Type point_ordering;
    _mfem_projector.extractNodePositions(to_pfespace, vxyz, point_ordering);

    // Evaluate source grid function at target points
    const int dim = to_pfespace.GetParMesh()->Dimension();
    const int nodes_cnt = vxyz.Size() / dim;
    const int to_gf_ncomp = to_gf.VectorDim();
    mfem::Vector interp_vals(nodes_cnt * to_gf_ncomp);
    _mfem_interpolator.SetDefaultInterpolationValue(getMFEMOutOfMeshValue());

    _mfem_interpolator.Interpolate(
        *from_gf.ParFESpace()->GetParMesh(), vxyz, from_gf, interp_vals, point_ordering);
    _mfem_projector.projectNodalValues(interp_vals, to_pfespace.GetOrdering(), to_gf);

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
            *from_gf_im.ParFESpace()->GetParMesh(), vxyz, from_gf_im, interp_vals, point_ordering);
        _mfem_projector.projectNodalValues(interp_vals, to_pfespace.GetOrdering(), to_gf_im);
      }
      else // Transfer from real variable to complex variable, so imag component zero
        to_gf_im = 0.0;
    }
  }
}

#endif
