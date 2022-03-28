#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <stdio.h>
// Ignore warnings from Eigen related to deprecated declarations (C++17)
#include "libmesh/ignore_warnings.h"
#include <Eigen/Eigen>
#include "libmesh/restore_warnings.h"

#include "MooseError.h"

void
buildStiffnessMatrix(Eigen::Ref<Eigen::Matrix<double, 6, 6>> C,
                     const double & G,
                     const double & lambda)
{
  C = Eigen::Matrix<double, 6, 6>::Zero();

  for (int i = 0; i < 3; i++)
  {
    C(i, i) += 2 * G;
    C(i + 3, i + 3) += G;
    for (int j = 0; j < 3; j++)
      C(i, j) += lambda;
  }
}

extern "C" void
umat_(double * stress,
      double * statev,
      double * ddsdde,
      double * sse,
      double * spd,
      double * scd,
      double * rpl,
      double * ddsddt,
      double * drplde,
      double * drpldt,
      double * stran,
      double * dstran,
      double * time,
      double * dtime,
      double * temp,
      double * dtemp,
      double * predef,
      double * dpred,
      char * cmname,
      int * ndi,
      int * nshr,
      int * ntens,
      int * nstatv,
      double * props,
      int * nprops,
      double * coords,
      double * drot,
      double * pnewdt,
      double * celent,
      double * dfgrd0,
      double * dfgrd1,
      int * noel,
      int * npt,
      int * layer,
      int * kspt,
      int * kstep,
      int * kinc,
      short cmname_len)
{

  double E = props[0];
  double nu = props[1];
  double G = E / 2.0 / (1.0 + nu);
  double lambda = 2.0 * G * nu / (1.0 - 2.0 * nu);
  double eps[6];
  double eps_trace;

  for (int i = 0; i < 6; i++)
    eps[i] = stran[i] + dstran[i];

  eps_trace = eps[0] + eps[1] + eps[2];

  for (int i = 0; i < 3; i++)
  {
    stress[i] = lambda * eps_trace + 2.0 * G * eps[i];
    stress[i + 3] = G * eps[i + 3];
  }

  Eigen::Matrix<double, 6, 6> C;
  buildStiffnessMatrix(C, G, lambda);

  for (int i = 0; i < 6; i++)
    for (int j = 0; j < 6; j++)
      ddsdde[6 * i + j] = 0.0;

  for (int i = 0; i < 3; i++)
  {
    ddsdde[6 * i + 0] = C(0, i);
    ddsdde[6 * i + 1] = C(1, i);
    ddsdde[6 * i + 2] = C(2, i);
    ddsdde[6 * (i + 3) + (i + 3)] = C(i + 3, i + 3);
  }

  // Print out eigenvalue using Eigen matrix
  auto eigenvalues = C.eigenvalues();

  if (*npt == 8 && time[0] == 0.0)
    for (int index_i = 0; index_i < *ntens; index_i++)
      Moose::out << "Eigenvalues " << index_i << " " << eigenvalues(index_i).real() << std::endl;
}

#pragma GCC diagnostic pop
