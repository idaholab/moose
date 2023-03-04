/***************************************************************************************
 **  UEL, FOR ABAQUS/STANDARD INCORPORATING ISOTROPIC ELASTICITY                      **
 ***************************************************************************************/

#include "MooseTypes.h"
#include "MooseError.h"

#include <libmesh/int_range.h>

#include <Eigen/Core>
#include <Eigen/Dense>

extern "C" void
uel_(double RHS[],
     double AMATRX[],
     double /*SVARS*/[],
     double /*ENERGY*/[],
     int * /*NDOFEL*/,
     int * /*NRHS*/,
     int * /*NSVARS*/,
     double PROPS[],
     int * NPROPS,
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
     double /*PREDEF*/[],
     int * /*NPREDF*/,
     int /*LFLAGS*/[],
     int * /*MLVARX*/,
     double /*DDLMAG*/[],
     int * /*MDLOAD*/,
     double * /*PNEWDT*/,
     int /*JPROPS*/[],
     int * /*NJPROP*/,
     double * /*PERIOD*/)
{
  if (*NPROPS != 2)
    mooseError("Wrong number of properties passed in.");

  // solution at the beginning of the increment
  Eigen::Matrix<Real, 6, 1> u;
  u << U_[0], U_[1], U_[2], U_[3], U_[4], U_[5];

  // current solution increment
  Eigen::Matrix<Real, 6, 1> du;
  du << DU_[0], DU_[1], DU_[2], DU_[3], DU_[4], DU_[5];

  // current solution
  u += du;

  const auto & x1 = COORDS[0];
  const auto & y1 = COORDS[1];
  const auto & x2 = COORDS[2];
  const auto & y2 = COORDS[3];
  const auto & x3 = COORDS[4];
  const auto & y3 = COORDS[5];

  const auto x13 = x1 - x3;
  const auto x21 = x2 - x1;
  const auto y12 = y1 - y2;
  const auto y21 = y2 - y1;
  const auto x31 = x3 - x1;
  const auto y31 = y3 - y1;
  const auto x32 = x3 - x2;
  const auto y23 = y2 - y3;

  // Element Jacobian
  Eigen::Matrix<Real, 2, 2> J;
  J << x21, y21, //
      x31, y31;  //

  const auto Jdet = J.determinant();
  if (Jdet <= 0)
    mooseError("Negative Jacobian in element ", *JELEM);

  // const auto Jinv = J.inverse();

  // Area
  const auto A = Jdet / 2.0;

  Eigen::Matrix<Real, 3, 6> B;
  B << y23, 0.0, y31, 0.0, y12, 0.0, //
      0.0, x32, 0.0, x13, 0.0, x21,  //
      x32, y23, x13, y31, x21, y12;
  B *= 1.0 / Jdet;

  const auto Bt = B.transpose();

  // element thickness
  const Real t = 1.0;

  // Young's Modulus
  const auto Y = PROPS[0];

  // Poisson's ratio
  const auto nu = PROPS[1];

  Eigen::Matrix<Real, 3, 3> E;

  // plane stress
  // E << 1.0, nu, 0.0, //
  //     nu, 1.0, 0.0,  //
  //     0.0, 0.0, (1.0 - nu) / 2.0;
  // E *= Y / (1.0 - nu * nu);

  // plane strain
  E << 1.0 - nu, nu, 0.0, //
      nu, 1.0 - nu, 0.0,  //
      0.0, 0.0, (1.0 - 2.0 * nu) / 2.0;
  E *= Y / (1.0 + nu) / (1.0 - 2.0 * nu);

  // integrating the CST to obtain the Element characteristic matrix (6,6)
  const auto ke = Bt * E * B * t * A;

  // nodal forces
  const auto re = ke * u;

  // copy residual (nodal forces) and Jacobian (Element characteristic matrix)
  for (const auto i : make_range(6))
  {
    RHS[i] = -re(i);
    for (const auto j : make_range(6))
      AMATRX[i * 6 + j] = -ke(i, j);
  }
}
