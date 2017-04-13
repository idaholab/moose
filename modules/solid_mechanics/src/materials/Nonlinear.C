/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Nonlinear.h"
#include "SolidModel.h"
#include "Problem.h"
#include "SymmIsotropicElasticityTensor.h"

// libmesh includes
#include "libmesh/quadrature.h"

namespace SolidMechanics
{

Nonlinear::Nonlinear(SolidModel & solid_model,
                     const std::string & name,
                     const InputParameters & parameters)
  : Element(solid_model, name, parameters),
    _decomp_method(RashidApprox),
    _incremental_rotation(3, 3),
    _Uhat(3, 3)
{

  std::string increment_calculation = solid_model.getParam<std::string>("increment_calculation");
  std::transform(increment_calculation.begin(),
                 increment_calculation.end(),
                 increment_calculation.begin(),
                 ::tolower);
  if (increment_calculation == "rashidapprox")
  {
    _decomp_method = RashidApprox;
  }
  else if (increment_calculation == "eigen")
  {
    _decomp_method = Eigen;
  }
  else
  {
    mooseError("The options for the increment calculation are RashidApprox and Eigen.");
  }
}

////////////////////////////////////////////////////////////////////////

Nonlinear::~Nonlinear() {}

////////////////////////////////////////////////////////////////////////

void
Nonlinear::computeStrainAndRotationIncrement(const ColumnMajorMatrix & Fhat,
                                             SymmTensor & strain_increment)
{
  if (_decomp_method == RashidApprox)
  {
    computeStrainIncrement(Fhat, strain_increment);
    computePolarDecomposition(Fhat);
  }

  else if (_decomp_method == Eigen)
  {
    Element::polarDecompositionEigen(Fhat, _incremental_rotation, strain_increment);
  }

  else
  {
    mooseError("Unknown polar decomposition type!");
  }
}

////////////////////////////////////////////////////////////////////////

void
Nonlinear::computeStrainIncrement(const ColumnMajorMatrix & Fhat, SymmTensor & strain_increment)
{

  //
  // A calculation of the strain at the mid-interval is probably more
  // accurate (second vs. first order).  This would require the
  // incremental deformation gradient at the mid-step, which we
  // currently don't have.  We would then have to calculate the
  // rotation for the whole step.
  //
  //
  // We are looking for:
  //     log( Uhat )
  //  =  log( sqrt( Fhat^T*Fhat ) )
  //  =  log( sqrt( Chat ) )
  // A Taylor series expansion gives:
  //     ( Chat - 0.25 * Chat^T*Chat - 0.75 * I )
  //  =  ( - 0.25 * Chat^T*Chat + Chat - 0.75 * I )
  //  =  ( (0.25*Chat - 0.75*I) * (Chat - I) )
  //  =  ( B * A )
  //    B
  //  = 0.25*Chat - 0.75*I
  //  = 0.25*(Chat - I) - 0.5*I
  //  = 0.25*A - 0.5*I
  //

  const Real Uxx = Fhat(0, 0);
  const Real Uxy = Fhat(0, 1);
  const Real Uxz = Fhat(0, 2);
  const Real Uyx = Fhat(1, 0);
  const Real Uyy = Fhat(1, 1);
  const Real Uyz = Fhat(1, 2);
  const Real Uzx = Fhat(2, 0);
  const Real Uzy = Fhat(2, 1);
  const Real Uzz = Fhat(2, 2);

  const Real Axx = Uxx * Uxx + Uyx * Uyx + Uzx * Uzx - 1.0;
  const Real Axy = Uxx * Uxy + Uyx * Uyy + Uzx * Uzy;
  const Real Axz = Uxx * Uxz + Uyx * Uyz + Uzx * Uzz;
  const Real Ayy = Uxy * Uxy + Uyy * Uyy + Uzy * Uzy - 1.0;
  const Real Ayz = Uxy * Uxz + Uyy * Uyz + Uzy * Uzz;
  const Real Azz = Uxz * Uxz + Uyz * Uyz + Uzz * Uzz - 1.0;

  const Real Bxx = 0.25 * Axx - 0.5;
  const Real Bxy = 0.25 * Axy;
  const Real Bxz = 0.25 * Axz;
  const Real Byy = 0.25 * Ayy - 0.5;
  const Real Byz = 0.25 * Ayz;
  const Real Bzz = 0.25 * Azz - 0.5;

  strain_increment.xx(-(Bxx * Axx + Bxy * Axy + Bxz * Axz));
  strain_increment.xy(-(Bxx * Axy + Bxy * Ayy + Bxz * Ayz));
  strain_increment.zx(-(Bxx * Axz + Bxy * Ayz + Bxz * Azz));
  strain_increment.yy(-(Bxy * Axy + Byy * Ayy + Byz * Ayz));
  strain_increment.yz(-(Bxy * Axz + Byy * Ayz + Byz * Azz));
  strain_increment.zz(-(Bxz * Axz + Byz * Ayz + Bzz * Azz));
}

////////////////////////////////////////////////////////////////////////

void
Nonlinear::computePolarDecomposition(const ColumnMajorMatrix & Fhat)
{

  // From Rashid, 1993.

  const Real Uxx = Fhat(0, 0);
  const Real Uxy = Fhat(0, 1);
  const Real Uxz = Fhat(0, 2);
  const Real Uyx = Fhat(1, 0);
  const Real Uyy = Fhat(1, 1);
  const Real Uyz = Fhat(1, 2);
  const Real Uzx = Fhat(2, 0);
  const Real Uzy = Fhat(2, 1);
  const Real Uzz = Fhat(2, 2);

  const Real Ax = Uyz - Uzy;
  const Real Ay = Uzx - Uxz;
  const Real Az = Uxy - Uyx;
  const Real Q = 0.25 * (Ax * Ax + Ay * Ay + Az * Az);
  const Real traceF = Uxx + Uyy + Uzz;
  const Real P = 0.25 * (traceF - 1) * (traceF - 1);
  const Real Y = 1 / ((Q + P) * (Q + P) * (Q + P));

  const Real C1 = std::sqrt(P * (1 + (P * (Q + Q + (Q + P))) * (1 - (Q + P)) * Y));
  const Real C2 = 0.125 + Q * 0.03125 * (P * P - 12 * (P - 1)) / (P * P);
  const Real C3 = 0.5 * std::sqrt((P * Q * (3 - Q) + P * P * P + Q * Q) * Y);

  // Since the input to this routine is the incremental deformation gradient
  //   and not the inverse incremental gradient, this result is the transpose
  //   of the one in Rashid's paper.
  _incremental_rotation(0, 0) = C1 + (C2 * Ax) * Ax;
  _incremental_rotation(0, 1) = (C2 * Ay) * Ax + (C3 * Az);
  _incremental_rotation(0, 2) = (C2 * Az) * Ax - (C3 * Ay);
  _incremental_rotation(1, 0) = (C2 * Ax) * Ay - (C3 * Az);
  _incremental_rotation(1, 1) = C1 + (C2 * Ay) * Ay;
  _incremental_rotation(1, 2) = (C2 * Az) * Ay + (C3 * Ax);
  _incremental_rotation(2, 0) = (C2 * Ax) * Az + (C3 * Ay);
  _incremental_rotation(2, 1) = (C2 * Ay) * Az - (C3 * Ax);
  _incremental_rotation(2, 2) = C1 + (C2 * Az) * Az;
}

////////////////////////////////////////////////////////////////////////

void
Nonlinear::finalizeStress(std::vector<SymmTensor *> & t)
{
  // Using the incremental rotation, update the stress to the current configuration (R*T*R^T)
  for (unsigned i = 0; i < t.size(); ++i)
  {
    Element::rotateSymmetricTensor(_incremental_rotation, *t[i], *t[i]);
  }
}

////////////////////////////////////////////////////////////////////////

void
Nonlinear::computeStrain(const unsigned qp,
                         const SymmTensor & total_strain_old,
                         SymmTensor & total_strain_new,
                         SymmTensor & strain_increment)
{
  computeStrainAndRotationIncrement(_Fhat[qp], strain_increment);

  total_strain_new = strain_increment;
  total_strain_new += total_strain_old;
}

////////////////////////////////////////////////////////////////////////

void
Nonlinear::init()
{
  _Fhat.resize(_solid_model.qrule()->n_points());

  computeIncrementalDeformationGradient(_Fhat);
}

////////////////////////////////////////////////////////////////////////
}
