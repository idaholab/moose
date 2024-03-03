//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "build_stiffness_matrix.h"

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
