#include "flux_monitor_aux.hpp"

namespace hephaestus
{
double
calcFlux(mfem::GridFunction * v_field, int face_attr, mfem::Coefficient & q)
{

  double flux = 0.0;
  double area = 0.0;

  mfem::FiniteElementSpace * fes = v_field->FESpace();
  mfem::Mesh * mesh = fes->GetMesh();

  mfem::Vector local_dofs, normal_vec;
  mfem::DenseMatrix dshape;
  mfem::Array<int> dof_ids;

  for (int i = 0; i < mesh->GetNBE(); i++)
  {

    if (mesh->GetBdrAttribute(i) != face_attr)
      continue;

    mfem::FaceElementTransformations * f_tr =
        mesh->GetFaceElementTransformations(mesh->GetBdrElementFaceIndex(i));
    if (f_tr == nullptr)
      continue;

    const mfem::FiniteElement & elem = *fes->GetFE(f_tr->Elem1No);
    f_tr->Attribute = mesh->GetAttribute(f_tr->Elem1No);
    const int int_order = 2 * elem.GetOrder() + 3;
    const mfem::IntegrationRule & ir = mfem::IntRules.Get(f_tr->FaceGeom, int_order);

    fes->GetElementDofs(f_tr->Elem1No, dof_ids);
    v_field->GetSubVector(dof_ids, local_dofs);
    const int space_dim = f_tr->Face->GetSpaceDim();
    normal_vec.SetSize(space_dim);
    dshape.SetSize(elem.GetDof(), space_dim);

    for (int j = 0; j < ir.GetNPoints(); j++)
    {

      const mfem::IntegrationPoint & ip = ir.IntPoint(j);
      mfem::IntegrationPoint eip;
      f_tr->Loc1.Transform(ip, eip);
      f_tr->Face->SetIntPoint(&ip);
      double face_weight = f_tr->Face->Weight();
      double val = 0.0;
      f_tr->Elem1->SetIntPoint(&eip);
      elem.CalcVShape(*f_tr->Elem1, dshape);
      mfem::CalcOrtho(f_tr->Face->Jacobian(), normal_vec);
      val += dshape.InnerProduct(normal_vec, local_dofs) / face_weight;

      // Measure the area of the boundary
      area += ip.weight * face_weight;

      // Integrate alpha * n.Grad(x) + beta * x
      flux += q.Eval(*f_tr, ip) * val * ip.weight * face_weight;
    }
  }

  double total_flux;
  MPI_Allreduce(&flux, &total_flux, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  return total_flux;
}

double
calcFlux(mfem::GridFunction * v_field, int face_attr)
{
  mfem::ConstantCoefficient one_coef(1.0);
  return calcFlux(v_field, face_attr, one_coef);
}

FluxMonitorAux::FluxMonitorAux(std::string var_name, int face_attr, std::string coef_name)
  : _var_name(std::move(var_name)), _coef_name(std::move(coef_name)), _face_attr(face_attr)
{
}

void
FluxMonitorAux::Init(const hephaestus::GridFunctions & gridfunctions,
                     hephaestus::Coefficients & coefficients)
{
  _gf = gridfunctions.Get(_var_name);
  if (coefficients._scalars.Has(_coef_name))
  {
    _coef = coefficients._scalars.Get(_coef_name);
  }
}

void
FluxMonitorAux::Solve(double t)
{
  double flux;
  if (_coef != nullptr)
  {
    flux = calcFlux(_gf, _face_attr, *_coef);
  }
  else
  {
    flux = calcFlux(_gf, _face_attr);
  }

  _times.Append(t);
  _fluxes.Append(flux);
}

} // namespace hephaestus
