//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarUtils.h"
#include "MooseLagrangeHelpers.h"

#include "libmesh/enum_to_string.h"
#include "metaphysicl/dualnumberarray.h"
#include "Eigen/Dense"

typedef DualNumber<Real, NumberArray<2, Real>> Dual2;

namespace Moose
{
namespace Mortar
{
void
projectQPoints3d(const Elem * const msm_elem,
                 const Elem * const primal_elem,
                 const unsigned int sub_elem_index,
                 const QBase & qrule_msm,
                 std::vector<Point> & q_pts)
{
  auto && msm_order = msm_elem->default_order();
  auto && msm_type = msm_elem->type();

  // Get normal to linearized element, could store and query but computation is easy
  Point e1 = msm_elem->point(0) - msm_elem->point(1);
  Point e2 = msm_elem->point(2) - msm_elem->point(1);
  const Point normal = e2.cross(e1).unit();

  // Get sub-elem (for second order meshes, otherwise trivial)
  const auto sub_elem = msm_elem->get_extra_integer(sub_elem_index);
  const ElemType primal_type = primal_elem->type();

  auto get_sub_elem_node_indices = [primal_type, sub_elem]() -> std::vector<unsigned int>
  {
    switch (primal_type)
    {
      case TRI3:
        return {{0, 1, 2}};
      case QUAD4:
        return {{0, 1, 2, 3}};
      case TRI6:
        switch (sub_elem)
        {
          case 0:
            return {{0, 3, 5}};
          case 1:
            return {{3, 4, 5}};
          case 2:
            return {{3, 1, 4}};
          case 3:
            return {{5, 4, 2}};
          default:
            mooseError("get_sub_elem_indices: Invalid sub_elem: ", sub_elem);
        }
      case QUAD8:
        switch (sub_elem)
        {
          case 0:
            return {{0, 4, 7}};
          case 1:
            return {{4, 1, 5}};
          case 2:
            return {{5, 2, 6}};
          case 3:
            return {{7, 6, 3}};
          case 4:
            return {{4, 5, 6, 7}};
          default:
            mooseError("get_sub_elem_nodes: Invalid sub_elem: ", sub_elem);
        }
      case QUAD9:
        switch (sub_elem)
        {
          case 0:
            return {{0, 4, 8, 7}};
          case 1:
            return {{4, 1, 5, 8}};
          case 2:
            return {{8, 5, 2, 6}};
          case 3:
            return {{7, 8, 6, 3}};
          default:
            mooseError("get_sub_elem_indices: Invalid sub_elem: ", sub_elem);
        }
      default:
        mooseError("get_sub_elem_indices: Face element type: ",
                   libMesh::Utility::enum_to_string<ElemType>(primal_type),
                   " invalid for 3D mortar");
    }
  };

  // Transforms quadrature point from first order sub-elements (in case of second-order)
  // to primal element
  auto transform_qp = [primal_type, sub_elem](const Real nu, const Real xi)
  {
    switch (primal_type)
    {
      case TRI3:
        return Point(nu, xi, 0);
      case QUAD4:
        return Point(nu, xi, 0);
      case TRI6:
        switch (sub_elem)
        {
          case 0:
            return Point(0.5 * nu, 0.5 * xi, 0);
          case 1:
            return Point(0.5 * (1 - xi), 0.5 * (nu + xi), 0);
          case 2:
            return Point(0.5 * (1 + nu), 0.5 * xi, 0);
          case 3:
            return Point(0.5 * nu, 0.5 * (1 + xi), 0);
          default:
            mooseError("get_sub_elem_indices: Invalid sub_elem: ", sub_elem);
        }
      case QUAD8:
        switch (sub_elem)
        {
          case 0:
            return Point(nu - 1, xi - 1, 0);
          case 1:
            return Point(nu + xi, xi - 1, 0);
          case 2:
            return Point(1 - xi, nu + xi, 0);
          case 3:
            return Point(nu - 1, nu + xi, 0);
          case 4:
            return Point(0.5 * (nu - xi), 0.5 * (nu + xi), 0);
          default:
            mooseError("get_sub_elem_indices: Invalid sub_elem: ", sub_elem);
        }
      case QUAD9:
        switch (sub_elem)
        {
          case 0:
            return Point(0.5 * (nu - 1), 0.5 * (xi - 1), 0);
          case 1:
            return Point(0.5 * (nu + 1), 0.5 * (xi - 1), 0);
          case 2:
            return Point(0.5 * (nu + 1), 0.5 * (xi + 1), 0);
          case 3:
            return Point(0.5 * (nu - 1), 0.5 * (xi + 1), 0);
          default:
            mooseError("get_sub_elem_indices: Invalid sub_elem: ", sub_elem);
        }
      default:
        mooseError("transform_qp: Face element type: ",
                   libMesh::Utility::enum_to_string<ElemType>(primal_type),
                   " invalid for 3D mortar");
    }
  };

  auto sub_element_type = [primal_type, sub_elem]()
  {
    switch (primal_type)
    {
      case TRI3:
      case TRI6:
        return TRI3;
      case QUAD4:
      case QUAD9:
        return QUAD4;
      case QUAD8:
        switch (sub_elem)
        {
          case 0:
          case 1:
          case 2:
          case 3:
            return TRI3;
          case 4:
            return QUAD4;
          default:
            mooseError("sub_element_type: Invalid sub_elem: ", sub_elem);
        }
      default:
        mooseError("sub_element_type: Face element type: ",
                   libMesh::Utility::enum_to_string<ElemType>(primal_type),
                   " invalid for 3D mortar");
    }
  };

  // Get sub-elem node indices
  auto sub_elem_node_indices = get_sub_elem_node_indices();

  // Loop through quadrature points on msm_elem
  for (auto qp : make_range(qrule_msm.n_points()))
  {
    // Get physical point on msm_elem to project
    Point x0;
    for (auto n : make_range(msm_elem->n_nodes()))
      x0 += Moose::fe_lagrange_2D_shape(
                msm_type, msm_order, n, static_cast<const TypeVector<Real> &>(qrule_msm.qp(qp))) *
            msm_elem->point(n);

    // Use msm_elem quadrature point as initial guess
    // (will be correct for aligned meshes)
    Dual2 xi1{};
    xi1.value() = qrule_msm.qp(qp)(0);
    xi1.derivatives()[0] = 1.0;
    Dual2 xi2{};
    xi2.value() = qrule_msm.qp(qp)(1);
    xi2.derivatives()[1] = 1.0;
    VectorValue<Dual2> xi(xi1, xi2, 0);
    unsigned int current_iterate = 0, max_iterates = 10;

    // Project qp from mortar segments to first order sub-elements (elements in case of first order
    // geometry)
    do
    {
      VectorValue<Dual2> x1;
      for (auto n : make_range(sub_elem_node_indices.size()))
        x1 += Moose::fe_lagrange_2D_shape(sub_element_type(), FIRST, n, xi) *
              primal_elem->point(sub_elem_node_indices[n]);
      auto u = x1 - x0;

      VectorValue<Dual2> F(u(1) * normal(2) - u(2) * normal(1),
                           u(2) * normal(0) - u(0) * normal(2),
                           u(0) * normal(1) - u(1) * normal(0));

      Real projection_tolerance(1e-10);

      // Normalize tolerance with quantities involved in the projection.
      // Absolute projection tolerance is loosened for displacements larger than those on the order
      // of one. Tightening the tolerance for displacements of smaller orders causes this tolerance
      // to not be reached in a number of tests.
      if (!u.is_zero() && u.norm().value() > 1.0)
        projection_tolerance *= u.norm().value();

      if (MetaPhysicL::raw_value(F).norm() < projection_tolerance)
        break;

      RealEigenMatrix J(3, 2);
      J << F(0).derivatives()[0], F(0).derivatives()[1], F(1).derivatives()[0],
          F(1).derivatives()[1], F(2).derivatives()[0], F(2).derivatives()[1];
      RealEigenVector f(3);
      f << F(0).value(), F(1).value(), F(2).value();
      const RealEigenVector dxi = -J.colPivHouseholderQr().solve(f);

      xi(0) += dxi(0);
      xi(1) += dxi(1);
    } while (++current_iterate < max_iterates);

    if (current_iterate < max_iterates)
    {
      // Transfer quadrature point from sub-element to element and store
      q_pts.push_back(transform_qp(xi(0).value(), xi(1).value()));

      // The following checks if quadrature point falls in correct domain.
      // On small mortar segment elements with very distorted elements this can fail, instead of
      // erroring simply truncate quadrature point, these points typically have very small
      // contributions to integrals
      auto & qp_back = q_pts.back();
      if (primal_elem->type() == TRI3 || primal_elem->type() == TRI6)
      {
        if (qp_back(0) < -TOLERANCE || qp_back(1) < -TOLERANCE ||
            qp_back(0) + qp_back(1) > (1 + TOLERANCE))
          mooseException("Quadrature point: ", qp_back, " out of bounds, truncating.");
      }
      else if (primal_elem->type() == QUAD4 || primal_elem->type() == QUAD8 ||
               primal_elem->type() == QUAD9)
      {
        if (qp_back(0) < (-1 - TOLERANCE) || qp_back(0) > (1 + TOLERANCE) ||
            qp_back(1) < (-1 - TOLERANCE) || qp_back(1) > (1 + TOLERANCE))
          mooseException("Quadrature point: ", qp_back, " out of bounds, truncating");
      }
    }
    else
    {
      mooseError("Newton iteration for mortar quadrature mapping msm element: ",
                 msm_elem->id(),
                 " to elem: ",
                 primal_elem->id(),
                 " didn't converge. MSM element volume: ",
                 msm_elem->volume());
    }
  }
}
}
}
