//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMBoundaryNetFluxPostprocessor.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMBoundaryNetFluxPostprocessor);

InputParameters
MFEMBoundaryNetFluxPostprocessor::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params += MFEMBoundaryRestrictable::validParams();
  params.addClassDescription(
      "Calculates the total flux of a vector field through an interface");
  params.addParam<VariableName>("variable",
                                "Name of the vector variable whose normal component will be averaged over the boundary.");
  return params;
}

MFEMBoundaryNetFluxPostprocessor::MFEMBoundaryNetFluxPostprocessor(const InputParameters & parameters)
  : MFEMPostprocessor(parameters),
    MFEMBoundaryRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    _var(getMFEMProblem().getProblemData().gridfunctions.GetRef(getParam<VariableName>("variable")))
{
}

void
MFEMBoundaryNetFluxPostprocessor::initialize()
{
}

void
MFEMBoundaryNetFluxPostprocessor::execute()
{
  double flux = 0.0;
  double area = 0.0;

  mfem::FiniteElementSpace *var_fespace = _var.FESpace();
  mfem::Mesh *mesh = var_fespace->GetMesh();

  mfem::Vector local_dofs, normal_vec;
  mfem::DenseMatrix dshape;
  mfem::Array<int> dof_ids;
  const mfem::Array<int> & boundary_attributes = getBoundaryAttributes();
  for (int i = 0; i < mesh->GetNBE(); i++) 
  {
    for (int bdr_attr_i = 0; bdr_attr_i < boundary_attributes.Size(); ++bdr_attr_i)
    {
      if (mesh->GetBdrAttribute(i) == boundary_attributes[bdr_attr_i])
      {
        mfem::FaceElementTransformations *face_transforms =
        mesh->GetFaceElementTransformations(mesh->GetBdrElementFaceIndex(i));
        if (face_transforms == nullptr)
          continue;

        const mfem::FiniteElement &elem = *var_fespace->GetFE(face_transforms->Elem1No);
        const int int_order = 2 * elem.GetOrder() + 3;
        const mfem::IntegrationRule &ir =
            mfem::IntRules.Get(face_transforms->FaceGeom, int_order);

        var_fespace->GetElementDofs(face_transforms->Elem1No, dof_ids);
        _var.GetSubVector(dof_ids, local_dofs);
        const int space_dim = face_transforms->Face->GetSpaceDim();
        normal_vec.SetSize(space_dim);
        dshape.SetSize(elem.GetDof(), space_dim);

        for (int j = 0; j < ir.GetNPoints(); j++) {

          const mfem::IntegrationPoint &ip = ir.IntPoint(j);
          mfem::IntegrationPoint eip;
          face_transforms->Loc1.Transform(ip, eip);
          face_transforms->Face->SetIntPoint(&ip);
          double face_weight = face_transforms->Face->Weight();
          double val = 0.0;
          face_transforms->Elem1->SetIntPoint(&eip);
          elem.CalcVShape(*face_transforms->Elem1, dshape);
          mfem::CalcOrtho(face_transforms->Face->Jacobian(), normal_vec);
          val += dshape.InnerProduct(normal_vec, local_dofs) / face_weight;

          // Measure the area of the boundary
          area += ip.weight * face_weight;

          // Integrate alpha * n.Grad(x) + beta * x
          flux += val * ip.weight * face_weight;
        }
      }
    }
  }
  MPI_Allreduce(&flux, &_total_flux, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);  
}

PostprocessorValue
MFEMBoundaryNetFluxPostprocessor::getValue() const
{
  return _total_flux;
}

#endif
