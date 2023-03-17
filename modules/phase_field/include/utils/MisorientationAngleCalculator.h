#pragma once

#include "Moose.h"
#include "EulerAngles.h"
#include "EulerAngleProvider.h"

typedef Eigen::Quaternion<Real> QuatReal;
struct MisorientationAngleData{Real misor; bool isTwinning; std::string twinType;};

/**
 * This is an orientation difference calculator, inspired by MTEX.
 * However, it is an incomplete version and needs further optimization TODO.
 */
namespace MisorientationAngleCalculator
{
  // function 1: input Euler1 and Euler2, output s
  MisorientationAngleData calculateMisorientaion(EulerAngles & Euler1, EulerAngles & Euler2, MisorientationAngleData & s, const std::string & CrystalType = "hcp");  

  // function 2: Obtaining the key orientation using quaternion, including twinning, CS, SS
  std::vector<QuatReal> getKeyQuat(const std::string & QuatType, const std::string & CrystalKype = "hcp");

  // function 3.1: computer the scalar dot product using quaternion
  Real dotQuaternion(const QuatReal & o1, const QuatReal & o2, 
                     const std::vector<QuatReal> & qcs, 
                     const std::vector<QuatReal> & qss);

  // function 3.2: computes inv(o1) .* o2 usig quaternion
  QuatReal itimesQuaternion(const QuatReal & q1, const QuatReal & q2);

  // function 3.3: computes outer inner product between two quaternions
  Real dotOuterQuaternion(const QuatReal & rot1, const std::vector<QuatReal> & rot2);

  // function 3.4: X*Y is the matrix product of X and Y. ~twice~
  Real mtimes2Quaternion(const QuatReal & q1, const std::vector<QuatReal> & q2, const QuatReal & qTwin);  
};