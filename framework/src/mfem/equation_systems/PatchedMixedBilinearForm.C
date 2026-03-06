//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "PatchedMixedBilinearForm.h"

namespace Moose::MFEM
{

void
ParMixedBilinearForm::Assemble(int skip_zeros)
{
  if (interior_face_integs.Size())
  {
    trial_pfes->ExchangeFaceNbrData();
    test_pfes->ExchangeFaceNbrData();
    if (!ext && mat == NULL)
    {
      pAllocMat();
    }
  }

  if (mfem::ParSubMesh::IsParSubMesh(trial_pfes->GetParMesh()))
    SubMeshTolerantAssemble(skip_zeros);
  else
    MixedBilinearForm::Assemble(skip_zeros);

  if (!ext && interior_face_integs.Size() > 0)
  {
    AssembleSharedFaces(skip_zeros);
  }
}

void
ParMixedBilinearForm::SubMeshTolerantAssemble(int skip_zeros)
{
  if (ext)
  {
    ext->Assemble();
    return;
  }

  mfem::ElementTransformation * eltrans;
  mfem::DenseMatrix elmat;

  // For usual mfem::MixedBilinearFormAssemble, the iterated FESpace is test_fes
  mfem::FiniteElementSpace & iterated_fes = *trial_fes;
  mfem::Mesh * mesh = iterated_fes.GetMesh();
  const int num_elem = iterated_fes.GetNE();
  const int num_boundary_elem = iterated_fes.GetNBE();

  if (mat == NULL)
  {
    mat = new mfem::SparseMatrix(height, width);
  }

  if (domain_integs.Size())
  {
    for (int k = 0; k < domain_integs.Size(); k++)
    {
      if (domain_integs_marker[k] != NULL)
      {
        MFEM_VERIFY(domain_integs_marker[k]->Size() ==
                        (mesh->attributes.Size() ? mesh->attributes.Max() : 0),
                    "invalid element marker for domain integrator #" << k
                                                                     << ", counting from zero");
      }
    }

    mfem::DofTransformation dom_dof_trans, ran_dof_trans;
    for (int i = 0; i < num_elem; i++)
    {
      const int elem_attr = mesh->GetAttribute(i);
      trial_fes->GetElementVDofs(i, trial_vdofs, dom_dof_trans);
      test_fes->GetElementVDofs(i, test_vdofs, ran_dof_trans);
      eltrans = iterated_fes.GetElementTransformation(i);

      elmat.SetSize(test_vdofs.Size(), trial_vdofs.Size());
      elmat = 0.0;
      for (int k = 0; k < domain_integs.Size(); k++)
      {
        if (domain_integs_marker[k] == NULL || (*(domain_integs_marker[k]))[elem_attr - 1] == 1)
        {
          domain_integs[k]->AssembleElementMatrix2(
              *trial_fes->GetFE(i), *test_fes->GetFE(i), *eltrans, elemmat);
          elmat += elemmat;
        }
      }
      TransformDual(ran_dof_trans, dom_dof_trans, elmat);
      mat->AddSubMatrix(test_vdofs, trial_vdofs, elmat, skip_zeros);
    }
  }

  if (boundary_integs.Size())
  {
    // Which boundary attributes need to be processed?
    mfem::Array<int> bdr_attr_marker(mesh->bdr_attributes.Size() ? mesh->bdr_attributes.Max() : 0);
    bdr_attr_marker = 0;
    for (int k = 0; k < boundary_integs.Size(); k++)
    {
      if (boundary_integs_marker[k] == NULL)
      {
        bdr_attr_marker = 1;
        break;
      }
      mfem::Array<int> & bdr_marker = *boundary_integs_marker[k];
      MFEM_ASSERT(bdr_marker.Size() == bdr_attr_marker.Size(),
                  "invalid boundary marker for boundary integrator #" << k
                                                                      << ", counting from zero");
      for (int i = 0; i < bdr_attr_marker.Size(); i++)
      {
        bdr_attr_marker[i] |= bdr_marker[i];
      }
    }

    mfem::DofTransformation dom_dof_trans, ran_dof_trans;
    for (int i = 0; i < num_boundary_elem; i++)
    {
      const int bdr_attr = mesh->GetBdrAttribute(i);
      if (bdr_attr_marker[bdr_attr - 1] == 0)
      {
        continue;
      }

      trial_fes->GetBdrElementVDofs(i, trial_vdofs, dom_dof_trans);
      test_fes->GetBdrElementVDofs(i, test_vdofs, ran_dof_trans);
      eltrans = iterated_fes.GetBdrElementTransformation(i);

      elmat.SetSize(test_vdofs.Size(), trial_vdofs.Size());
      elmat = 0.0;
      for (int k = 0; k < boundary_integs.Size(); k++)
      {
        if (boundary_integs_marker[k] && (*boundary_integs_marker[k])[bdr_attr - 1] == 0)
        {
          continue;
        }

        boundary_integs[k]->AssembleElementMatrix2(
            *trial_fes->GetBE(i), *test_fes->GetBE(i), *eltrans, elemmat);
        elmat += elemmat;
      }
      TransformDual(ran_dof_trans, dom_dof_trans, elmat);
      mat->AddSubMatrix(test_vdofs, trial_vdofs, elmat, skip_zeros);
    }
  }

  if (interior_face_integs.Size())
  {
    mfem::FaceElementTransformations * ftr;
    mfem::Array<int> trial_vdofs2, test_vdofs2;
    const mfem::FiniteElement *trial_fe1, *trial_fe2, *test_fe1, *test_fe2;

    int nfaces = mesh->GetNumFaces();
    for (int i = 0; i < nfaces; i++)
    {
      ftr = mesh->GetInteriorFaceTransformations(i);
      if (ftr != NULL)
      {
        trial_fes->GetElementVDofs(ftr->Elem1No, trial_vdofs);
        test_fes->GetElementVDofs(ftr->Elem1No, test_vdofs);
        trial_fe1 = trial_fes->GetFE(ftr->Elem1No);
        test_fe1 = test_fes->GetFE(ftr->Elem1No);
        if (ftr->Elem2No >= 0)
        {
          trial_fes->GetElementVDofs(ftr->Elem2No, trial_vdofs2);
          test_fes->GetElementVDofs(ftr->Elem2No, test_vdofs2);
          trial_vdofs.Append(trial_vdofs2);
          test_vdofs.Append(test_vdofs2);
          trial_fe2 = trial_fes->GetFE(ftr->Elem2No);
          test_fe2 = test_fes->GetFE(ftr->Elem2No);
        }
        else
        {
          // The test_fe2 object is really a dummy and not used on the
          // boundaries, but we can't dereference a NULL pointer, and we don't
          // want to actually make a fake element.
          trial_fe2 = trial_fe1;
          test_fe2 = test_fe1;
        }
        for (int k = 0; k < interior_face_integs.Size(); k++)
        {
          interior_face_integs[k]->AssembleFaceMatrix(
              *trial_fe1, *test_fe1, *trial_fe2, *test_fe2, *ftr, elemmat);
          mat->AddSubMatrix(test_vdofs, trial_vdofs, elemmat, skip_zeros);
        }
      }
    }
  }

  if (boundary_face_integs.Size())
  {
    mfem::FaceElementTransformations * ftr;
    mfem::Array<int> tr_vdofs2, te_vdofs2;
    const mfem::FiniteElement *trial_fe1, *trial_fe2, *test_fe1, *test_fe2;

    // Which boundary attributes need to be processed?
    mfem::Array<int> bdr_attr_marker(mesh->bdr_attributes.Size() ? mesh->bdr_attributes.Max() : 0);
    bdr_attr_marker = 0;
    for (int k = 0; k < boundary_face_integs.Size(); k++)
    {
      if (boundary_face_integs_marker[k] == NULL)
      {
        bdr_attr_marker = 1;
        break;
      }
      mfem::Array<int> & bdr_marker = *boundary_face_integs_marker[k];
      MFEM_ASSERT(bdr_marker.Size() == bdr_attr_marker.Size(),
                  "invalid boundary marker for boundary face integrator #"
                      << k << ", counting from zero");
      for (int i = 0; i < bdr_attr_marker.Size(); i++)
      {
        bdr_attr_marker[i] |= bdr_marker[i];
      }
    }

    for (int i = 0; i < trial_fes->GetNBE(); i++)
    {
      const int bdr_attr = mesh->GetBdrAttribute(i);
      if (bdr_attr_marker[bdr_attr - 1] == 0)
      {
        continue;
      }

      ftr = mesh->GetBdrFaceTransformations(i);
      if (ftr != NULL)
      {
        trial_fes->GetElementVDofs(ftr->Elem1No, trial_vdofs);
        test_fes->GetElementVDofs(ftr->Elem1No, test_vdofs);
        trial_fe1 = trial_fes->GetFE(ftr->Elem1No);
        test_fe1 = test_fes->GetFE(ftr->Elem1No);
        // The test_fe2 object is really a dummy and not used on the
        // boundaries, but we can't dereference a NULL pointer, and we don't
        // want to actually make a fake element.
        trial_fe2 = trial_fe1;
        test_fe2 = test_fe1;
        for (int k = 0; k < boundary_face_integs.Size(); k++)
        {
          if (boundary_face_integs_marker[k] &&
              (*boundary_face_integs_marker[k])[bdr_attr - 1] == 0)
          {
            continue;
          }

          boundary_face_integs[k]->AssembleFaceMatrix(
              *trial_fe1, *test_fe1, *trial_fe2, *test_fe2, *ftr, elemmat);
          mat->AddSubMatrix(test_vdofs, trial_vdofs, elemmat, skip_zeros);
        }
      }
    }
  }

  if (trace_face_integs.Size())
  {
    mfem::FaceElementTransformations * ftr;
    mfem::Array<int> test_vdofs2;
    const mfem::FiniteElement *trial_face_fe, *test_fe1, *test_fe2;

    int nfaces = mesh->GetNumFaces();
    for (int i = 0; i < nfaces; i++)
    {
      ftr = mesh->GetFaceElementTransformations(i);
      trial_fes->GetFaceVDofs(i, trial_vdofs);
      test_fes->GetElementVDofs(ftr->Elem1No, test_vdofs);
      trial_face_fe = trial_fes->GetFaceElement(i);
      test_fe1 = test_fes->GetFE(ftr->Elem1No);
      if (ftr->Elem2No >= 0)
      {
        test_fes->GetElementVDofs(ftr->Elem2No, test_vdofs2);
        test_vdofs.Append(test_vdofs2);
        test_fe2 = test_fes->GetFE(ftr->Elem2No);
      }
      else
      {
        // The test_fe2 object is really a dummy and not used on the
        // boundaries, but we can't dereference a NULL pointer, and we don't
        // want to actually make a fake element.
        test_fe2 = test_fe1;
      }
      for (int k = 0; k < trace_face_integs.Size(); k++)
      {
        trace_face_integs[k]->AssembleFaceMatrix(
            *trial_face_fe, *test_fe1, *test_fe2, *ftr, elemmat);
        mat->AddSubMatrix(test_vdofs, trial_vdofs, elemmat, skip_zeros);
      }
    }
  }

  if (boundary_trace_face_integs.Size())
  {
    mfem::FaceElementTransformations * ftr;
    mfem::Array<int> te_vdofs2;
    const mfem::FiniteElement *trial_face_fe, *test_fe1, *test_fe2;

    // Which boundary attributes need to be processed?
    mfem::Array<int> bdr_attr_marker(mesh->bdr_attributes.Size() ? mesh->bdr_attributes.Max() : 0);
    bdr_attr_marker = 0;
    for (int k = 0; k < boundary_trace_face_integs.Size(); k++)
    {
      if (boundary_trace_face_integs_marker[k] == NULL)
      {
        bdr_attr_marker = 1;
        break;
      }
      mfem::Array<int> & bdr_marker = *boundary_trace_face_integs_marker[k];
      MFEM_ASSERT(bdr_marker.Size() == bdr_attr_marker.Size(),
                  "invalid boundary marker for boundary trace face"
                  "integrator #"
                      << k << ", counting from zero");
      for (int i = 0; i < bdr_attr_marker.Size(); i++)
      {
        bdr_attr_marker[i] |= bdr_marker[i];
      }
    }

    for (int i = 0; i < trial_fes->GetNBE(); i++)
    {
      const int bdr_attr = mesh->GetBdrAttribute(i);
      if (bdr_attr_marker[bdr_attr - 1] == 0)
      {
        continue;
      }

      ftr = mesh->GetBdrFaceTransformations(i);
      if (ftr)
      {
        const int iface = mesh->GetBdrElementFaceIndex(i);
        trial_fes->GetFaceVDofs(iface, trial_vdofs);
        test_fes->GetElementVDofs(ftr->Elem1No, test_vdofs);
        trial_face_fe = trial_fes->GetFaceElement(iface);
        test_fe1 = test_fes->GetFE(ftr->Elem1No);
        // The test_fe2 object is really a dummy and not used on the
        // boundaries, but we can't dereference a NULL pointer, and we don't
        // want to actually make a fake element.
        test_fe2 = test_fe1;
        for (int k = 0; k < boundary_trace_face_integs.Size(); k++)
        {
          if (boundary_trace_face_integs_marker[k] &&
              (*boundary_trace_face_integs_marker[k])[bdr_attr - 1] == 0)
          {
            continue;
          }

          boundary_trace_face_integs[k]->AssembleFaceMatrix(
              *trial_face_fe, *test_fe1, *test_fe2, *ftr, elemmat);
          mat->AddSubMatrix(test_vdofs, trial_vdofs, elemmat, skip_zeros);
        }
      }
    }
  }
}

} // namespace Moose::MFEM

#endif
