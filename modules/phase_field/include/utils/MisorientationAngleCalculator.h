#pragma once

#include "Moose.h"
#include "EulerAngles.h"
#include "EulerAngleProvider.h"

typedef Eigen::Quaternion<Real> quatReal;
struct misoriAngle_isTwining{Real misor; bool isTwinning; std::string twinType;};

/**
 * This is an orientation difference calculator, inspired by MTEX.
 * However, it is an incomplete version and needs further optimization TODO.
 */
class MisorientationAngleCalculator
{
public:
  // function 1: input Euler1 and Euler2, output s
  static misoriAngle_isTwining calculateMisorientaion(EulerAngles & Euler1, EulerAngles & Euler2, misoriAngle_isTwining & s, const std::string & CrystalType = "hcp");  

  // function 2: Obtaining the key orientation using quaternion, including twinning, CS, SS
  static std::vector<quatReal> getKeyQuat(const std::string & QuatType, const std::string & CrystalKype = "hcp");

  // function 3.1: computer the scalar dot product using quaternion
  static Real dotQuaternion(const quatReal & o1, const quatReal & o2, 
                     const std::vector<quatReal> & qcs, 
                     const std::vector<quatReal> & qss);

  // function 3.2: computes inv(o1) .* o2 usig quaternion
  static quatReal itimesQuaternion(const quatReal & q1, const quatReal & q2);

  // function 3.3: computes outer inner product between two quaternions
  static Real dotOuterQuaternion(const quatReal & rot1, const std::vector<quatReal> & rot2);

  // function 3.4: X*Y is the matrix product of X and Y. ~twice~
  static Real mtimes2Quaternion(const quatReal & q1, const std::vector<quatReal> & q2, const quatReal & qTwin);  
};