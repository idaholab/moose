//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVVelocityVariable.h"
#include "FaceInfo.h"
#include "ADReal.h"
#include "MathFVUtils.h"
#include "FVUtils.h"
#include "MooseError.h"
#include "SubProblem.h"
#include "INSFVNoSlipWallBC.h"
#include "INSFVAttributes.h"
#include "SystemBase.h"
#include "FVDirichletBCBase.h"
#include "Assembly.h"

#include "libmesh/elem.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"
#include "libmesh/dense_vector.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/libmesh_common.h"

#include <vector>
#include <utility>

namespace Moose
{
namespace FV
{
template <typename ActionFunctor>
void
loopOverElemFaceInfo(const Elem & elem,
                     const MooseMesh & mesh,
                     const SubProblem & subproblem,
                     ActionFunctor & act)
{
  const auto coord_type = subproblem.getCoordSystem(elem.subdomain_id());
  loopOverElemFaceInfo(elem,
                       mesh,
                       act,
                       coord_type,
                       coord_type == Moose::COORD_RZ ? subproblem.getAxisymmetricRadialCoord()
                                                     : libMesh::invalid_uint);
}
}
}

registerMooseObject("NavierStokesApp", INSFVVelocityVariable);

InputParameters
INSFVVelocityVariable::validParams()
{
  return INSFVVariable::validParams();
}

INSFVVelocityVariable::INSFVVelocityVariable(const InputParameters & params) : INSFVVariable(params)
{
}

#ifdef MOOSE_GLOBAL_AD_INDEXING
const VectorValue<ADReal> &
INSFVVelocityVariable::adGradSln(const Elem * const elem, bool correct_skewness) const
{
  VectorValue<ADReal> * value_pointer = &_temp_cell_gradient;

  // We ensure that no caching takes place when we compute skewness-corrected
  // quantities.
  if (_cache_cell_gradients && !correct_skewness)
  {
    auto it = _elem_to_grad.find(elem);

    if (it != _elem_to_grad.end())
      return it->second;
  }

  ADReal elem_value = getElemValue(elem);

  // We'll save off the extrapolated boundary faces (ebf) for later assignment to the cache (these
  // are the keys). The boolean in the pair will denote whether the ebf face is a fully developed
  // flow (e.g. fdf) face
  std::vector<std::pair<const FaceInfo *, bool>> ebf_faces;

  try
  {
    VectorValue<ADReal> & grad = *value_pointer;

    bool volume_set = false;
    Real volume = 0;

    // If we are performing a two term Taylor expansion for extrapolated boundary faces (faces on
    // boundaries that do not have associated Dirichlet conditions), then the element gradient
    // depends on the boundary face value and the boundary face value depends on the element
    // gradient, so we have a system of equations to solve. Here is the system:
    //
    // \nabla \phi_C - \frac{1}{V} \sum_{ebf} \phi_{ebf} \vec{S_f} =
    //   \frac{1}{V} \sum_{of} \phi_{of} \vec{S_f}                                            eqn. 1
    //
    // \phi_{ebf} - \vec{d_{Cf}} \cdot \nabla \phi_C = \phi_C                                 eqn. 2
    //
    // where $C$ refers to the cell centroid, $ebf$ refers to an extrapolated boundary face, $of$
    // refers to "other faces", e.g. non-ebf faces, and $f$ is a general face. $d_{Cf}$ is the
    // vector drawn from the element centroid to the face centroid, and $\vec{S_f}$ is the surface
    // vector, e.g. the face area times the outward facing normal
    //
    // NOTE: On fully developed flow boundaries, we modify our equation set slightly. In equation 2,
    // $\nabla \phi_C$ is replaced with $\nabla \phi_{ebf,fdf}$ where $fdf$ denotes fully developed
    // flow. Moreover, we introduce a third equation:
    //
    // \nabla \phi_{ebf,fdf} - \nabla \phi_C + (\nabla \phi_C \cdot \hat{n}) \hat{n} = 0      eqn. 3
    //
    // These modifications correspond to Moukalled's equations 15.140 and 15.141, but with
    // $\hat{e_b}$ replaced with $\hat{n}$ because we believe the equation as written doesn't
    // reflect the intent of the text, which is to guarantee a zero normal gradient in the direction
    // of the surface normal

    // ebf eqns: element gradient coefficients, e.g. eqn. 2, LHS term 2 coefficient. *Note* that
    // each element of the std::vector could correspond to a cell centroid gradient or to a face
    // gradient computed on a fully developed flow face
    std::vector<VectorValue<Real>> ebf_grad_coeffs;
    // ebf eqns: rhs b values. These will actually correspond to the elem_value so we can use a
    // pointer and avoid copying. This is the RHS of eqn. 2
    std::vector<const ADReal *> ebf_b;

    // elem grad eqns: ebf coefficients, e.g. eqn. 1, LHS term 2 coefficients
    std::vector<VectorValue<Real>> grad_ebf_coeffs;
    // elem grad eqns: rhs b value, e.g. eqn. 1 RHS
    VectorValue<ADReal> grad_b = 0;

    // eqn. 3 coefficients for cell centroid gradient, e.g. the coefficients that fall out of term 2
    // on the LHS of eqn. 3
    std::vector<TensorValue<Real>> fdf_grad_centroid_coeffs;

    const unsigned int lm_dim = LIBMESH_DIM;

    auto action_functor = [&volume_set,
                           &volume,
                           &elem_value,
#ifndef NDEBUG
                           &elem,
#endif
                           &ebf_faces,
                           &ebf_grad_coeffs,
                           &ebf_b,
                           &grad_ebf_coeffs,
                           &grad_b,
                           &fdf_grad_centroid_coeffs,
                           correct_skewness,
                           this](const Elem & functor_elem,
                                 const Elem * const neighbor,
                                 const FaceInfo * const fi,
                                 const Point & surface_vector,
                                 Real coord,
                                 const bool elem_has_info)
    {
      mooseAssert(fi, "We need a FaceInfo for this action_functor");
      mooseAssert(elem == &functor_elem,
                  "Just a sanity check that the element being passed in is the one we passed out.");

      if (isExtrapolatedBoundaryFace(*fi))
      {
        if (_two_term_boundary_expansion)
        {
          const bool fdf_face = isFullyDevelopedFlowFace(*fi);
          ebf_faces.push_back(std::make_pair(fi, fdf_face));

          // eqn. 2
          ebf_grad_coeffs.push_back(-1. * (elem_has_info
                                               ? (fi->faceCentroid() - fi->elemCentroid())
                                               : (fi->faceCentroid() - fi->neighborCentroid())));
          ebf_b.push_back(&elem_value);

          // eqn. 1
          grad_ebf_coeffs.push_back(-surface_vector);

          // eqn. 3
          if (fdf_face)
          {
            // Will be nice in C++17 we'll get a returned reference from this method
            fdf_grad_centroid_coeffs.emplace_back();
            auto & current_coeffs = fdf_grad_centroid_coeffs.back();
            const auto normal = surface_vector / (fi->faceArea() * coord);
            for (const auto i : make_range(lm_dim))
              for (const auto j : make_range(lm_dim))
              {
                auto & current_coeff = current_coeffs(i, j);
                current_coeff = normal(i) * normal(j);
                if (i == j)
                  current_coeff -= 1.;
              }
          }
        }
        else
          // We are doing a one-term expansion for the extrapolated boundary faces, in which case
          // we have no eqn. 2 and we have no second term in the LHS of eqn. 1. Instead we apply
          // the element centroid value as the face value (one-term expansion) in the RHS of eqn.
          // 1
          grad_b += surface_vector * elem_value;
      }
      else if (isInternalFace(*fi))
        grad_b += surface_vector * getInternalFaceValue(*fi, correct_skewness);
      else
      {
        mooseAssert(isDirichletBoundaryFace(*fi), "We've run out of face types");
        grad_b += surface_vector * getDirichletBoundaryFaceValue(*fi);
      }

      if (!volume_set)
      {
        // We use the FaceInfo volumes because those values have been pre-computed and cached.
        // An explicit call to elem->volume() here would incur unnecessary expense
        if (elem_has_info)
        {
          coordTransformFactor(
              this->_subproblem, functor_elem.subdomain_id(), fi->elemCentroid(), coord);
          volume = fi->elemVolume() * coord;
        }
        else
        {
          coordTransformFactor(
              this->_subproblem, neighbor->subdomain_id(), fi->neighborCentroid(), coord);
          volume = fi->neighborVolume() * coord;
        }

        volume_set = true;
      }
    };

    Moose::FV::loopOverElemFaceInfo(*elem, this->_mesh, this->_subproblem, action_functor);

    mooseAssert(volume_set && volume > 0, "We should have set the volume");
    grad_b /= volume;

    const auto coord_system = this->_subproblem.getCoordSystem(elem->subdomain_id());
    if (coord_system == Moose::CoordinateSystemType::COORD_RZ)
    {
      const auto r_coord = this->_subproblem.getAxisymmetricRadialCoord();
      grad_b(r_coord) -= elem_value / elem->vertex_average()(r_coord);
    }

    mooseAssert(
        coord_system != Moose::CoordinateSystemType::COORD_RSPHERICAL,
        "We have not yet implemented the correct translation from gradient to divergence for "
        "spherical coordinates yet.");

    mooseAssert(
        ebf_faces.size() < UINT_MAX,
        "You've created a mystical element that has more faces than can be held by unsigned "
        "int. I applaud you.");
    const auto num_ebfs = static_cast<unsigned int>(ebf_faces.size());

    // test for simple case
    if (num_ebfs == 0)
      grad = grad_b;
    else
    {
      // We have to solve a system
      const unsigned int sys_dim =
          lm_dim + num_ebfs + lm_dim * static_cast<unsigned int>(fdf_grad_centroid_coeffs.size());
      DenseVector<ADReal> x(sys_dim), b(sys_dim);
      DenseMatrix<ADReal> A(sys_dim, sys_dim);

      // eqn. 1
      for (const auto lm_dim_index : make_range(lm_dim))
      {
        // LHS term 1 coeffs
        A(lm_dim_index, lm_dim_index) = 1;

        // LHS term 2 coeffs
        for (const auto ebf_index : make_range(num_ebfs))
          A(lm_dim_index, lm_dim + ebf_index) = grad_ebf_coeffs[ebf_index](lm_dim_index) / volume;

        // RHS
        b(lm_dim_index) = grad_b(lm_dim_index);
      }

      unsigned int num_fdf_faces = 0;

      // eqn. 2
      for (const auto ebf_index : make_range(num_ebfs))
      {
        // LHS term 1 coeffs
        A(lm_dim + ebf_index, lm_dim + ebf_index) = 1;

        const bool fdf_face = ebf_faces[ebf_index].second;
        const unsigned int starting_j_index =
            fdf_face ? lm_dim + num_ebfs + num_fdf_faces * lm_dim : 0;

        num_fdf_faces += fdf_face;

        // LHS term 2 coeffs
        for (const auto lm_dim_index : make_range(lm_dim))
          A(lm_dim + ebf_index, starting_j_index + lm_dim_index) =
              ebf_grad_coeffs[ebf_index](lm_dim_index);

        // RHS
        b(lm_dim + ebf_index) = *ebf_b[ebf_index];
      }

      mooseAssert(num_fdf_faces == fdf_grad_centroid_coeffs.size(),
                  "Bad math in INSFVVelocityVariable::adGradlnSln(const Elem *). Please contact a "
                  "MOOSE developer");

      // eqn. 3
      for (const auto fdf_face_index : make_range(num_fdf_faces))
      {
        const auto starting_i_index = lm_dim + num_ebfs + fdf_face_index * lm_dim;

        for (const auto lm_dim_i_index : make_range(lm_dim))
        {
          auto i_index = starting_i_index + lm_dim_i_index;
          A(i_index, i_index) = 1;

          for (const auto lm_dim_j_index : make_range(lm_dim))
            // j_index = lm_dim_j_index
            A(i_index, lm_dim_j_index) =
                fdf_grad_centroid_coeffs[fdf_face_index](lm_dim_i_index, lm_dim_j_index);
        }
      }

      A.lu_solve(b, x);
      for (const auto lm_dim_index : make_range(lm_dim))
        grad(lm_dim_index) = x(lm_dim_index);

      // Cache the face value information
      for (const auto ebf_index : make_range(num_ebfs))
        _face_to_value.emplace(ebf_faces[ebf_index].first, x(lm_dim + ebf_index));

      if (_cache_face_gradients && !correct_skewness)
      {
        // Cache the extrapolated face gradient information
        auto it = ebf_faces.begin();
        for (const auto fdf_index : make_range(num_fdf_faces))
        {
          it = std::find_if(it,
                            ebf_faces.end(),
                            [](const std::pair<const FaceInfo *, bool> & in) { return in.second; });
          mooseAssert(it != ebf_faces.end(), "We should have found a fully developed flow face");

          const auto starting_index =
              static_cast<unsigned int>(lm_dim + num_ebfs + lm_dim * fdf_index);

          auto pr = _face_to_unc_grad.emplace(it->first, VectorValue<ADReal>());
          mooseAssert(pr.second, "We should have inserted a new face gradient");
          for (const auto lm_index : make_range(lm_dim))
            pr.first->second(lm_index) = x(starting_index + lm_index);

          // increment the iterator so we don't find the same element again
          ++it;
        }
      }
    }

    if (_cache_cell_gradients && !correct_skewness)
    {
      auto pr = _elem_to_grad.emplace(elem, std::move(grad));
      mooseAssert(pr.second, "Insertion should have just happened.");
      return pr.first->second;
    }
    else
      return grad;
  }
  catch (libMesh::LogicError &)
  {
    // Retry without two-term
    mooseAssert(_two_term_boundary_expansion,
                "I believe we should only get singular systems when two-term boundary expansion is "
                "being used");
    const_cast<INSFVVelocityVariable *>(this)->_two_term_boundary_expansion = false;
    const auto & grad = adGradSln(elem, correct_skewness);

    // We failed to compute the extrapolated boundary faces with two-term expansion and callers of
    // this method may be relying on those values (e.g. if the caller is
    // getExtrapolatedBoundaryFaceValue) so we populate them here with one-term expansion, e.g. we
    // set the boundary face values to the cell centroid value
    for (const auto & ebf_face_pr : ebf_faces)
      _face_to_value.emplace(ebf_face_pr.first, elem_value);

    // Two term boundary expansion should only fail at domain corners. We want to keep trying it at
    // other boundary locations
    const_cast<INSFVVelocityVariable *>(this)->_two_term_boundary_expansion = true;
    return grad;
  }
}
#endif
