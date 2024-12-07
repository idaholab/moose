//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseFunctor.h"
#include "MathFVUtils.h"
#include "FVUtils.h"
#include "MooseMeshUtils.h"
#include "VectorComponentFunctor.h"
#include "ArrayComponentFunctor.h"
#include "libmesh/elem.h"

namespace Moose
{
namespace FV
{
/**
 * Compute a cell gradient using the method of Green-Gauss
 * @param elem_arg An element argument specifying the current element and whether to perform skew
 * corrections
 * @param state_arg A state argument that indicates what temporal / solution iteration data to use
 * when evaluating the provided functor
 * @param functor The functor that will provide information such as cell and face value evaluations
 * necessary to construct the cell gradient
 * @param two_term_boundary_expansion Whether to perform a two-term expansion to compute
 * extrapolated boundary face values. If this is true, then an implicit system has to be solved. If
 * false, then the cell center value will be used as the extrapolated boundary face value
 * @param mesh The mesh on which we are computing the gradient
 * @return The computed cell gradient
 */
template <typename T,
          typename Enable = typename std::enable_if<libMesh::ScalarTraits<T>::value>::type>
libMesh::VectorValue<T>
greenGaussGradient(const ElemArg & elem_arg,
                   const StateArg & state_arg,
                   const FunctorBase<T> & functor,
                   const bool two_term_boundary_expansion,
                   const MooseMesh & mesh,
                   const bool force_green_gauss = false)
{
  mooseAssert(elem_arg.elem, "This should be non-null");
  const auto coord_type = mesh.getCoordSystem(elem_arg.elem->subdomain_id());
  const auto rz_radial_coord = mesh.getAxisymmetricRadialCoord();

  const T elem_value = functor(elem_arg, state_arg);

  // We'll count the number of extrapolated boundary faces (ebfs)
  unsigned int num_ebfs = 0;

  // Gradient to be returned
  VectorValue<T> grad;

  if (!elem_arg.correct_skewness || force_green_gauss) // Do Green-Gauss
  {
    try
    {
      bool volume_set = false;
      Real volume = 0;

      // If we are performing a two term Taylor expansion for extrapolated boundary faces (faces on
      // boundaries that do not have associated Dirichlet conditions), then the element gradient
      // depends on the boundary face value and the boundary face value depends on the element
      // gradient, so we have a system of equations to solve. Here is the system:
      //
      // \nabla \phi_C - \frac{1}{V} \sum_{ebf} \phi_{ebf} \vec{S_f} =
      //   \frac{1}{V} \sum_{of} \phi_{of} \vec{S_f}                       eqn. 1
      //
      // \phi_{ebf} - \vec{d_{Cf}} \cdot \nabla \phi_C = \phi_C            eqn. 2
      //
      // where $C$ refers to the cell centroid, $ebf$ refers to an extrapolated boundary face, $of$
      // refers to "other faces", e.g. non-ebf faces, and $f$ is a general face. $d_{Cf}$ is the
      // vector drawn from the element centroid to the face centroid, and $\vec{S_f}$ is the surface
      // vector, e.g. the face area times the outward facing normal

      // ebf eqns: element gradient coefficients, e.g. eqn. 2, LHS term 2 coefficient
      std::vector<libMesh::VectorValue<Real>> ebf_grad_coeffs;
      // ebf eqns: rhs b values. These will actually correspond to the elem_value so we can use a
      // pointer and avoid copying. This is the RHS of eqn. 2
      std::vector<const T *> ebf_b;

      // elem grad eqns: ebf coefficients, e.g. eqn. 1, LHS term 2 coefficients
      std::vector<libMesh::VectorValue<Real>> grad_ebf_coeffs;
      // elem grad eqns: rhs b value, e.g. eqn. 1 RHS
      libMesh::VectorValue<T> grad_b = 0;

      auto action_functor = [&volume_set,
                             &volume,
                             &elem_value,
                             &elem_arg,
                             &num_ebfs,
                             &ebf_grad_coeffs,
                             &ebf_b,
                             &grad_ebf_coeffs,
                             &grad_b,
                             &state_arg,
                             &functor,
                             two_term_boundary_expansion,
                             coord_type,
                             rz_radial_coord](const Elem & libmesh_dbg_var(functor_elem),
                                              const Elem *,
                                              const FaceInfo * const fi,
                                              const Point & surface_vector,
                                              Real coord,
                                              const bool elem_has_info)
      {
        mooseAssert(fi, "We need a FaceInfo for this action_functor");
        mooseAssert(
            elem_arg.elem == &functor_elem,
            "Just a sanity check that the element being passed in is the one we passed out.");

        if (functor.isExtrapolatedBoundaryFace(*fi, elem_arg.elem, state_arg))
        {
          if (two_term_boundary_expansion)
          {
            num_ebfs += 1;

            // eqn. 2
            ebf_grad_coeffs.push_back(-1. * (elem_has_info
                                                 ? (fi->faceCentroid() - fi->elemCentroid())
                                                 : (fi->faceCentroid() - fi->neighborCentroid())));
            ebf_b.push_back(&elem_value);

            // eqn. 1
            grad_ebf_coeffs.push_back(-surface_vector);
          }
          else
            // We are doing a one-term expansion for the extrapolated boundary faces, in which case
            // we have no eqn. 2 and we have no second term in the LHS of eqn. 1. Instead we apply
            // the element centroid value as the face value (one-term expansion) in the RHS of eqn.
            // 1
            grad_b += surface_vector * elem_value;
        }
        else
          grad_b +=
              surface_vector * functor(Moose::FaceArg{fi,
                                                      Moose::FV::LimiterType::CentralDifference,
                                                      true,
                                                      elem_arg.correct_skewness,
                                                      elem_arg.elem,
                                                      nullptr},
                                       state_arg);

        if (!volume_set)
        {
          // We use the FaceInfo volumes because those values have been pre-computed and cached.
          // An explicit call to elem->volume() here would incur unnecessary expense
          if (elem_has_info)
          {
            MooseMeshUtils::coordTransformFactor(
                fi->elemCentroid(), coord, coord_type, rz_radial_coord);
            volume = fi->elemVolume() * coord;
          }
          else
          {
            MooseMeshUtils::coordTransformFactor(
                fi->neighborCentroid(), coord, coord_type, rz_radial_coord);
            volume = fi->neighborVolume() * coord;
          }

          volume_set = true;
        }
      };

      Moose::FV::loopOverElemFaceInfo(
          *elem_arg.elem, mesh, action_functor, coord_type, rz_radial_coord);

      mooseAssert(volume_set && volume > 0, "We should have set the volume");
      grad_b /= volume;

      if (coord_type == Moose::CoordinateSystemType::COORD_RZ)
      {
        mooseAssert(rz_radial_coord != libMesh::invalid_uint, "rz_radial_coord must be set");
        grad_b(rz_radial_coord) -= elem_value / elem_arg.elem->vertex_average()(rz_radial_coord);
      }

      mooseAssert(
          coord_type != Moose::CoordinateSystemType::COORD_RSPHERICAL,
          "We have not yet implemented the correct translation from gradient to divergence for "
          "spherical coordinates yet.");

      // test for simple case
      if (num_ebfs == 0)
        grad = grad_b;
      else
      {
        // We have to solve a system
        const unsigned int sys_dim = Moose::dim + num_ebfs;
        DenseVector<T> x(sys_dim), b(sys_dim);
        DenseMatrix<T> A(sys_dim, sys_dim);

        // Let's make i refer to Moose::dim indices, and j refer to num_ebfs indices

        // eqn. 1
        for (const auto i : make_range(Moose::dim))
        {
          // LHS term 1 coeffs
          A(i, i) = 1;

          // LHS term 2 coeffs
          for (const auto j : make_range(num_ebfs))
            A(i, Moose::dim + j) = grad_ebf_coeffs[j](i) / volume;

          // RHS
          b(i) = grad_b(i);
        }

        // eqn. 2
        for (const auto j : make_range(num_ebfs))
        {
          // LHS term 1 coeffs
          A(Moose::dim + j, Moose::dim + j) = 1;

          // LHS term 2 coeffs
          for (const auto i : make_range(unsigned(Moose::dim)))
            A(Moose::dim + j, i) = ebf_grad_coeffs[j](i);

          // RHS
          b(Moose::dim + j) = *ebf_b[j];
        }

        A.lu_solve(b, x);
        // libMesh is generous about what it considers nonsingular. Let's check a little more
        // strictly
        if (MooseUtils::absoluteFuzzyEqual(MetaPhysicL::raw_value(A(sys_dim - 1, sys_dim - 1)), 0))
          throw libMesh::LogicError("Matrix A is singular!");
        for (const auto i : make_range(Moose::dim))
          grad(i) = x(i);
      }
    }
    catch (libMesh::LogicError & e)
    {
      // Retry without two-term
      if (!two_term_boundary_expansion)
        mooseError(
            "I believe we should only get singular systems when two-term boundary expansion is "
            "being used. The error thrown during the computation of the gradient: ",
            e.what());

      grad = greenGaussGradient(elem_arg, state_arg, functor, false, mesh, true);
    }
  }
  else // Do Least-Squares
  {
    try
    {

      // The least squares method aims to find the gradient at the element centroid by minimizing
      // the difference between the estimated and actual value differences across neighboring cells.
      // The least squares formulation is:
      //
      // Minimize J = \sum_{n} [ w_n ((\nabla \phi_C \cdot \delta \vec{x}_n) - \delta \phi_n) ]^2
      //
      // where:
      // - \(\nabla \phi_C\) is the gradient at the element centroid C (unknown).
      // - \(\delta \vec{x}_n = \vec{x}_n - \vec{x}_C\) is the vector from the element centroid to
      // neighbor \(n\).
      // - \(\delta \phi_n = \phi_n - \phi_C\) is the difference in the scalar value between
      // neighbor \(n\) and element \(C\).
      // - w_n = 1/||\vec{x}_n|| is a vector of weights that is used to handle large aspect ratio
      // cells
      //
      // To handle extrapolated boundary faces (faces on boundaries without Dirichlet conditions),
      // we include additional unknowns and equations in the least squares system.
      // For ebfs, we may not know the value \(\phi_n\), so we include it as an unknown.
      // This results in an augmented system that simultaneously solves for the gradient and the ebf
      // values.

      // Get estimated number of faces in a cell
      const auto ptr_range = elem_arg.elem->neighbor_ptr_range();
      const size_t num_faces = std::distance(ptr_range.begin(), ptr_range.end());

      // Lists to store position differences and value differences for least squares computation
      std::vector<Point> delta_x_list; // List of position differences between neighbor centroids
                                       // and element centroid
      delta_x_list.reserve(num_faces);

      std::vector<T>
          delta_phi_list; // List of value differences between neighbor values and element value
      delta_phi_list.reserve(num_faces);

      // Variables to handle extrapolated boundary faces (ebfs) in the least squares method
      std::vector<Point> delta_x_ebf_list; // Position differences for ebfs
      delta_phi_list.reserve(num_faces);

      std::vector<VectorValue<Real>>
          ebf_grad_coeffs; // Coefficients for the gradient in ebf equations
      delta_phi_list.reserve(num_faces);

      std::vector<const T *> ebf_b; // RHS values for ebf equations (pointers to avoid copying)
      delta_phi_list.reserve(num_faces);

      unsigned int num_ebfs = 0; // Number of extrapolated boundary faces

      // Action functor to collect data from each face of the element
      auto action_functor = [&elem_value,
                             &elem_arg,
                             &num_ebfs,
                             &ebf_grad_coeffs,
                             &ebf_b,
                             &delta_x_list,
                             &delta_phi_list,
                             &delta_x_ebf_list,
                             &state_arg,
                             &functor,
                             two_term_boundary_expansion](const Elem & libmesh_dbg_var(loc_elem),
                                                          const Elem * loc_neigh,
                                                          const FaceInfo * const fi,
                                                          const Point & /*surface_vector*/,
                                                          Real /*coord*/,
                                                          const bool elem_has_info)
      {
        mooseAssert(fi, "We need a FaceInfo for this action_functor");
        mooseAssert(
            elem_arg.elem == &loc_elem,
            "Just a sanity check that the element being passed in is the one we passed out.");

        if (functor.isExtrapolatedBoundaryFace(*fi, elem_arg.elem, state_arg))
        {
          // Extrapolated Boundary Face (ebf)
          const Point delta_x = elem_has_info ? (fi->faceCentroid() - fi->elemCentroid())
                                              : (fi->faceCentroid() - fi->neighborCentroid());
          delta_x_list.push_back(delta_x);

          T delta_phi;

          if (two_term_boundary_expansion)
          {
            // Two-term expansion: include ebf values as unknowns in the augmented system
            num_ebfs += 1;

            // Coefficient for the gradient in the ebf equation
            ebf_grad_coeffs.push_back(-delta_x);
            // RHS value for the ebf equation
            ebf_b.push_back(&elem_value);

            delta_phi = -elem_value;
            delta_x_ebf_list.push_back(delta_x);
          }
          else
          {
            // One-term expansion: assume zero difference across the boundary (\delta \phi = 0)
            delta_phi = T(0);
          }
          delta_phi_list.push_back(delta_phi);
        }
        else
        {
          // Internal Face or Boundary Face with Dirichlet condition
          Point delta_x;
          T neighbor_value;
          if (functor.isInternalFace(*fi))
          {
            // Internal face with a neighboring element
            delta_x = loc_neigh->vertex_average() - elem_arg.elem->vertex_average();

            const ElemArg neighbor_arg = {loc_neigh, elem_arg.correct_skewness};
            neighbor_value = functor(neighbor_arg, state_arg);
          }
          else
          {
            // Boundary face with Dirichlet condition
            delta_x = elem_has_info ? (fi->faceCentroid() - fi->elemCentroid())
                                    : (fi->faceCentroid() - fi->neighborCentroid());
            neighbor_value = functor(Moose::FaceArg{fi,
                                                    Moose::FV::LimiterType::CentralDifference,
                                                    true,
                                                    elem_arg.correct_skewness,
                                                    elem_arg.elem,
                                                    nullptr},
                                     state_arg);
          }

          delta_x_list.push_back(delta_x);
          delta_phi_list.push_back(neighbor_value - elem_value);
        }
      };

      // Loop over element faces and apply the action_functor
      Moose::FV::loopOverElemFaceInfo(
          *elem_arg.elem, mesh, action_functor, coord_type, rz_radial_coord);

      // Compute Least Squares gradient
      const unsigned int n = delta_x_list.size();
      mooseAssert(n >= dim, "Not enough neighbors to perform least squares");

      DenseMatrix<T> ATA(dim, dim);
      DenseVector<T> ATb(dim);

      // Assemble the normal equations for the least squares method
      // ATA = \sum_n (\delta \vec{x}_n^T * \delta \vec{x}_n)
      // ATb = \sum_n (\delta \vec{x}_n^T * \delta \phi_n)
      ATA.zero();
      ATb.zero();

      for (unsigned int i = 0; i < n; ++i)
      {
        const Point & delta_x = delta_x_list[i];
        const T & delta_phi = delta_phi_list[i];

        for (unsigned int d1 = 0; d1 < dim; ++d1)
        {
          const Real dx_d1 = delta_x(d1);
          const Real dx_norm = delta_x.norm();
          const Real dx_d1_norm = dx_d1 / dx_norm;

          for (unsigned int d2 = 0; d2 < dim; ++d2)
          {
            const Real dx_d2_norm = delta_x(d2) / dx_norm;
            ATA(d1, d2) += dx_d1_norm * dx_d2_norm;
          }

          ATb(d1) += dx_d1_norm * delta_phi / dx_norm;
        }
      }

      // Add regularization to ATA to prevent singularity
      T epsilon = 1e-12; // Small regularization parameter
      for (unsigned int d = 0; d < dim; ++d)
      {
        ATA(d, d) += epsilon;
      }

      if (num_ebfs == 0)
      {
        // Solve and store the least squares gradient
        DenseVector<T> grad_ls(dim);
        ATA.lu_solve(ATb, grad_ls);

        // Store the least squares gradient
        for (unsigned int d = 0; d < dim; ++d)
        {
          grad(d) = grad_ls(d);
        }
      }
      else
      {
        // We have to solve a system of equations
        const unsigned int sys_dim = Moose::dim + num_ebfs;
        DenseVector<T> x(sys_dim), b(sys_dim);
        DenseMatrix<T> A(sys_dim, sys_dim);

        // Let's make i refer to Moose::dim indices, and j refer to num_ebfs indices

        // eqn. 1: Element gradient equations
        for (unsigned int d1 = 0; d1 < dim; ++d1)
        {
          // LHS term 1 coefficients (gradient components)
          for (unsigned int d2 = 0; d2 < dim; ++d2)
            A(d1, d2) = ATA(d1, d2);

          // RHS
          b(d1) = ATb(d1);
        }

        // LHS term 2 coefficients (ebf contributions)
        for (const auto i : make_range(num_ebfs))
        {
          const Point & delta_x = delta_x_ebf_list[i];
          for (unsigned int d1 = 0; d1 < dim; ++d1)
          {
            const Real dx_d1 = delta_x(d1);
            A(d1, Moose::dim + i) -= dx_d1 / delta_x.norm() / delta_x.norm();
          }
        }

        // eqn. 2: Extrapolated boundary face equations
        for (const auto j : make_range(num_ebfs))
        {
          // LHS term 1 coefficients (ebf values)
          A(Moose::dim + j, Moose::dim + j) = 1;

          // LHS term 2 coefficients (gradient components)
          for (const auto i : make_range(unsigned(Moose::dim)))
            A(Moose::dim + j, i) = ebf_grad_coeffs[j](i);

          // RHS
          b(Moose::dim + j) = *ebf_b[j];
        }

        // Solve the system
        A.lu_solve(b, x);

        // Check for singularity
        if (MooseUtils::absoluteFuzzyEqual(MetaPhysicL::raw_value(A(sys_dim - 1, sys_dim - 1)), 0))
          throw libMesh::LogicError("Matrix A is singular!");

        // Extract the gradient components
        for (const auto i : make_range(Moose::dim))
          grad(i) = x(i);
      }

      mooseAssert(
          coord_type != Moose::CoordinateSystemType::COORD_RSPHERICAL,
          "We have not yet implemented the correct translation from gradient to divergence for "
          "spherical coordinates yet.");
    }
    catch (libMesh::LogicError & e)
    {
      // Log warning and default to simple green Gauss
      mooseWarning(
          "Singular matrix encountered in least squares gradient computation. Falling back "
          "to Green-Gauss gradient.");

      grad = greenGaussGradient(
          elem_arg, state_arg, functor, false, mesh, /* force_green_gauss */ true);
    }
  }

  return grad;

  // Notes to future developer:
  // Note 1:
  // For the least squares gradient, the LS matrix could be precomputed and stored for every cell
  // I tried doing this on October 2024, but the element lookup for these matrices is too slow
  // and seems better to compute weights on the fly.
  // Consider building a map from elem_id to these matrices and speed up lookup with octree
  // Note 2:
  // In the future one would like to have a hybrid gradient scheme, where:
  // \nabla \phi = \beta (\nabla \phi)_{LS} + (1 - \beta) (\nabla \phi)_{GG}
  // Then optize \beta based on mesh heuristics
}

/**
 * Compute a face gradient from Green-Gauss cell gradients, with orthogonality correction
 * On the boundaries, the boundary element value is used
 * @param face_arg A face argument specifying the current faceand whether to perform skew
 * corrections
 * @param state_arg A state argument that indicates what temporal / solution iteration data to use
 * when evaluating the provided functor
 * @param functor The functor that will provide information such as cell and face value evaluations
 * necessary to construct the cell gradient
 * @param two_term_boundary_expansion Whether to perform a two-term expansion to compute
 * extrapolated boundary face values. If this is true, then an implicit system has to be solved. If
 * false, then the cell center value will be used as the extrapolated boundary face value
 * @param mesh The mesh on which we are computing the gradient
 * @return The computed cell gradient
 */
template <typename T,
          typename Enable = typename std::enable_if<libMesh::ScalarTraits<T>::value>::type>
libMesh::VectorValue<T>
greenGaussGradient(const FaceArg & face_arg,
                   const StateArg & state_arg,
                   const FunctorBase<T> & functor,
                   const bool two_term_boundary_expansion,
                   const MooseMesh & mesh)
{
  mooseAssert(face_arg.fi, "We should have a face info to compute a face gradient");
  const auto & fi = *(face_arg.fi);
  const auto & elem_arg = face_arg.makeElem();
  const auto & neighbor_arg = face_arg.makeNeighbor();
  const bool defined_on_elem = functor.hasBlocks(fi.elemSubdomainID());
  const bool defined_on_neighbor = fi.neighborPtr() && functor.hasBlocks(fi.neighborSubdomainID());

  if (defined_on_elem && defined_on_neighbor)
  {
    const auto & value_elem = functor(elem_arg, state_arg);
    const auto & value_neighbor = functor(neighbor_arg, state_arg);

    // This is the component of the gradient which is parallel to the line connecting
    // the cell centers. Therefore, we can use our second order, central difference
    // scheme to approximate it.
    libMesh::VectorValue<T> face_gradient = (value_neighbor - value_elem) / fi.dCNMag() * fi.eCN();

    // We only need nonorthogonal correctors in 2+ dimensions
    if (mesh.dimension() > 1)
    {
      // We are using an orthogonal approach for the non-orthogonal correction, for more information
      // see Hrvoje Jasak's PhD Thesis (Imperial College, 1996)
      libMesh::VectorValue<T> interpolated_gradient;

      // Compute the gradients in the two cells on both sides of the face
      const auto & grad_elem =
          greenGaussGradient(elem_arg, state_arg, functor, two_term_boundary_expansion, mesh);
      const auto & grad_neighbor =
          greenGaussGradient(neighbor_arg, state_arg, functor, two_term_boundary_expansion, mesh);

      Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                             interpolated_gradient,
                             grad_elem,
                             grad_neighbor,
                             fi,
                             true);

      face_gradient += interpolated_gradient - (interpolated_gradient * fi.eCN()) * fi.eCN();
    }

    return face_gradient;
  }
  else if (defined_on_elem)
    return functor.gradient(elem_arg, state_arg);
  else if (defined_on_neighbor)
    return functor.gradient(neighbor_arg, state_arg);
  else
    mooseError("The functor must be defined on one of the sides");
}

template <typename T>
TensorValue<T>
greenGaussGradient(const ElemArg & elem_arg,
                   const StateArg & state_arg,
                   const Moose::FunctorBase<libMesh::VectorValue<T>> & functor,
                   const bool two_term_boundary_expansion,
                   const MooseMesh & mesh)
{
  TensorValue<T> ret;
  for (const auto i : make_range(Moose::dim))
  {
    VectorComponentFunctor<T> scalar_functor(functor, i);
    const auto row_gradient =
        greenGaussGradient(elem_arg, state_arg, scalar_functor, two_term_boundary_expansion, mesh);
    for (const auto j : make_range(unsigned(Moose::dim)))
      ret(i, j) = row_gradient(j);
  }

  return ret;
}

template <typename T>
TensorValue<T>
greenGaussGradient(const FaceArg & face_arg,
                   const StateArg & state_arg,
                   const Moose::FunctorBase<libMesh::VectorValue<T>> & functor,
                   const bool two_term_boundary_expansion,
                   const MooseMesh & mesh)
{
  TensorValue<T> ret;
  for (const auto i : make_range(unsigned(Moose::dim)))
  {
    VectorComponentFunctor<T> scalar_functor(functor, i);
    const auto row_gradient =
        greenGaussGradient(face_arg, state_arg, scalar_functor, two_term_boundary_expansion, mesh);
    for (const auto j : make_range(unsigned(Moose::dim)))
      ret(i, j) = row_gradient(j);
  }

  return ret;
}

template <typename T>
typename Moose::FunctorBase<std::vector<T>>::GradientType
greenGaussGradient(const ElemArg & elem_arg,
                   const StateArg & state_arg,
                   const Moose::FunctorBase<std::vector<T>> & functor,
                   const bool two_term_boundary_expansion,
                   const MooseMesh & mesh)
{
  // Determine the size of the container
  const auto vals = functor(elem_arg, state_arg);
  typedef typename Moose::FunctorBase<std::vector<T>>::GradientType GradientType;
  GradientType ret(vals.size());
  for (const auto i : index_range(ret))
  {
    // Note that this can be very inefficient. Within the scalar greenGaussGradient routine we're
    // going to do value type evaluations of the array functor from scalar_functor and we will be
    // discarding all the value type evaluations other than the one corresponding to i
    ArrayComponentFunctor<T, FunctorBase<std::vector<T>>> scalar_functor(functor, i);
    ret[i] =
        greenGaussGradient(elem_arg, state_arg, scalar_functor, two_term_boundary_expansion, mesh);
  }

  return ret;
}

template <typename T>
typename Moose::FunctorBase<std::vector<T>>::GradientType
greenGaussGradient(const FaceArg & face_arg,
                   const StateArg & state_arg,
                   const Moose::FunctorBase<std::vector<T>> & functor,
                   const bool two_term_boundary_expansion,
                   const MooseMesh & mesh)
{
  // Determine the size of the container
  const auto vals = functor(face_arg, state_arg);
  typedef typename Moose::FunctorBase<std::vector<T>>::GradientType GradientType;
  GradientType ret(vals.size());
  for (const auto i : index_range(ret))
  {
    // Note that this can be very inefficient. Within the scalar greenGaussGradient routine we're
    // going to do value type evaluations of the array functor from scalar_functor and we will be
    // discarding all the value type evaluations other than the one corresponding to i
    ArrayComponentFunctor<T, FunctorBase<std::vector<T>>> scalar_functor(functor, i);
    ret[i] =
        greenGaussGradient(face_arg, state_arg, scalar_functor, two_term_boundary_expansion, mesh);
  }

  return ret;
}

template <typename T, std::size_t N>
typename Moose::FunctorBase<std::array<T, N>>::GradientType
greenGaussGradient(const ElemArg & elem_arg,
                   const StateArg & state_arg,
                   const Moose::FunctorBase<std::array<T, N>> & functor,
                   const bool two_term_boundary_expansion,
                   const MooseMesh & mesh)
{
  typedef typename Moose::FunctorBase<std::array<T, N>>::GradientType GradientType;
  GradientType ret;
  for (const auto i : make_range(N))
  {
    // Note that this can be very inefficient. Within the scalar greenGaussGradient routine we're
    // going to do value type evaluations of the array functor from scalar_functor and we will be
    // discarding all the value type evaluations other than the one corresponding to i
    ArrayComponentFunctor<T, FunctorBase<std::array<T, N>>> scalar_functor(functor, i);
    ret[i] =
        greenGaussGradient(elem_arg, state_arg, scalar_functor, two_term_boundary_expansion, mesh);
  }

  return ret;
}

template <typename T, std::size_t N>
typename Moose::FunctorBase<std::array<T, N>>::GradientType
greenGaussGradient(const FaceArg & face_arg,
                   const StateArg & state_arg,
                   const Moose::FunctorBase<std::array<T, N>> & functor,
                   const bool two_term_boundary_expansion,
                   const MooseMesh & mesh)
{
  typedef typename Moose::FunctorBase<std::array<T, N>>::GradientType GradientType;
  GradientType ret;
  for (const auto i : make_range(N))
  {
    // Note that this can be very inefficient. Within the scalar greenGaussGradient routine we're
    // going to do value type evaluations of the array functor from scalar_functor and we will be
    // discarding all the value type evaluations other than the one corresponding to i
    ArrayComponentFunctor<T, FunctorBase<std::array<T, N>>> scalar_functor(functor, i);
    ret[i] =
        greenGaussGradient(face_arg, state_arg, scalar_functor, two_term_boundary_expansion, mesh);
  }

  return ret;
}
}
}
