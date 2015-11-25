/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsPlasticMohrCoulombMulti.h"

// Following is for perturbing eigvenvalues.  This looks really bodgy, but works quite well!
#include "MooseRandom.h"

template<>
InputParameters validParams<TensorMechanicsPlasticMohrCoulombMulti>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addClassDescription("Non-associative Mohr-Coulomb plasticity with hardening/softening");
  params.addRequiredParam<UserObjectName>("cohesion", "A TensorMechanicsHardening UserObject that defines hardening of the cohesion");
  params.addRequiredParam<UserObjectName>("friction_angle", "A TensorMechanicsHardening UserObject that defines hardening of the friction angle (in radians)");
  params.addRequiredParam<UserObjectName>("dilation_angle", "A TensorMechanicsHardening UserObject that defines hardening of the dilation angle (in radians)");
  params.addParam<Real>("shift", "Yield surface is shifted by this amount to avoid problems with defining derivatives when eigenvalues are equal.  If this is larger than f_tol, a warning will be issued.  Default = f_tol.");

  return params;
}

TensorMechanicsPlasticMohrCoulombMulti::TensorMechanicsPlasticMohrCoulombMulti(const InputParameters & parameters) :
    TensorMechanicsPlasticModel(parameters),
    _cohesion(getUserObject<TensorMechanicsHardeningModel>("cohesion")),
    _phi(getUserObject<TensorMechanicsHardeningModel>("friction_angle")),
    _psi(getUserObject<TensorMechanicsHardeningModel>("dilation_angle")),
    _shift(parameters.isParamValid("shift") ? getParam<Real>("shift") : _f_tol)
{
  if (_shift < 0)
    mooseError("Value of 'shift' in TensorMechanicsPlasticMohrCoulombMulti must not be negative\n");
  if (_shift > _f_tol)
    _console << "WARNING: value of 'shift' in TensorMechanicsPlasticMohrCoulombMulti is probably set too high\n";
  MooseRandom::seed(0);
}

unsigned int
TensorMechanicsPlasticMohrCoulombMulti::numberSurfaces() const
{
  return 6;
}

void
TensorMechanicsPlasticMohrCoulombMulti::yieldFunctionV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & f) const
{
  std::vector<Real> eigvals;
  stress.symmetricEigenvalues(eigvals);
  eigvals[0] += _shift;
  eigvals[2] -= _shift;

  Real sinphi = std::sin(phi(intnl));
  Real cosphi = std::cos(phi(intnl));
  Real cohcos = cohesion(intnl)*cosphi;

  // Naively it seems a shame to have 6 yield functions active instead of just
  // 3.  But 3 won't do.  Eg, think of a loading with eigvals[0]=eigvals[1]=eigvals[2]
  // Then to return to the yield surface would require 2 positive plastic multipliers
  // and one negative one.  Boo hoo.

  f.resize(6);
  f[0] = 0.5*(eigvals[0] - eigvals[1]) + 0.5*(eigvals[0] + eigvals[1])*sinphi - cohcos;
  f[1] = 0.5*(eigvals[1] - eigvals[0]) + 0.5*(eigvals[0] + eigvals[1])*sinphi - cohcos;
  f[2] = 0.5*(eigvals[0] - eigvals[2]) + 0.5*(eigvals[0] + eigvals[2])*sinphi - cohcos;
  f[3] = 0.5*(eigvals[2] - eigvals[0]) + 0.5*(eigvals[0] + eigvals[2])*sinphi - cohcos;
  f[4] = 0.5*(eigvals[1] - eigvals[2]) + 0.5*(eigvals[1] + eigvals[2])*sinphi - cohcos;
  f[5] = 0.5*(eigvals[2] - eigvals[1]) + 0.5*(eigvals[1] + eigvals[2])*sinphi - cohcos;
}

void
TensorMechanicsPlasticMohrCoulombMulti::perturbStress(const RankTwoTensor & stress, std::vector<Real> & eigvals, std::vector<RankTwoTensor> & deigvals) const
{
  Real small_perturbation;
  RankTwoTensor shifted_stress = stress;
  while (eigvals[0] > eigvals[1] - 0.1*_shift || eigvals[1] > eigvals[2] - 0.1*_shift)
  {
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j <= i; ++j)
      {
        small_perturbation = 0.1*_shift*2*(MooseRandom::rand() - 0.5);
        shifted_stress(i, j) += small_perturbation;
        shifted_stress(j, i) += small_perturbation;
      }
    shifted_stress.dsymmetricEigenvalues(eigvals, deigvals);
  }
}


void
TensorMechanicsPlasticMohrCoulombMulti::df_dsig(const RankTwoTensor & stress, const Real & sin_angle, std::vector<RankTwoTensor> & df) const
{
  std::vector<Real> eigvals;
  std::vector<RankTwoTensor> deigvals;
  stress.dsymmetricEigenvalues(eigvals, deigvals);

  if (eigvals[0] > eigvals[1] - 0.1*_shift || eigvals[1] > eigvals[2] - 0.1*_shift)
    perturbStress(stress, eigvals, deigvals);

  df.resize(6);
  df[0] = 0.5*(deigvals[0] - deigvals[1]) + 0.5*(deigvals[0] + deigvals[1])*sin_angle;
  df[1] = 0.5*(deigvals[1] - deigvals[0]) + 0.5*(deigvals[0] + deigvals[1])*sin_angle;
  df[2] = 0.5*(deigvals[0] - deigvals[2]) + 0.5*(deigvals[0] + deigvals[2])*sin_angle;
  df[3] = 0.5*(deigvals[2] - deigvals[0]) + 0.5*(deigvals[0] + deigvals[2])*sin_angle;
  df[4] = 0.5*(deigvals[1] - deigvals[2]) + 0.5*(deigvals[1] + deigvals[2])*sin_angle;
  df[5] = 0.5*(deigvals[2] - deigvals[1]) + 0.5*(deigvals[1] + deigvals[2])*sin_angle;
}

void
TensorMechanicsPlasticMohrCoulombMulti::dyieldFunction_dstressV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & df_dstress) const
{
  Real sinphi = std::sin(phi(intnl));
  df_dsig(stress, sinphi, df_dstress);
}


void
TensorMechanicsPlasticMohrCoulombMulti::dyieldFunction_dintnlV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & df_dintnl) const
{
  std::vector<Real> eigvals;
  stress.symmetricEigenvalues(eigvals);
  eigvals[0] += _shift;
  eigvals[2] -= _shift;

  Real sin_angle = std::sin(phi(intnl));
  Real cos_angle = std::cos(phi(intnl));
  Real dsin_angle = cos_angle*dphi(intnl);
  Real dcos_angle = -sin_angle*dphi(intnl);
  Real dcohcos = dcohesion(intnl)*cos_angle + cohesion(intnl)*dcos_angle;

  df_dintnl.resize(6);
  df_dintnl[0] = df_dintnl[1] = 0.5*(eigvals[0] + eigvals[1])*dsin_angle - dcohcos;
  df_dintnl[2] = df_dintnl[3] = 0.5*(eigvals[0] + eigvals[2])*dsin_angle - dcohcos;
  df_dintnl[4] = df_dintnl[5] = 0.5*(eigvals[1] + eigvals[2])*dsin_angle - dcohcos;
}

void
TensorMechanicsPlasticMohrCoulombMulti::flowPotentialV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & r) const
{
  Real sinpsi = std::sin(psi(intnl));
  df_dsig(stress, sinpsi, r);
}

void
TensorMechanicsPlasticMohrCoulombMulti::dflowPotential_dstressV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankFourTensor> & dr_dstress) const
{
  std::vector<RankFourTensor> d2eigvals;
  stress.d2symmetricEigenvalues(d2eigvals);

  Real sinpsi = std::sin(psi(intnl));

  dr_dstress.resize(6);
  dr_dstress[0] = 0.5*(d2eigvals[0] - d2eigvals[1]) + 0.5*(d2eigvals[0] + d2eigvals[1])*sinpsi;
  dr_dstress[1] = 0.5*(d2eigvals[1] - d2eigvals[0]) + 0.5*(d2eigvals[0] + d2eigvals[1])*sinpsi;
  dr_dstress[2] = 0.5*(d2eigvals[0] - d2eigvals[2]) + 0.5*(d2eigvals[0] + d2eigvals[2])*sinpsi;
  dr_dstress[3] = 0.5*(d2eigvals[2] - d2eigvals[0]) + 0.5*(d2eigvals[0] + d2eigvals[2])*sinpsi;
  dr_dstress[4] = 0.5*(d2eigvals[1] - d2eigvals[2]) + 0.5*(d2eigvals[1] + d2eigvals[2])*sinpsi;
  dr_dstress[5] = 0.5*(d2eigvals[2] - d2eigvals[1]) + 0.5*(d2eigvals[1] + d2eigvals[2])*sinpsi;
}


void
TensorMechanicsPlasticMohrCoulombMulti::dflowPotential_dintnlV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & dr_dintnl) const
{
  Real cos_angle = std::cos(psi(intnl));
  Real dsin_angle = cos_angle*dpsi(intnl);

  std::vector<Real> eigvals;
  std::vector<RankTwoTensor> deigvals;
  stress.dsymmetricEigenvalues(eigvals, deigvals);

  if (eigvals[0] > eigvals[1] - 0.1*_shift || eigvals[1] > eigvals[2] - 0.1*_shift)
    perturbStress(stress, eigvals, deigvals);

  dr_dintnl.resize(6);
  dr_dintnl[0] = dr_dintnl[1] = 0.5*(deigvals[0] + deigvals[1])*dsin_angle;
  dr_dintnl[2] = dr_dintnl[3] = 0.5*(deigvals[0] + deigvals[2])*dsin_angle;
  dr_dintnl[4] = dr_dintnl[5] = 0.5*(deigvals[1] + deigvals[2])*dsin_angle;
}


void
TensorMechanicsPlasticMohrCoulombMulti::activeConstraints(const std::vector<Real> & f, const RankTwoTensor & stress, const Real & intnl, const RankFourTensor & Eijkl, std::vector<bool> & act, RankTwoTensor & returned_stress) const
{
  act.assign(6, false);

  if (f[0] <= _f_tol && f[1] <= _f_tol && f[2] <= _f_tol && f[3] <= _f_tol && f[4] <= _f_tol && f[5] <= _f_tol)
  {
    returned_stress = stress;
    return;
  }


  returned_stress = RankTwoTensor();

  std::vector<Real> eigvals;
  RankTwoTensor eigvecs;
  stress.symmetricEigenvaluesEigenvectors(eigvals, eigvecs);
  eigvals[0] += _shift;
  eigvals[2] -= _shift;

  Real sinphi = std::sin(phi(intnl));
  Real cosphi = std::cos(phi(intnl));
  Real coh = cohesion(intnl);
  Real cohcot = coh*cosphi/sinphi;

  std::vector<Real> v(3);
  v[0] = eigvals[0] - cohcot;
  v[1] = eigvals[1] - cohcot;
  v[2] = eigvals[2] - cohcot;

  // naively there are lots of different combinations
  // to try.  Eg, there are 6 yield surfaces, and so
  // for returning-to-the-tip, there are 6*5*4 possible
  // combinations of 3 yield functions that we could try.
  // However, with the constraint v0<=v1<=v2, and symmetry,
  // the following combinations of act are checked:
  // 110100 (tip) (about 43%)
  // 010101 (tip) (about 1%)
  // 010100 (edge) (about 22%)
  // 000101 (edge) (about 22%)
  // 000100 (plane) (about 12%)
  // Some other "tip" combinations are tried by
  // FiniteStrainMultiPlasticity, in about 0.001% of cases,
  // but these are not work checking.

  // these are the normals to the 6 yield surfaces
  std::vector<std::vector<Real> > norm(6);
  Real sinpsi = std::sin(psi(intnl));
  Real oneminus = 1 - sinpsi;
  Real oneplus = 1 + sinpsi;
  norm[0].resize(3);
  norm[0][0] = oneplus; norm[0][1] = -oneminus; norm[0][2] = 0;
  norm[1].resize(3);
  norm[1][0] = -oneminus; norm[1][1] = oneplus; norm[1][2] = 0;
  norm[2].resize(3);
  norm[2][0] = oneplus; norm[2][1] = 0; norm[2][2] = -oneminus;
  norm[3].resize(3);
  norm[3][0] = -oneminus; norm[3][1] = 0; norm[3][2] = oneplus;
  norm[4].resize(3);
  norm[4][0] = 0; norm[4][1] = oneplus; norm[4][2] = -oneminus;
  norm[5].resize(3);
  norm[5][0] = 0; norm[5][1] = -oneminus; norm[5][2] = oneplus;

  // the flow directions are these norm multiplied by Eijkl.
  // I call the flow directions "n".
  // In the following I assume that the Eijkl is
  // for an isotropic situation.  Then I don't have to
  // rotate to the principal-stress frame, and i don't
  // have to worry about strange off-diagonal things
  std::vector<std::vector<Real> > n(6);
  for (unsigned ys = 0; ys < 6; ++ys)
  {
    n[ys].assign(3, 0);
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j)
        n[ys][i] += Eijkl(i,i,j,j)*norm[ys][j];
  }


  // Check for return to the tip.
  // For tip-return to satisfy Kuhn-Tucker we need
  // v = a*n[a] + b*n[b] + c*n[c]
  // with a, b, and c all being non-negative (they are
  // the plasticity multipliers)

  Real denom;

  // 110100 (tip)
  denom = triple(n[0], n[1], n[3]);
  if (triple(v, n[0], n[1])/denom >= 0 && triple(v, n[1], n[3])/denom >= 0 && triple(v, n[3], n[0])/denom >= 0)
  {
    act[0] = act[1] = act[3] = true;
    returned_stress(0, 0) = returned_stress(1, 1) = returned_stress(2, 2) = cohcot;
    returned_stress = eigvecs*returned_stress*(eigvecs.transpose());
    return;
  }

  // 010101 (tip)
  denom = triple(n[1], n[3], n[5]);
  if (triple(v, n[1], n[3])/denom >= 0 && triple(v, n[3], n[5])/denom >= 0 && triple(v, n[5], n[1])/denom >= 0)
  {
    act[1] = act[3] = act[5] = true;
    returned_stress(0, 0) = returned_stress(1, 1) = returned_stress(2, 2) = cohcot;
    returned_stress = eigvecs*returned_stress*(eigvecs.transpose());
    return;
  }


  // the following are tangents to the 1, 3, and 5 yield surfaces
  // used below.
  std::vector<Real> n1xn3(3);
  cross(n[1], n[3], n1xn3);
  std::vector<Real> n1xn5(3);
  cross(n[1], n[5], n1xn5);
  std::vector<Real> n3xn5(3);
  cross(n[3], n[5], n3xn5);



  // 010100 (edge)
  // work out the point to which we would return, "a".  It is defined by
  // f1 = 0 = f3, and that (p - a).(n1 x n3) = 0, where "p" is the
  // starting position (p = eigvals).
  // In the following a = (lam0, lam1, lam1)
  Real pdotn1xn3 = dot(eigvals, n1xn3);
  //Real lam0 = pdotn1xn3 - 4*coh*cosphi*oneminus/(1 + sinphi);
  Real lam0 = pdotn1xn3 - 2*coh*cosphi*(n1xn3[1] + n1xn3[2])/(1 + sinphi);
  //lam0 /= oneplus + 2*oneminus*(1 - sinphi)/(1 + sinphi);
  lam0 /= n1xn3[0] + (n1xn3[1] + n1xn3[2])*(1 - sinphi)/(1 + sinphi);
  Real lam1 = lam0*(1 - sinphi)/(1 + sinphi) + 2*coh*cosphi/(1 + sinphi);
  std::vector<Real> pminusa(3);
  pminusa[0] = eigvals[0] - lam0;
  pminusa[1] = eigvals[1] - lam1;
  pminusa[2] = eigvals[2] - lam1;
  // p = a + alpha*n1 + beta*n3, and for Kuhn-Tucker we need alpha and beta non-negative
  if (dot(pminusa, n3xn5)/dot(n[1], n3xn5) >= 0 && dot(pminusa, n1xn5)/dot(n[3], n1xn5) >= 0)
  {
    returned_stress(0, 0) = lam0;
    returned_stress(1, 1) = returned_stress(2, 2) = lam1;
    returned_stress = eigvecs*returned_stress*(eigvecs.transpose());
    act[1] = act[3] = true;
    return;
  }

  // 000101 (edge)
  // work out the point to which we would return, "a".  It is defined by
  // f3 = 0 = f5, and that (p - a).(n3 x n5) = 0, where "p" is the
  // starting position (p = eigvals).
  // In the following a = (lam1, lam1, lam2)
  Real pdotn3xn5 = dot(eigvals, n3xn5);
  Real lam2 = pdotn3xn5 + 2*coh*cosphi*(n3xn5[0] + n3xn5[1])/(1 - sinphi);
  lam2 /= n3xn5[2] + (n3xn5[0] + n3xn5[1])*(1 + sinphi)/(1 - sinphi);
  lam1 = lam2*(1 + sinphi)/(1 - sinphi) - 2*coh*cosphi/(1 - sinphi);
  pminusa[0] = eigvals[0] - lam1;
  pminusa[1] = eigvals[1] - lam1;
  pminusa[2] = eigvals[2] - lam2;
  // p = a + alpha*n3 + beta*n5, and for Kuhn-Tucker we need alpha and beta non-negative
  if (dot(pminusa, n1xn5)/dot(n[3], n1xn5) >= 0 && dot(pminusa, n1xn3)/dot(n[5], n1xn3) >= 0)
  {
    returned_stress(0, 0) = returned_stress(1, 1) = lam1;
    returned_stress(2, 2) = lam2;
    returned_stress = eigvecs*returned_stress*(eigvecs.transpose());
    act[3] = act[5] = true;
    return;
  }

  // 000100 (plane)
  // the returned point, "a" is defined by f3=0 and
  // a = p - alpha*n3
  Real alpha = 0.5*eigvals[2]*(1 + sinphi) + 0.5*eigvals[0]*(-1 + sinphi) - coh*cosphi;
  alpha /= 0.5*n[3][2]*(1 + sinphi) + 0.5*n[3][0]*(-1 + sinphi);
  returned_stress(0, 0) = eigvals[0] - alpha*n[3][0];
  returned_stress(1, 1) = eigvals[1] - alpha*n[3][1];
  returned_stress(2, 2) = eigvals[2] - alpha*n[3][2];
  returned_stress = eigvecs*returned_stress*(eigvecs.transpose());
  act[3] = true;
  return;
}

Real
TensorMechanicsPlasticMohrCoulombMulti::dot(const std::vector<Real> & a, const std::vector<Real> & b) const
{
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void
TensorMechanicsPlasticMohrCoulombMulti::cross(const std::vector<Real> & a, const std::vector<Real> & b, std::vector<Real> & c) const
{
  c.resize(3);
  c[0] = a[1]*b[2] - a[2]*b[1];
  c[1] = a[2]*b[0] - a[0]*b[2];
  c[2] = a[0]*b[1] - a[1]*b[0];
}


Real
TensorMechanicsPlasticMohrCoulombMulti::triple(const std::vector<Real> & a, const std::vector<Real> & b, const std::vector<Real> & c) const
{
  return a[0]*(b[1]*c[2] - b[2]*c[1]) - a[1]*(b[0]*c[2] - b[2]*c[0]) + a[2]*(b[0]*c[1] - b[1]*c[0]);
}

Real
TensorMechanicsPlasticMohrCoulombMulti::cohesion(const Real internal_param) const
{
  return _cohesion.value(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombMulti::dcohesion(const Real internal_param) const
{
  return _cohesion.derivative(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombMulti::phi(const Real internal_param) const
{
  return _phi.value(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombMulti::dphi(const Real internal_param) const
{
  return _phi.derivative(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombMulti::psi(const Real internal_param) const
{
  return _psi.value(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombMulti::dpsi(const Real internal_param) const
{
  return _psi.derivative(internal_param);
}

std::string
TensorMechanicsPlasticMohrCoulombMulti::modelName() const
{
  return "MohrCoulombMulti";
}

