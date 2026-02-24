//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MultiAppMFEMGeneralFieldTransfer.h"
#include "FEProblemBase.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include "MFEMProblem.h"
#include "MFEMMesh.h"

registerMooseObject("MooseApp", MultiAppMFEMGeneralFieldTransfer);

InputParameters
MultiAppMFEMGeneralFieldTransfer::validParams()
{
  InputParameters params = MultiAppMFEMGeneralFieldTransferBase::validParams();
  params.addClassDescription("Copies variable values from one MFEM application to another");
  return params;
}

MultiAppMFEMGeneralFieldTransfer::MultiAppMFEMGeneralFieldTransfer(InputParameters const & params)
  : MultiAppMFEMGeneralFieldTransferBase(params)
{
  checkValidTransferProblemTypes<MFEMProblem, MFEMProblem>();
}

void
MultiAppMFEMGeneralFieldTransfer::transferVariables()
{
  for (const auto v : make_range(numToVar()))
  {
    mfem::ParGridFunction & from_gf = *getActiveFromProblem().getProblemData().gridfunctions.Get(getFromVarName(v));
    mfem::ParGridFunction & to_gf = *getActiveToProblem().getProblemData().gridfunctions.Get(getToVarName(v));

    mfem::ParFiniteElementSpace & from_pfespace = *from_gf.ParFESpace();
    mfem::ParFiniteElementSpace & to_pfespace = *to_gf.ParFESpace();

    from_pfespace.GetParMesh()->EnsureNodes();
    to_pfespace.GetParMesh()->EnsureNodes();
    
    // Generate list of points where the grid function will be evaluated
    mfem::Vector vxyz;
    mfem::Ordering::Type point_ordering;    
    extractProjectionPoints(to_pfespace, vxyz, point_ordering);

    // Evaluate source grid function at target points
    const int dim = to_pfespace.GetParMesh()->Dimension();
    const int nodes_cnt = vxyz.Size() / dim;
    const int to_gf_ncomp = to_gf.VectorDim();
    mfem::Vector interp_vals(nodes_cnt*to_gf_ncomp);
    _mfem_interpolator.SetDefaultInterpolationValue(std::numeric_limits<mfem::real_t>::infinity());
    _mfem_interpolator.Interpolate(*from_gf.ParFESpace()->GetParMesh(), vxyz, from_gf, interp_vals, point_ordering);
    
    projectValues(interp_vals, to_pfespace.GetOrdering(), to_gf);
  }
}

#endif
