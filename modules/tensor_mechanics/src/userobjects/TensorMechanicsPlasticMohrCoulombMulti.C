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
  params.addParam<Real>("shift", "Yield surface is shifted by this amount to avoid problems with defining derivatives when eigenvalues are equal.  If this is larger than f_tol, a warning will be issued.  Default = 0.1*f_tol.");

  return params;
}

TensorMechanicsPlasticMohrCoulombMulti::TensorMechanicsPlasticMohrCoulombMulti(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters),
    _cohesion(getUserObject<TensorMechanicsHardeningModel>("cohesion")),
    _phi(getUserObject<TensorMechanicsHardeningModel>("friction_angle")),
    _psi(getUserObject<TensorMechanicsHardeningModel>("dilation_angle")),
    _shift(parameters.isParamValid("shift") ? getParam<Real>("shift") : 0.1*_f_tol)
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
    for (unsigned i = 0 ; i < 3 ; ++i)
      for (unsigned j = 0 ; j <= i ; ++j)
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
