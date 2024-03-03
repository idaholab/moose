//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <Eigen/Eigen>

/**
 * build a 6x6 representation of the stiffness tensor in C from the
 * shear modulus G and Lame's first parameter lambda.
 */
void buildStiffnessMatrix(Eigen::Ref<Eigen::Matrix<double, 6, 6>> C,
                          const double & G,
                          const double & lambda);
