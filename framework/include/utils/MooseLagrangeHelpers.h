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

// Copy of libMesh function but templated to enable calling with DualNumber vectors
template <typename T, template <typename> class VectorType>
T
fe_lagrange_2D_shape(const ElemType type,
                     const Order order,
                     const unsigned int i,
                     const VectorType<T> & p)
{
  switch (order)
  {
    // linear Lagrange shape functions
    case FIRST:
    {
      switch (type)
      {
        case QUAD4:
        case QUADSHELL4:
        case QUAD8:
        case QUADSHELL8:
        case QUAD9:
        {
          // Compute quad shape functions as a tensor-product
          const T xi = p(0);
          const T eta = p(1);

          libmesh_assert_less(i, 4);

          //                                0  1  2  3
          static const unsigned int i0[] = {0, 1, 1, 0};
          static const unsigned int i1[] = {0, 0, 1, 1};

          return (fe_lagrange_1D_shape(FIRST, i0[i], xi) * fe_lagrange_1D_shape(FIRST, i1[i], eta));
        }

        case TRI3:
        case TRISHELL3:
        case TRI6:
        {
          const T zeta1 = p(0);
          const T zeta2 = p(1);
          const T zeta0 = 1. - zeta1 - zeta2;

          libmesh_assert_less(i, 3);

          switch (i)
          {
            case 0:
              return zeta0;

            case 1:
              return zeta1;

            case 2:
              return zeta2;

            default:
              mooseError("Invalid shape function index i = ", i);
          }
        }

        default:
          mooseError("Unsupported element type:", type);
      }
    }

    // quadratic Lagrange shape functions
    case SECOND:
    {
      switch (type)
      {
        case QUAD8:
        {
          // Compute quad shape functions as a tensor-product
          const T xi = p(0);
          const T eta = p(1);

          libmesh_assert_less(i, 8);

          switch (i)
          {
            case 0:
              return .25 * (1. - xi) * (1. - eta) * (-1. - xi - eta);
            case 1:
              return .25 * (1. + xi) * (1. - eta) * (-1. + xi - eta);
            case 2:
              return .25 * (1. + xi) * (eta + 1.) * (-1. + xi + eta);
            case 3:
              return .25 * (1. - xi) * (eta + 1.) * (-1. - xi + eta);
            case 4:
              return .5 * (1. - xi * xi) * (1. - eta);
            case 5:
              return .5 * (1. + xi) * (1. - eta * eta);
            case 6:
              return .5 * (1. - xi * xi) * (1. + eta);
            case 7:
              return .5 * (1. - xi) * (1. - eta * eta);
            default:
              mooseError("Invalid shape function index i = ", i);
          }
        }
        case QUAD9:
        {
          // Compute quad shape functions as a tensor-product
          const T xi = p(0);
          const T eta = p(1);

          libmesh_assert_less(i, 9);

          //                                0  1  2  3  4  5  6  7  8
          static const unsigned int i0[] = {0, 1, 1, 0, 2, 1, 2, 0, 2};
          static const unsigned int i1[] = {0, 0, 1, 1, 0, 2, 1, 2, 2};

          return (fe_lagrange_1D_shape(SECOND, i0[i], xi) *
                  fe_lagrange_1D_shape(SECOND, i1[i], eta));
        }
        case TRI6:
        {
          const T zeta1 = p(0);
          const T zeta2 = p(1);
          const T zeta0 = 1. - zeta1 - zeta2;

          libmesh_assert_less(i, 6);

          switch (i)
          {
            case 0:
              return 2. * zeta0 * (zeta0 - 0.5);

            case 1:
              return 2. * zeta1 * (zeta1 - 0.5);

            case 2:
              return 2. * zeta2 * (zeta2 - 0.5);

            case 3:
              return 4. * zeta0 * zeta1;

            case 4:
              return 4. * zeta1 * zeta2;

            case 5:
              return 4. * zeta2 * zeta0;

            default:
              mooseError("Invalid shape function index i = ", i);
          }
        }

        default:
          mooseError("Unsupported 2D element type");
      }
    }

      // unsupported order
    default:
      mooseError("Unsupported order");
  }
}

template <typename T, template <typename> class VectorType>
T
fe_lagrange_2D_shape_deriv(const ElemType type,
                           const Order order,
                           const unsigned int i,
                           const unsigned int j,
                           const VectorType<T> & p)
{
  libmesh_assert_less(j, 2);

  switch (order)
  {
      // linear Lagrange shape functions
    case FIRST:
    {
      switch (type)
      {
        case QUAD4:
        case QUADSHELL4:
        case QUAD8:
        case QUADSHELL8:
        case QUAD9:
        {
          // Compute quad shape functions as a tensor-product
          const T xi = p(0);
          const T eta = p(1);

          libmesh_assert_less(i, 4);

          //                                0  1  2  3
          static const unsigned int i0[] = {0, 1, 1, 0};
          static const unsigned int i1[] = {0, 0, 1, 1};

          switch (j)
          {
              // d()/dxi
            case 0:
              return (fe_lagrange_1D_shape_deriv(FIRST, i0[i], xi) *
                      fe_lagrange_1D_shape(FIRST, i1[i], eta));

              // d()/deta
            case 1:
              return (fe_lagrange_1D_shape(FIRST, i0[i], xi) *
                      fe_lagrange_1D_shape_deriv(FIRST, i1[i], eta));

            default:
              mooseError("Invalid derivative index j = ", j);
          }
        }

        case TRI3:
        case TRISHELL3:
        case TRI6:
        {
          libmesh_assert_less(i, 3);

          const T dzeta0dxi = -1.;
          const T dzeta1dxi = 1.;
          const T dzeta2dxi = 0.;

          const T dzeta0deta = -1.;
          const T dzeta1deta = 0.;
          const T dzeta2deta = 1.;

          switch (j)
          {
              // d()/dxi
            case 0:
            {
              switch (i)
              {
                case 0:
                  return dzeta0dxi;

                case 1:
                  return dzeta1dxi;

                case 2:
                  return dzeta2dxi;

                default:
                  mooseError("Invalid shape function index i = ", i);
              }
            }
              // d()/deta
            case 1:
            {
              switch (i)
              {
                case 0:
                  return dzeta0deta;

                case 1:
                  return dzeta1deta;

                case 2:
                  return dzeta2deta;

                default:
                  mooseError("Invalid shape function index i = ", i);
              }
            }
            default:
              mooseError("Invalid derivative index j = ", j);
          }
        }

        default:
          mooseError("Unsupported 2D element type");
      }
    }

      // quadratic Lagrange shape functions
    case SECOND:
    {
      switch (type)
      {
        case QUAD8:
        case QUADSHELL8:
        {
          const T xi = p(0);
          const T eta = p(1);

          libmesh_assert_less(i, 8);

          switch (j)
          {
              // d/dxi
            case 0:
              switch (i)
              {
                case 0:
                  return .25 * (1. - eta) * ((1. - xi) * (-1.) + (-1.) * (-1. - xi - eta));

                case 1:
                  return .25 * (1. - eta) * ((1. + xi) * (1.) + (1.) * (-1. + xi - eta));

                case 2:
                  return .25 * (1. + eta) * ((1. + xi) * (1.) + (1.) * (-1. + xi + eta));

                case 3:
                  return .25 * (1. + eta) * ((1. - xi) * (-1.) + (-1.) * (-1. - xi + eta));

                case 4:
                  return .5 * (-2. * xi) * (1. - eta);

                case 5:
                  return .5 * (1.) * (1. - eta * eta);

                case 6:
                  return .5 * (-2. * xi) * (1. + eta);

                case 7:
                  return .5 * (-1.) * (1. - eta * eta);

                default:
                  mooseError("Invalid shape function index i = ", i);
              }

              // d/deta
            case 1:
              switch (i)
              {
                case 0:
                  return .25 * (1. - xi) * ((1. - eta) * (-1.) + (-1.) * (-1. - xi - eta));

                case 1:
                  return .25 * (1. + xi) * ((1. - eta) * (-1.) + (-1.) * (-1. + xi - eta));

                case 2:
                  return .25 * (1. + xi) * ((1. + eta) * (1.) + (1.) * (-1. + xi + eta));

                case 3:
                  return .25 * (1. - xi) * ((1. + eta) * (1.) + (1.) * (-1. - xi + eta));

                case 4:
                  return .5 * (1. - xi * xi) * (-1.);

                case 5:
                  return .5 * (1. + xi) * (-2. * eta);

                case 6:
                  return .5 * (1. - xi * xi) * (1.);

                case 7:
                  return .5 * (1. - xi) * (-2. * eta);

                default:
                  mooseError("Invalid shape function index i = ", i);
              }

            default:
              mooseError("ERROR: Invalid derivative index j = ", j);
          }
        }

        case QUAD9:
        {
          // Compute quad shape functions as a tensor-product
          const T xi = p(0);
          const T eta = p(1);

          libmesh_assert_less(i, 9);

          //                                0  1  2  3  4  5  6  7  8
          static const unsigned int i0[] = {0, 1, 1, 0, 2, 1, 2, 0, 2};
          static const unsigned int i1[] = {0, 0, 1, 1, 0, 2, 1, 2, 2};

          switch (j)
          {
              // d()/dxi
            case 0:
              return (fe_lagrange_1D_shape_deriv(SECOND, i0[i], xi) *
                      fe_lagrange_1D_shape(SECOND, i1[i], eta));

              // d()/deta
            case 1:
              return (fe_lagrange_1D_shape(SECOND, i0[i], xi) *
                      fe_lagrange_1D_shape_deriv(SECOND, i1[i], eta));

            default:
              mooseError("Invalid derivative index j = ", j);
          }
        }

        case TRI6:
        {
          libmesh_assert_less(i, 6);

          const T zeta1 = p(0);
          const T zeta2 = p(1);
          const T zeta0 = 1. - zeta1 - zeta2;

          const T dzeta0dxi = -1.;
          const T dzeta1dxi = 1.;
          const T dzeta2dxi = 0.;

          const T dzeta0deta = -1.;
          const T dzeta1deta = 0.;
          const T dzeta2deta = 1.;

          switch (j)
          {
            case 0:
            {
              switch (i)
              {
                case 0:
                  return (4. * zeta0 - 1.) * dzeta0dxi;

                case 1:
                  return (4. * zeta1 - 1.) * dzeta1dxi;

                case 2:
                  return (4. * zeta2 - 1.) * dzeta2dxi;

                case 3:
                  return 4. * zeta1 * dzeta0dxi + 4. * zeta0 * dzeta1dxi;

                case 4:
                  return 4. * zeta2 * dzeta1dxi + 4. * zeta1 * dzeta2dxi;

                case 5:
                  return 4. * zeta2 * dzeta0dxi + 4 * zeta0 * dzeta2dxi;

                default:
                  mooseError("Invalid shape function index i = ", i);
              }
            }

            case 1:
            {
              switch (i)
              {
                case 0:
                  return (4. * zeta0 - 1.) * dzeta0deta;

                case 1:
                  return (4. * zeta1 - 1.) * dzeta1deta;

                case 2:
                  return (4. * zeta2 - 1.) * dzeta2deta;

                case 3:
                  return 4. * zeta1 * dzeta0deta + 4. * zeta0 * dzeta1deta;

                case 4:
                  return 4. * zeta2 * dzeta1deta + 4. * zeta1 * dzeta2deta;

                case 5:
                  return 4. * zeta2 * dzeta0deta + 4 * zeta0 * dzeta2deta;

                default:
                  mooseError("Invalid shape function index i = ", i);
              }
            }
            default:
              mooseError("ERROR: Invalid derivative index j = ", j);
          }
        }

        default:
          mooseError("ERROR: Unsupported 2D element type");
      }
    }

      // unsupported order
    default:
      mooseError("Unsupported order");
  }
}
}
