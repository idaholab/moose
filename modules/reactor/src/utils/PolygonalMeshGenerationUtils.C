//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolygonalMeshGenerationUtils.h"

Real
PolygonalMeshGenerationUtils::dummyTRI6VolCalculator(const std::pair<Real, Real> & azi_pair)
{
  // The algorithm is copied from libMesh's face_tri6.C
  // Original license is LGPL so it can be used here.
  const Real & azi_1 = azi_pair.first;
  const Real & azi_2 = azi_pair.second;

  Point x0 = Point(0.0, 0.0, 0.0), x1 = Point(1.0, 0.0, 0.0),
        x2 = Point(std::cos(azi_1 + azi_2), std::sin(azi_1 + azi_2), 0.0),
        x3 = Point(0.5, 0.0, 0.0), x4 = Point(std::cos(azi_1), std::sin(azi_1), 0.0),
        x5 = Point(std::cos(azi_1 + azi_2) / 2.0, std::sin(azi_1 + azi_2) / 2.0, 0.0);

  // Construct constant data vectors.
  // \vec{x}_{\xi}  = \vec{a1}*xi + \vec{b1}*eta + \vec{c1}
  // \vec{x}_{\eta} = \vec{a2}*xi + \vec{b2}*eta + \vec{c2}
  Point a1 = 4 * x0 + 4 * x1 - 8 * x3, b1 = 4 * x0 - 4 * x3 + 4 * x4 - 4 * x5,
        c1 = -3 * x0 - 1 * x1 + 4 * x3, b2 = 4 * x0 + 4 * x2 - 8 * x5,
        c2 = -3 * x0 - 1 * x2 + 4 * x5;

  // 7-point rule, exact for quintics.
  const unsigned int N = 7;

  // Parameters of the quadrature rule
  const static Real w1 = Real(31) / 480 + Real(std::sqrt(15.0L) / 2400),
                    w2 = Real(31) / 480 - Real(std::sqrt(15.0L) / 2400),
                    q1 = Real(2) / 7 + Real(std::sqrt(15.0L) / 21),
                    q2 = Real(2) / 7 - Real(std::sqrt(15.0L) / 21);

  const static Real xi[N] = {Real(1) / 3, q1, q1, 1 - 2 * q1, q2, q2, 1 - 2 * q2};
  const static Real eta[N] = {Real(1) / 3, q1, 1 - 2 * q1, q1, q2, 1 - 2 * q2, q2};
  const static Real wts[N] = {Real(9) / 80, w1, w1, w1, w2, w2, w2};

  // Approximate the area with quadrature
  Real vol = 0.;
  for (unsigned int q = 0; q < N; ++q)
    vol += wts[q] * cross_norm(xi[q] * a1 + eta[q] * b1 + c1, xi[q] * b1 + eta[q] * b2 + c2);

  return vol;
}

Real
PolygonalMeshGenerationUtils::radiusCorrectionFactor(const std::vector<Real> & azimuthal_list,
                                                     const bool full_circle,
                                                     const unsigned int order,
                                                     const bool is_first_value_vertex)
{
  Real tmp_acc = 0.0;
  Real tmp_acc_azi = 0.0;
  if (order == 1)
  {
    // A vector to collect the azimuthal intervals of all EDGE2 side elements
    std::vector<Real> azi_interval_list;
    for (unsigned int i = 1; i < azimuthal_list.size(); i++)
      azi_interval_list.push_back((azimuthal_list[i] - azimuthal_list[i - 1]) / 180.0 * M_PI);
    if (full_circle)
      azi_interval_list.push_back((azimuthal_list.front() + 360.0 - azimuthal_list.back()) / 180.0 *
                                  M_PI);
    // summation of triangles S = 0.5 * r * r * Sigma_i [sin (azi_i)]
    // Circle area S_c = pi * r_0 * r_0
    // r = sqrt{2 * pi / Sigma_i [sin (azi_i)]} * r_0
    for (const auto i : index_range(azi_interval_list))
    {
      tmp_acc += std::sin(azi_interval_list[i]);
      tmp_acc_azi += azi_interval_list[i];
    }
  }
  else // order == 2
  {
    // A vector to collect the pairs of azimuthal intervals of all EDGE3 side elements
    // The midpoint divides the interval into two parts, which are stored as first and second of
    // each pair; the sum of the two parts is the full interval of the EDGE3 element
    std::vector<std::pair<Real, Real>> azi_interval_list;
    for (unsigned int i = is_first_value_vertex ? 2 : 3; i < azimuthal_list.size(); i += 2)
      azi_interval_list.push_back(
          std::make_pair((azimuthal_list[i - 1] - azimuthal_list[i - 2]) / 180.0 * M_PI,
                         (azimuthal_list[i] - azimuthal_list[i - 1]) / 180.0 * M_PI));
    // If it is not a full circle, the first value should always belong to a vertex
    if (full_circle)
    {
      if (is_first_value_vertex)
        azi_interval_list.push_back(std::make_pair(
            (azimuthal_list.back() - azimuthal_list[azimuthal_list.size() - 2]) / 180.0 * M_PI,
            (azimuthal_list.front() + 360.0 - azimuthal_list.back()) / 180.0 * M_PI));
      else
        azi_interval_list.push_back(
            std::make_pair((azimuthal_list.front() + 360.0 - azimuthal_list.back()) / 180.0 * M_PI,
                           (azimuthal_list[1] - azimuthal_list.front()) / 180.0 * M_PI));
    }
    // Use the libMesh TRI6 element volume algorithm
    for (const auto i : index_range(azi_interval_list))
    {
      tmp_acc += 2.0 * dummyTRI6VolCalculator(azi_interval_list[i]);
      tmp_acc_azi += azi_interval_list[i].first + azi_interval_list[i].second;
    }
  }
  return std::sqrt(tmp_acc_azi / tmp_acc);
}
