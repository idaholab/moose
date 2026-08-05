//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPMLStretchVector.h"

MFEMPMLStretchVector::MFEMPMLStretchVector(mfem::ParMesh & mesh,
                                           const mfem::Array<int> & pml_attributes,
                                           double decay_coefficient,
                                           double decay_polynomial,
                                           MPI_Comm comm)
  : mfem::VectorCoefficient(mesh.Dimension()),
    _dim(mesh.Dimension()),
    _decay_coefficient(decay_coefficient),
    _decay_polynomial(decay_polynomial),
    // W is a polynomial of degree decay_polynomial in the harmonic coordinate psi, so a space of at
    // least that degree interpolates it exactly where the layer lies between parallel surfaces.
    _h1_coll(std::max(2, static_cast<int>(std::ceil(decay_polynomial))), _dim),
    _h1_scalar_fes(&mesh, &_h1_coll),
    _h1_vector_fes(&mesh, &_h1_coll, _dim),
    _psi(&_h1_scalar_fes),
    _W_h1(&_h1_vector_fes)
{
  solveHarmonicCoordinate(mesh, pml_attributes, comm);
  checkFoliation(mesh, pml_attributes, comm);
  _W_h1.ProjectDiscCoefficient(*this, mfem::GridFunction::ARITHMETIC);
}

void
MFEMPMLStretchVector::Eval(mfem::Vector & W,
                           mfem::ElementTransformation & transformation,
                           const mfem::IntegrationPoint & integration_point)
{
  transformation.SetIntPoint(&integration_point);
  const double depth = _psi.GetValue(transformation, integration_point);
  _psi.GetGradient(transformation, W);
  const double slope = W.Norml2();

  // Outside the layer the harmonic coordinate is flat, so it carries no direction to stretch along.
  if (depth <= 0.0 || slope == 0.0)
    W = 0.0;
  else
    W *= _decay_coefficient * std::pow(depth, _decay_polynomial) / slope;
}

void
MFEMPMLStretchVector::solveHarmonicCoordinate(mfem::ParMesh & mesh,
                                              const mfem::Array<int> & pml_attributes,
                                              MPI_Comm comm)
{
  auto in_pml = [&pml_attributes](int attribute) { return pml_attributes.Find(attribute) != -1; };

  int n_boundary_attributes = mesh.bdr_attributes.Size() ? mesh.bdr_attributes.Max() : 0;
  MPI_Allreduce(MPI_IN_PLACE, &n_boundary_attributes, 1, MPI_INT, MPI_MAX, comm);

  // A boundary attribute that also borders the rest of the domain runs alongside the layer rather
  // than capping it, and so carries the natural no flux condition instead of being held at one.
  mfem::Array<int> lateral(n_boundary_attributes);
  lateral = 0;
  for (const auto b : make_range(mesh.GetNBE()))
  {
    int element = -1, info = 0;
    mesh.GetBdrElementAdjacentElement(b, element, info);
    if (!in_pml(mesh.GetAttribute(element)))
      lateral[mesh.GetBdrAttribute(b) - 1] = 1;
  }
  MPI_Allreduce(MPI_IN_PLACE, lateral.GetData(), n_boundary_attributes, MPI_INT, MPI_MAX, comm);

  mfem::Array<int> interior_marker(_h1_scalar_fes.GetVSize());
  mfem::Array<int> outer_marker(_h1_scalar_fes.GetVSize());
  interior_marker = 0;
  outer_marker = 0;

  mfem::Array<int> dofs;
  for (const auto e : make_range(mesh.GetNE()))
    if (!in_pml(mesh.GetAttribute(e)))
    {
      _h1_scalar_fes.GetElementDofs(e, dofs);
      for (const auto d : dofs)
        interior_marker[d] = 1;
    }

  int n_outer_faces = 0;
  for (const auto b : make_range(mesh.GetNBE()))
  {
    int element = -1, info = 0;
    mesh.GetBdrElementAdjacentElement(b, element, info);
    if (!in_pml(mesh.GetAttribute(element)) || lateral[mesh.GetBdrAttribute(b) - 1])
      continue;

    ++n_outer_faces;
    _h1_scalar_fes.GetBdrElementDofs(b, dofs);
    for (const auto d : dofs)
      outer_marker[d] = 1;
  }
  MPI_Allreduce(MPI_IN_PLACE, &n_outer_faces, 1, MPI_INT, MPI_SUM, comm);

  if (n_outer_faces == 0)
    mooseError("MFEMPMLStretchVector: the perfectly matched layer has no outer surface. Check "
               "that the kernel is restricted to a layer of elements that borders the exterior of "
               "the mesh, and that the boundary capping the layer does not share its attribute "
               "with a boundary of the rest of the domain.");

  _h1_scalar_fes.Synchronize(interior_marker);
  _h1_scalar_fes.Synchronize(outer_marker);

  _psi = 0.0;
  mfem::Array<int> essential_marker(_h1_scalar_fes.GetVSize());
  for (const auto d : make_range(_h1_scalar_fes.GetVSize()))
  {
    essential_marker[d] = interior_marker[d] || outer_marker[d];
    if (outer_marker[d])
      _psi(d) = 1.0;
  }

  mfem::Array<int> essential_true_marker, essential_true_dofs;
  _h1_scalar_fes.GetRestrictionMatrix()->BooleanMult(essential_marker, essential_true_marker);
  mfem::FiniteElementSpace::MarkerToList(essential_true_marker, essential_true_dofs);

  // Every degree of freedom outside the layer is essential, so assembling over the whole mesh
  // leaves the same reduced system as assembling over the layer alone.
  mfem::ParBilinearForm laplacian(&_h1_scalar_fes);
  laplacian.AddDomainIntegrator(new mfem::DiffusionIntegrator);
  laplacian.Assemble();

  mfem::ParLinearForm rhs(&_h1_scalar_fes);
  rhs = 0.0;

  mfem::OperatorPtr system;
  mfem::Vector solution, load;
  laplacian.FormLinearSystem(essential_true_dofs, _psi, rhs, system, solution, load);

  mfem::HypreBoomerAMG preconditioner(*system.As<mfem::HypreParMatrix>());
  preconditioner.SetPrintLevel(0);

  mfem::CGSolver solver(comm);
  solver.SetRelTol(1e-12);
  solver.SetMaxIter(2000);
  solver.SetPrintLevel(0);
  solver.SetPreconditioner(preconditioner);
  solver.SetOperator(*system);
  solver.Mult(load, solution);

  if (!solver.GetConverged())
    mooseError("MFEMPMLStretchVector: the solve for the harmonic coordinate of the perfectly "
               "matched layer did not converge.");

  laplacian.RecoverFEMSolution(solution, rhs, _psi);
}

void
MFEMPMLStretchVector::checkFoliation(mfem::ParMesh & mesh,
                                     const mfem::Array<int> & pml_attributes,
                                     MPI_Comm comm) const
{
  double smallest = std::numeric_limits<double>::max(), largest = 0.0;
  mfem::Vector gradient(_dim);

  for (const auto e : make_range(mesh.GetNE()))
  {
    if (pml_attributes.Find(mesh.GetAttribute(e)) == -1)
      continue;

    auto & transformation = *_h1_scalar_fes.GetElementTransformation(e);
    transformation.SetIntPoint(&mfem::Geometries.GetCenter(transformation.GetGeometryType()));
    _psi.GetGradient(transformation, gradient);

    const double norm = gradient.Norml2();
    smallest = std::min(smallest, norm);
    largest = std::max(largest, norm);
  }

  MPI_Allreduce(MPI_IN_PLACE, &smallest, 1, MPI_DOUBLE, MPI_MIN, comm);
  MPI_Allreduce(MPI_IN_PLACE, &largest, 1, MPI_DOUBLE, MPI_MAX, comm);

  // The gradient scales with the reciprocal of the local thickness of the layer, so a ratio this
  // extreme is a stagnation point rather than a merely thick part of it.
  if (smallest <= 1e-6 * largest)
    mooseError("MFEMPMLStretchVector: the harmonic coordinate of the perfectly matched layer "
               "stagnates, so the direction of the coordinate stretch is undefined there. The "
               "inner and outer surfaces of the layer must be convex.");
}

#endif
