//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalVoronoiTJVoidIC.h"
#include "PolycrystalHex.h"

registerMooseObject("PhaseFieldApp", PolycrystalVoronoiTJVoidIC);

InputParameters
PolycrystalVoronoiTJVoidIC::validParams()
{
  InputParameters params = PolycrystalVoronoiVoidIC::validParams();
  params.addClassDescription("Random distribution of smooth circles at triple juctions "
                             "of voronoi grains, with a given minimum spacing");
  return params;
}

PolycrystalVoronoiTJVoidIC::PolycrystalVoronoiTJVoidIC(const InputParameters & parameters)
  : PolycrystalVoronoiVoidIC(parameters), _dim(_mesh.dimension())
{
}

void
PolycrystalVoronoiTJVoidIC::initialSetup()
{
  PolycrystalVoronoiVoidIC::initialSetup();
}

void
PolycrystalVoronoiTJVoidIC::computeCircleCenters()
{
  _centers.resize(_numbub);

  const bool _is_columnar_3D = _poly_ic_uo.isColumnar3D();

  for (unsigned int vp = 0; vp < _numbub; ++vp)
  {
    bool try_again;
    unsigned int num_tries = 0;
    do
    {
      try_again = false;
      num_tries++;

      if (num_tries > _max_num_tries)
        mooseError("Too many tries of assigning void centers in "
                   "PolycrystalVoronoiTJVoidIC");

      Point rand_point;
      for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
        rand_point(i) = _bottom_left(i) + _range(i) * _random.rand(_tid);

      unsigned int _pbc_grain_num;
      unsigned int num_cells = 0;

      unsigned int dim = _dim;
      unsigned int rank = _dim + 1;

      if (_is_columnar_3D)
      {
        dim -= 1;
        rank -= 1;
      }

      // Check if periodic BC is not applicable
      for (unsigned int op = 0; op < _op_num; ++op)
        for (unsigned int i = 0; i < dim; ++i)
          if (!_mesh.isTranslatedPeriodic(op, i))
            _pbc = false;

      // If periodic BC is applicable, generate grain centers in periodic domains
      if (_pbc == true)
      {
        Point translate;

        translate(0) = 0;
        translate(1) = 1;
        translate(2) = -1;

        if (_dim == 2 || _is_columnar_3D)
          num_cells = 9;
        else if (_dim == 3 && !_is_columnar_3D)
          num_cells = 27;

        _pbc_grain_num = num_cells * _grain_num;
        _pbc_centerpoints.resize(_pbc_grain_num);

        int cell = -1;
        for (unsigned int i = 0; i < 3; ++i)
          for (unsigned int j = 0; j < 3; ++j)
            if (_dim == 2 || _is_columnar_3D)
            {
              cell += 1;
              for (unsigned int gr = cell * _grain_num; gr < (cell + 1) * _grain_num; ++gr)
              {
                _pbc_centerpoints[gr](0) =
                    _centerpoints[gr - cell * _grain_num](0) + translate(i) * _top_right(0);
                _pbc_centerpoints[gr](1) =
                    _centerpoints[gr - cell * _grain_num](1) + translate(j) * _top_right(1);

                if (_is_columnar_3D == true)
                  _pbc_centerpoints[gr](2) = _centerpoints[gr - cell * _grain_num](2);
              }
            }
            else if (_dim == 3 && !_is_columnar_3D)
            {
              for (unsigned int k = 0; k < 3; ++k)
              {
                cell += 1;
                for (unsigned int gr = cell * _grain_num; gr < (cell + 1) * _grain_num; ++gr)
                {
                  _pbc_centerpoints[gr](0) =
                      _centerpoints[gr - cell * _grain_num](0) + translate(i) * _top_right(0);
                  _pbc_centerpoints[gr](1) =
                      _centerpoints[gr - cell * _grain_num](1) + translate(j) * _top_right(1);
                  _pbc_centerpoints[gr](2) =
                      _centerpoints[gr - cell * _grain_num](2) + translate(k) * _top_right(2);
                }
              }
            }
      }
      else // If not periodic, just use grain centers in current domain
      {
        num_cells = 1;

        _pbc_grain_num = num_cells * _grain_num;
        _pbc_centerpoints.resize(_pbc_grain_num);
        _pbc_centerpoints = _centerpoints;
      }

      // Search and sort nearest grain centers to a random point (i.e. void center point)
      std::vector<PolycrystalVoronoiVoidIC::DistancePoint> diff(_pbc_grain_num);

      for (unsigned int gr = 0; gr < _pbc_grain_num; ++gr)
      {
        diff[gr].d = (rand_point - _pbc_centerpoints[gr]).norm();
        diff[gr].gr = gr;
      }

      std::sort(diff.begin(), diff.end(), _customLess);

      Point closest_point = _pbc_centerpoints[diff[0].gr];
      Point next_closest_point = _pbc_centerpoints[diff[1].gr];
      Point third_closest_point = _pbc_centerpoints[diff[2].gr];

      Point vertex;

      // Locate voronoi vertex by searching nearest three grain centers
      // Project the random point on to the vertex (TJ point)
      if (_dim == 2 || _is_columnar_3D)
      {
        RankTwoTensor D;
        RankTwoTensor M0;
        RankTwoTensor M1;
        RankTwoTensor M2;

        for (unsigned int d = 0; d < dim; ++d)
        {
          D(0, d) = 2 * closest_point(d);
          D(1, d) = 2 * next_closest_point(d);
          D(2, d) = 2 * third_closest_point(d);
          M0(0, 0) += pow(closest_point(d), 2);
          M0(1, 0) += pow(next_closest_point(d), 2);
          M0(2, 0) += pow(third_closest_point(d), 2);
        }

        for (unsigned int r = 0; r < rank; ++r)
        {
          D(r, dim) = 1;
          M0(r, dim) = 1;
          M0(r, 1) = D(r, 1);
          M1(r, 0) = D(r, 0);
          M1(r, 1) = M0(r, 0);
          M1(r, dim) = 1;
        }

        Real inv_D_det = 1 / D.det();

        // This is to prevent leak of voids when nan or inf is returned by .det()
        if (std::isnan(inv_D_det) || std::isinf(inv_D_det))
          try_again = true;
        else
        {
          vertex(0) = M0.det() * inv_D_det;
          vertex(1) = M1.det() * inv_D_det;
        }

        if (_is_columnar_3D)
        {
          vertex(2) = rand_point(2);
        }

        _centers[vp] = vertex;
      }

      // It could be possible to have a more succinct code here since 2D and 3D cases have common
      // matrix elements. However, using DenseMatrix for 2D returns Lapack LU factorization
      // error when testing TJ void placement for hexagonal polycrystal, and RankTwoTensor
      // doesn't seem to support resizing to 4x4 for 3D.

      // Locate voronoi vertex by searching nearest four grain centers
      // Project the random point on to the nearest TJ line
      if (_dim == 3 && !_is_columnar_3D)
      {
        DenseMatrix<Real> D(rank, rank);
        DenseMatrix<Real> M0(rank, rank);
        DenseMatrix<Real> M1(rank, rank);
        DenseMatrix<Real> M2(rank, rank);

        for (unsigned int d = 0; d < _dim; ++d)
        {
          D(0, d) = 2 * closest_point(d);
          D(1, d) = 2 * next_closest_point(d);
          D(2, d) = 2 * third_closest_point(d);
          M0(0, 0) += pow(closest_point(d), 2);
          M0(1, 0) += pow(next_closest_point(d), 2);
          M0(2, 0) += pow(third_closest_point(d), 2);

          Point fourth_closest_point = _pbc_centerpoints[diff[3].gr];
          D(3, d) = 2 * fourth_closest_point(d);
          M0(3, 0) += pow(fourth_closest_point(d), 2);
        }

        for (unsigned int r = 0; r < rank; ++r)
        {
          D(r, _dim) = 1;
          M0(r, _dim) = 1;
          M0(r, 1) = D(r, 1);
          M1(r, 0) = D(r, 0);
          M1(r, 1) = M0(r, 0);
          M1(r, _dim) = 1;

          M0(r, 2) = D(r, 2);
          M1(r, 2) = D(r, 2);
          M1(3, 1) = M0(3, 0);
          M2(r, 0) = D(r, 0);
          M2(r, 1) = D(r, 1);
          M2(r, 2) = M0(r, 0);
          M2(r, 3) = 1;
        }

        Real inv_D_det = 1 / D.det();

        // This is to prevent leak of voids when nan or inf is returned by .det()
        if (std::isnan(inv_D_det) || std::isinf(inv_D_det))
          try_again = true;
        else
        {
          vertex(0) = M0.det() * inv_D_det;
          vertex(1) = M1.det() * inv_D_det;
          vertex(2) = M2.det() * inv_D_det;
        }

        if (try_again == false)
        {
          Point diff_pbc_centerpoints = next_closest_point - closest_point;
          Point diff_next_pbc_centerpoints = third_closest_point - closest_point;

          Point unit_cornercenters =
              diff_pbc_centerpoints / std::sqrt(diff_pbc_centerpoints * diff_pbc_centerpoints);
          Point unit_next_cornercenters =
              diff_next_pbc_centerpoints /
              std::sqrt(diff_next_pbc_centerpoints * diff_next_pbc_centerpoints);

          Real lambda = 0;

          Point corner_vector = unit_next_cornercenters.cross(unit_cornercenters);
          Point unit_corner_vector = corner_vector / std::sqrt(corner_vector * corner_vector);

          Point vertex_rand_vector = rand_point - vertex;

          for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
          {
            lambda += (vertex_rand_vector(i) * unit_corner_vector(i));
          }

          _centers[vp] = vertex + lambda * unit_corner_vector;
        }
      }

      // Check if void centers are within domain
      if (try_again == false)
        for (unsigned int i = 0; i < LIBMESH_DIM; i++)
          if ((_centers[vp](i) > _top_right(i)) || (_centers[vp](i) < _bottom_left(i)))
            try_again = true;

      // Check whether voids centers at TJs - equidistant to three grain centers
      if (try_again == false)
      {
        Real min_rij_1, min_rij_2, min_rij_3, rij, rij_diff_tol;

        min_rij_1 = _range.norm();
        min_rij_2 = min_rij_1;
        min_rij_3 = min_rij_1;

        rij_diff_tol = 0.1 * _radius;

        for (unsigned int gr = 0; gr < _pbc_grain_num; ++gr)
        {
          rij = (_centers[vp] - _pbc_centerpoints[gr]).norm();

          if (rij < min_rij_1)
          {
            min_rij_3 = min_rij_2;
            min_rij_2 = min_rij_1;
            min_rij_1 = rij;
          }
          else if (rij < min_rij_2)
            min_rij_2 = rij;
          else if (rij < min_rij_3)
            min_rij_3 = rij;
        }

        if (std::abs(min_rij_1 - min_rij_2) > rij_diff_tol ||
            std::abs(min_rij_2 - min_rij_3) > rij_diff_tol ||
            std::abs(min_rij_1 - min_rij_3) > rij_diff_tol)
          try_again = true;
      }

      if (try_again == false)
      {
        for (unsigned int i = 0; i < vp; ++i)
        {
          Real dist = _mesh.minPeriodicDistance(_var.number(), _centers[vp], _centers[i]);

          if (dist < _bubspac)
            try_again = true;
        }
      }
    } while (try_again == true);
  }
}

// Use methods in PolycrystalVoronoiVoidIC to set order parameter value
// and gradient within the void and along the interface
Real
PolycrystalVoronoiTJVoidIC::value(const Point & p)
{
  Real value = PolycrystalVoronoiVoidIC::value(p);

  return value;
}

RealGradient
PolycrystalVoronoiTJVoidIC::gradient(const Point & p)
{
  RealGradient gradient = PolycrystalVoronoiVoidIC::gradient(p);

  return gradient;
}
