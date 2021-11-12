//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <stdexcept>

#include "omi_for_c.h"
#include "build_stiffness_matrix.h"

extern "C" void FOR_NAME(umat, UMAT)(double * stress,
                                     double * /*statev*/,
                                     double * ddsdde,
                                     double * /*sse*/,
                                     double * /*spd*/,
                                     double * /*scd*/,
                                     double * /*rpl*/,
                                     double * /*ddsddt*/,
                                     double * /*drplde*/,
                                     double * /*drpldt*/,
                                     double * stran,
                                     double * dstran,
                                     double * /*time*/,
                                     double * /*dtime*/,
                                     double * /*temp*/,
                                     double * /*dtemp*/,
                                     double * /*predef*/,
                                     double * /*dpred*/,
                                     char * /*cmname*/,
                                     int * /*ndi*/,
                                     int * /*nshr*/,
                                     int * /*ntens*/,
                                     int * /*nstatv*/,
                                     double * props,
                                     int * nprops,
                                     double * /*coords*/,
                                     double * /*drot*/,
                                     double * /*pnewdt*/,
                                     double * /*celent*/,
                                     double * /*dfgrd0*/,
                                     double * /*dfgrd1*/,
                                     int * /*noel*/,
                                     int * /*npt*/,
                                     int * /*layer*/,
                                     int * /*kspt*/,
                                     int * /*kstep*/,
                                     int * /*kinc*/,
                                     short /*cmname_len*/)
{
  if (*nprops != 2)
    throw std::invalid_argument("This UMAT requires exactly two properties.");

  double E = props[0];
  double nu = props[1];
  double G = E / 2.0 / (1.0 + nu);
  double lambda = 2.0 * G * nu / (1.0 - 2.0 * nu);
  double eps[6];

  // Build stress as in
  // https://github.com/michael-schw/Abaqus-UMAT-Cpp-Subroutine/blob/main/umat.cpp
  for (int i = 0; i < 6; i++)
    eps[i] = stran[i] + dstran[i];

  auto eps_trace = eps[0] + eps[1] + eps[2];

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
}
