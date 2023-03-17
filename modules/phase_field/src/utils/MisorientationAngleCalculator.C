#include "MisorientationAngleCalculator.h"

namespace MisorientationAngleCalculator
{

  MisorientationAngleData calculateMisorientaion(EulerAngles & Euler1, EulerAngles & Euler2, MisorientationAngleData & s, const CrystalType & crystal_type)
  {
    // a conversion from Radians to degrees
    constexpr Real degree = 1.7453e-02;

    // Tolerance for identifying twins
    constexpr Real tolerance_mis = 3.90;

    // Initialize the computed twin tolerance
    Real misor_twinning = tolerance_mis + 1.0;

    Real value_acos = 0.0;
    const QuatReal & q1 = Euler1.toQuaternion();
    const QuatReal & q2 = Euler2.toQuaternion();
    const QuatReal mori_q1q2 = itimesQuaternion(q1, q2); // inv(q1)*q2

    const std::vector<QuatReal> q3_twin = getKeyQuat(QuatType::getTwinning);
    std::vector<QuatReal> qcs = getKeyQuat(QuatType::getCSymm, crystal_type);
    std::vector<QuatReal> qss = getKeyQuat(QuatType::getSSymm);

    // calculate misorientation angle
    value_acos = dotQuaternion(q1, q2, qcs, qss);
    if (value_acos <= 1.0 && value_acos >= -1.0)
      s._misor = (Real)(2.0*std::acos(value_acos))/degree;
    else
      s._misor =  tolerance_mis + 1;

    for (unsigned i = 0; i < q3_twin.size(); ++i)
    {
      value_acos = dotQuaternion(mori_q1q2, q3_twin[i], qcs, qcs);
      if (value_acos <= 1.0 && value_acos >= -1.0)
        misor_twinning = (Real)(2.0*std::acos(value_acos))/degree;

      s._is_twin = (bool)(misor_twinning < tolerance_mis); // Judging whether it is a twin boundary

      // Determine which type of twin boundary 0 ~ TT1 (tensile twins), 1 ~ CT1 (compression twins)
      if (s._is_twin)
        switch (i)
        {
          case 0:
              s._twin_type = TwinType::TT1;
              break;
          case 1:
              s._twin_type = TwinType::ST1;
              break;
          default:
              s._twin_type = TwinType::NONE;
              break;
        }
    }
    return s;
  }

  std::vector<QuatReal> getKeyQuat(const QuatType & quat_type, const CrystalType & crystal_type)
  {
    std::vector<std::vector<Real>> q_num;

    if (quat_type == QuatType::getTwinning)
      q_num = {
        {0.73728,  0.58508,  0.3378,  0},
        {0.84339,  0.53730,  0,       0}
      }; // Quaternion for HCP twinning from MTEX (TT1 and CT2);
    else if (quat_type == QuatType::getSSymm)
      q_num = {
        {-1.000e+00,  0.000e+00,  0.000e+00, -2.220e-16}
      }; // from MTEX;  
    else if ( quat_type == QuatType::getCSymm && crystal_type == CrystalType::HCP)
      q_num = {
        { 1.000e+00,  0.000e+00,  0.000e+00,  0.000e+00},
        { 0.000e+00,  8.660e-01, -5.000e-01,  0.000e+00},
        { 8.660e-01,  0.000e+00,  0.000e+00,  5.000e-01},
        { 0.000e+00,  5.000e-01, -8.660e-01,  0.000e+00},
        { 5.000e-01,  0.000e+00,  0.000e+00,  8.660e-01},
        { 0.000e+00,  0.000e+00, -1.000e+00,  0.000e+00},
        { 0.000e+00,  0.000e+00,  0.000e+00,  1.000e+00},
        { 0.000e+00, -5.000e-01, -8.660e-01,  0.000e+00},
        {-5.000e-01,  0.000e+00,  0.000e+00,  8.660e-01},
        { 0.000e+00, -8.660e-01, -5.000e-01,  0.000e+00},
        {-8.660e-01,  0.000e+00,  0.000e+00,  5.000e-01},
        { 0.000e+00, -1.000e+00,  0.000e+00,  0.000e+00}
      }; // 12 symmetric for hcp
    else if ( quat_type == QuatType::getCSymm && crystal_type == CrystalType::FCC)
      q_num = {
        { 1.000E+00,	 0.000E+00, 	 0.000E+00, 	 0.000E+00},
        { 5.000E-01,	 5.000E-01, 	 5.000E-01, 	 5.000E-01},
        {-5.000E-01,	 5.000E-01, 	 5.000E-01, 	 5.000E-01},
        { 6.123E-17,	 7.071E-01, 	 7.071E-01, 	 8.660E-17},
        {-7.071E-01,	 1.543E-16, 	 7.071E-01, 	 9.881E-17},
        {-7.071E-01,	-7.071E-01, 	 2.343E-16, 	 1.221E-17},
        { 7.071E-01,	 0.000E+00, 	 0.000E+00, 	 7.071E-01},
        { 1.110E-16,	 7.071E-01, 	 3.925E-17, 	 7.071E-01},
        {-7.071E-01,	 7.071E-01, 	 5.551E-17, 	 2.680E-16},
        {-1.793E-17,	 1.000E+00, 	 8.865E-17, 	 1.045E-16},
        {-5.000E-01,	 5.000E-01, 	 5.000E-01, 	-5.000E-01},
        {-5.000E-01,	-5.000E-01, 	 5.000E-01, 	-5.000E-01},
        { 6.123E-17,	 0.000E+00, 	 0.000E+00, 	 1.000E+00},
        {-5.000E-01,	 5.000E-01, 	-5.000E-01, 	 5.000E-01},
        {-5.000E-01,	 5.000E-01, 	-5.000E-01, 	-5.000E-01},
        {-8.660E-17,	 7.071E-01, 	-7.071E-01, 	 6.123E-17},
        {-1.421E-16,	 7.071E-01, 	-1.110E-16, 	-7.071E-01},
        {-5.551E-17,	 1.910E-16, 	 7.071E-01, 	-7.071E-01},
        {-7.071E-01,	 0.000E+00, 	 0.000E+00, 	 7.071E-01},
        {-7.071E-01,	 7.177E-17, 	-7.071E-01, 	 1.340E-16},
        {-3.005E-16,	 5.551E-17, 	-7.071E-01, 	-7.071E-01},
        {-1.045E-16,	 1.009E-16, 	-1.000E+00, 	-1.793E-17},
        { 5.000E-01,	 5.000E-01, 	-5.000E-01, 	-5.000E-01},
        { 5.000E-01,	 5.000E-01, 	 5.000E-01, 	-5.000E-01}
      }; // 24 symmetric for fcc

    std::vector<QuatReal> q(q_num.size());
    for (unsigned int i = 0; i < q_num.size(); ++i)
    {
      q[i].w() = q_num[i][0];
      q[i].x() = q_num[i][1];
      q[i].y() = q_num[i][2];
      q[i].z() = q_num[i][3];
    }
    return q;
  }

  Real dotQuaternion(const QuatReal & o1, const QuatReal & o2, 
                                              const std::vector<QuatReal> & qcs, 
                                              const std::vector<QuatReal> & qss)
  {
    Real d = 0.0; // used to get misorientation angle
    QuatReal mori = itimesQuaternion(o1, o2);

    if (qss.size() <= 1)
      d = dotOuterQuaternion(mori, qcs); 
    else
      d = mtimes2Quaternion(o1, qcs, o2); // mtimesQuaternion(qss,mtimesQuaternion(o1,qcs,0),1)

    return d;
  }

  QuatReal itimesQuaternion(const QuatReal & q1, const QuatReal & q2)
  {
    Real a1, b1, c1, d1;
    Real a2, b2, c2, d2;

    a1 = q1.w(); b1 = -q1.x(); c1 = -q1.y(); d1 = -q1.z();
    a2 = q2.w(); b2 = q2.x(); c2 = q2.y(); d2 = q2.z();

    // standart algorithm
    QuatReal q;
    q.w() = a1 * a2 - b1 * b2 - c1 * c2 - d1 * d2;
    q.x() = b1 * a2 + a1 * b2 - d1 * c2 + c1 * d2;
    q.y() = c1 * a2 + d1 * b2 + a1 * c2 - b1 * d2;
    q.z() = d1 * a2 - c1 * b2 + b1 * c2 + a1 * d2;

    return q;
  }

  Real dotOuterQuaternion(const QuatReal & rot1, const std::vector<QuatReal> & rot2)
  {
    std::vector<Real> d_vec(rot2.size());

    for (unsigned int i = 0; i < rot2.size(); ++i)
      d_vec[i] = std::abs(rot1.w()*rot2[i].w() + rot1.x()*rot2[i].x() + rot1.y()*rot2[i].y() + rot1.z()*rot2[i].z()); // rot1 * rot2'

    Real d = *std::max_element(d_vec.begin(), d_vec.end());

    return d;
  }

  Real mtimes2Quaternion(const QuatReal & q1, const std::vector<QuatReal> & q2, const QuatReal & qTwin)
  {
    // step 1 -- q1-o1(1*4), q2'-qcs'(4*12)
    std::vector<QuatReal> q_s1(q2.size());
    for (unsigned int j = 0; j < q2.size(); ++j)
    {
      q_s1[j].w() = q1.w()*q2[j].w() - q1.x()*q2[j].x() - q1.y()*q2[j].y() - q1.z()*q2[j].z();
      q_s1[j].x() = q1.x()*q2[j].w() + q1.w()*q2[j].x() - q1.z()*q2[j].y() + q1.y()*q2[j].z();
      q_s1[j].y() = q1.y()*q2[j].w() + q1.z()*q2[j].x() + q1.w()*q2[j].y() - q1.x()*q2[j].z();
      q_s1[j].z() = q1.z()*q2[j].w() - q1.y()*q2[j].x() + q1.x()*q2[j].y() + q1.w()*q2[j].z();
    }

    // step 2 -- q1-qcs(12*4), q2'-q_s1'(1*12)
    std::vector<std::vector<QuatReal>> q_s2(q2.size());
    for (unsigned int i = 0; i < q2.size(); ++i)
      q_s2[i].resize(q2.size());

    for (unsigned int i = 0; i < q2.size(); ++i)
      for (unsigned int j = 0; j < q2.size(); ++j)
      {
        q_s2[i][j].w() = q2[i].w()*q_s1[j].w() - q2[i].x()*q_s1[j].x() - q2[i].y()*q_s1[j].y() - q2[i].z()*q_s1[j].z();
        q_s2[i][j].x() = q2[i].x()*q_s1[j].w() + q2[i].w()*q_s1[j].x() - q2[i].z()*q_s1[j].y() + q2[i].y()*q_s1[j].z();
        q_s2[i][j].y() = q2[i].y()*q_s1[j].w() + q2[i].z()*q_s1[j].x() + q2[i].w()*q_s1[j].y() - q2[i].x()*q_s1[j].z();
        q_s2[i][j].z() = q2[i].z()*q_s1[j].w() - q2[i].y()*q_s1[j].x() + q2[i].x()*q_s1[j].y() + q2[i].w()*q_s1[j].z();      
      }
    
    // inline dot_outer(o1,o2) and find max d_vec
    Real d_max = 0.0;
    std::vector<std::vector<Real>> d_vec(q2.size());
    for (unsigned int i = 0; i < q2.size(); ++i)
      d_vec[i].resize(q2.size());

    for (unsigned int i = 0; i < q2.size(); ++i)
      for (unsigned int j = 0; j < q2.size(); ++j)
      {
        d_vec[i][j] = std::abs(qTwin.w()*q_s2[i][j].w() + qTwin.x()*q_s2[i][j].x() + qTwin.y()*q_s2[i][j].y() + qTwin.z()*q_s2[i][j].z());
        if (d_max < d_vec[i][j])
          d_max = d_vec[i][j];
      }
    return d_max;
  }
}