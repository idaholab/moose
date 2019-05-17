//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"
#include "libmesh/fe_type.h"

namespace Moose
{
using libMesh::Order;

// Copy in libmesh's lagrange helper functions, but we template it
template <typename T>
T
fe_lagrange_1D_shape(const Order order, const unsigned int i, const T & xi)
{
  switch (order)
  {
      // Lagrange linears
    case FIRST:
    {
      libmesh_assert_less(i, 2);

      switch (i)
      {
        case 0:
          return .5 * (1. - xi);

        case 1:
          return .5 * (1. + xi);

        default:
          mooseError("Invalid shape function index i = ", i);
      }
    }

      // Lagrange quadratics
    case SECOND:
    {
      libmesh_assert_less(i, 3);

      switch (i)
      {
        case 0:
          return .5 * xi * (xi - 1.);

        case 1:
          return .5 * xi * (xi + 1);

        case 2:
          return (1. - xi * xi);

        default:
          mooseError("Invalid shape function index i = ", i);
      }
    }

    default:
      mooseError("Unsupported order");
  }
}

template <typename T>
T
fe_lagrange_1D_shape_deriv(const Order order, const unsigned int i, const T & xi)
{
  switch (order)
  {
      // Lagrange linear shape function derivatives
    case FIRST:
    {
      libmesh_assert_less(i, 2);

      switch (i)
      {
        case 0:
          return -.5;

        case 1:
          return .5;

        default:
          mooseError("Invalid shape function index i = ", i);
      }
    }

      // Lagrange quadratic shape function derivatives
    case SECOND:
    {
      libmesh_assert_less(i, 3);

      switch (i)
      {
        case 0:
          return xi - .5;

        case 1:
          return xi + .5;

        case 2:
          return -2. * xi;

        default:
          mooseError("Invalid shape function index i = ", i);
      }
    }

    default:
      mooseError("Unsupported order");
  }
}
}
