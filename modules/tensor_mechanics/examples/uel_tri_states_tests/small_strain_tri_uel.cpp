/***************************************************************************************
 **  UEL, FOR ABAQUS/STANDARD INCORPORATING ISOTROPIC ELASTICITY                      **
 ***************************************************************************************/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <Eigen/Core>
#include <Eigen/Dense>

#include <iostream>

void umat_simple(double * stress,
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
                 short cmname_len);

extern "C" void
uel_(double RHS[],
     double AMATRX[],
     double SVARS[],
     double /*ENERGY*/[],
     int * /*NDOFEL*/,
     int * /*NRHS*/,
     int * /*NSVARS*/,
     double PROPS[],
     int * /*NPROPS*/,
     double COORDS[],
     int * /*MCRD*/,
     int * /*NNODE*/,
     double U_[],
     double DU_[],
     double /*V_*/[],
     double /*A_*/[],
     int * /*JTYPE*/,
     double /*TIME*/[],
     double * /*DTIME*/,
     int * /*KSTEP*/,
     int * /*KINC*/,
     int * JELEM,
     double /*PRAMS*/[],
     int * /*NDLOAD*/,
     int /*JDLTYP*/[],
     double /*ADLMAG*/[],
     double PREDEF[],
     int * NPREDF,
     int /*LFLAGS*/[],
     int * /*MLVARX*/,
     double /*DDLMAG*/[],
     int * /*MDLOAD*/,
     double * /*PNEWDT*/,
     int /*JPROPS*/[],
     int * /*NJPROP*/,
     double * /*PERIOD*/)
{
  // solution at the beginning of the increment
  Eigen::Matrix<double, 6, 1> u;
  u << U_[0], U_[1], U_[2], U_[3], U_[4], U_[5];

  // current solution increment
  Eigen::Matrix<double, 6, 1> du;
  du << DU_[0], DU_[1], DU_[2], DU_[3], DU_[4], DU_[5];

  // current solution
  u += du;

  const auto x1 = COORDS[0];
  const auto y1 = COORDS[1];
  const auto x2 = COORDS[2];
  const auto y2 = COORDS[3];
  const auto x3 = COORDS[4];
  const auto y3 = COORDS[5];

  const auto x13 = x1 - x3;
  const auto x21 = x2 - x1;
  const auto y12 = y1 - y2;
  const auto y21 = y2 - y1;
  const auto x31 = x3 - x1;
  const auto y31 = y3 - y1;
  const auto x32 = x3 - x2;
  const auto y23 = y2 - y3;

  // Element Jacobian
  Eigen::Matrix<double, 2, 2> J;
  J << x21, y21, //
      x31, y31;  //

  const auto Jdet = J.determinant();
  if (Jdet <= 0)
    std::cerr << "Negative Jacobian in element git " << *JELEM << "\n";

  // Area
  const auto A = Jdet / 2.0;

  Eigen::Matrix<double, 3, 6> B;
  B << y23, 0.0, y31, 0.0, y12, 0.0, //
      0.0, x32, 0.0, x13, 0.0, x21,  //
      x32, y23, x13, y31, x21, y12;
  B *= 1.0 / Jdet;

  const auto Bt = B.transpose();

  // element thickness
  const double t = 1.0;

  // Young's Modulus
  auto Y = PROPS[0];

  // Coarse interpolation for testing spatially varying external fields
  const double temperature = 1.0 / 3.0 * (PREDEF[0] + PREDEF[4] + PREDEF[8]);
  const double voltage = 1.0 / 3.0 * (PREDEF[2] + PREDEF[6] + PREDEF[10]);

  Y *= (temperature + voltage) / 1000;

  // State-dependent contribution of elasticity modulus.
  Y *= (1 + SVARS[6] * 10.0);

  double PROPS_MOD[2];
  PROPS_MOD[0] = Y;
  PROPS_MOD[1] = PROPS[1];

  const auto nu = PROPS[1];

  double * statev = nullptr;
  double * sse = nullptr;
  double * spd = nullptr;
  double * scd = nullptr;
  double * rpl = nullptr;
  double * ddsddt = nullptr;
  double * drplde = nullptr;
  double * drpldt = nullptr;
  double * stran = nullptr;
  double * time = nullptr;
  double * dtime = nullptr;
  double * temp = nullptr;
  double * dtemp = nullptr;
  double * predef = nullptr;
  double * dpred = nullptr;
  char * cmname = nullptr;
  int * ndi = nullptr;
  int * nshr = nullptr;
  int * nstatv = nullptr;
  int * nprops = nullptr;
  double * coords = nullptr;
  double * drot = nullptr;
  double * pnewdt = nullptr;
  double * celent = nullptr;
  double * dfgrd0 = nullptr;
  double * dfgrd1 = nullptr;
  int * noel = nullptr;
  int * npt = nullptr;
  int * layer = nullptr;
  int * kspt = nullptr;
  int * kstep = nullptr;
  int * kinc = nullptr;
  short cmname_len = 32;

  // Number of state variables for uel.
  // 3 (stress tensor) * 1 (number of quadrature points)
  int range_stress[] = {0, 1, 2};
  double stress[3];
  double dstran[3];

  // 3x3 ddsdde
  double ddsdde[9];
  int ntens = 3;

  // copy residual (nodal forces) and Jacobian (Element characteristic matrix)
  for (const auto i : range_stress)
    stress[i] = SVARS[i];

  auto depsilon = B * du;
  for (const auto i : range_stress)
    dstran[i] = depsilon[i];

  umat_simple(stress,
              statev,
              ddsdde,
              sse,
              spd,
              scd,
              rpl,
              ddsddt,
              drplde,
              drpldt,
              stran,
              dstran,
              time,
              dtime,
              temp,
              dtemp,
              predef,
              dpred,
              cmname,
              ndi,
              nshr,
              &ntens,
              nstatv,
              PROPS_MOD,
              nprops,
              coords,
              drot,
              pnewdt,
              celent,
              dfgrd0,
              dfgrd1,
              noel,
              npt,
              layer,
              kspt,
              kstep,
              kinc,
              cmname_len);

  for (const auto i : range_stress)
    SVARS[i] = stress[i];

  Eigen::Matrix<double, 3, 3> E;

  E << ddsdde[0], ddsdde[1], ddsdde[2], //
      ddsdde[3], ddsdde[4], ddsdde[5],  //
      ddsdde[6], ddsdde[7], ddsdde[8];

  // integrating the CST to obtain the Element characteristic matrix (6,6)
  const Eigen::Matrix<double, 6, 6> ke = Bt * E * B * t * A;

  Eigen::Matrix<double, 3, 1> Sigma;
  Sigma << stress[0], stress[1], stress[2];

  // nodal forces
  const auto re = Bt * Sigma * t * A;

  const int range[] = {0, 1, 2, 3, 4, 5};
  // copy residual (nodal forces) and Jacobian (Element characteristic matrix)
  for (const auto i : range)
  {
    RHS[i] = -re(i);
    for (const auto j : range)
      AMATRX[i * 6 + j] = -ke(i, j);
  }

  SVARS[6] += std::fabs(dstran[0]) + std::fabs(dstran[1]);

  // This is the "equivalent" piece of code to UMAT within the UEL routine
  if (false)
  {
    // If true the test should pass (as long as umat does nothing)
    // E -> ddsdde
    E << 1.0 - nu, nu, 0.0, //
        nu, 1.0 - nu, 0.0,  //
        0.0, 0.0, (1.0 - 2.0 * nu) / 2.0;
    E *= Y / (1.0 + nu) / (1.0 - 2.0 * nu);

    // integrating the CST to obtain the Element characteristic matrix (6,6)
    const auto ke = Bt * E * B * t * A;

    // nodal forces
    const auto re = ke * u;

    int range[] = {0, 1, 2, 3, 4, 5};
    // copy residual (nodal forces) and Jacobian (Element characteristic matrix)
    for (const auto i : range)
    {
      RHS[i] = -re(i);
      for (const auto j : range)
        AMATRX[i * 6 + j] = -ke(i, j);
    }
  }
}

void
umat_simple(double * stress,
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
  const double E = props[0];
  const double nu = props[1];
  const double constant = E / (1.0 + nu) / (1.0 - 2.0 * nu);

  // First column
  ddsdde[0] = constant * (1.0 - nu);
  ddsdde[1] = constant * nu;
  ddsdde[2] = 0.0;

  // Second column
  ddsdde[3] = constant * nu;
  ddsdde[4] = constant * (1.0 - nu);
  ddsdde[5] = 0.0;

  // Third column
  ddsdde[6] = 0.0;
  ddsdde[7] = 0.0;
  ddsdde[8] = constant * (0.5 - nu);

  // calculate stress
  for (int k1 = 0; k1 < *ntens; ++k1)
    for (int k2 = 0; k2 < *ntens; ++k2)
      stress[k1] += ddsdde[k1 * *ntens + k2] * dstran[k2];
}

#pragma GCC diagnostic pop
