#include "FiniteStrainTensile.h"
#include <math.h> // for M_PI

template<>
InputParameters validParams<FiniteStrainTensile>()
{
  InputParameters params = validParams<FiniteStrainPlasticBase>();
  params.addRequiredRangeCheckedParam<Real>("tensile_strength", "tensile_strength>=0", "Tensile strength");
  params.addParam<Real>("tensile_strength_residual", "Tensile strength at infinite hardening.  If not given, this defaults to tensile_strength, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("tensile_strength_rate", 0, "tensile_strength_rate>=0", "Tensile strength = tensile_strength_residual + (tensile_strength - tensile_strength_residual)*exp(-tensile_strength_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRangeCheckedParam<Real>("tensile_edge_smoother", 25.0, "tensile_edge_smoother>=0 & tensile_edge_smoother<=30", "Smoothing parameter: the edges of the cone are smoothed by the given amount.");
  params.addRequiredRangeCheckedParam<Real>("tensile_tip_smoother", "tensile_tip_smoother>=0", "Smoothing parameter: the cone vertex at mean = Tensile strength, will be smoothed by the given amount.  Typical value is 0.1*tensile_strength");
  params.addClassDescription("Non-associative tensile plasticity with hardening/softening");

  return params;
}

FiniteStrainTensile::FiniteStrainTensile(const std::string & name,
                                                         InputParameters parameters) :
    FiniteStrainPlasticBase(name, parameters),
    _tensile_strength0(getParam<Real>("tensile_strength")),
    _tensile_strength_residual(parameters.isParamValid("tensile_strength_residual") ? getParam<Real>("tensile_strength_residual") : _tensile_strength0),
    _tensile_strength_rate(getParam<Real>("tensile_strength_rate")),
    _small_smoother2(std::pow(getParam<Real>("tensile_tip_smoother"), 2)),
    _tt(getParam<Real>("tensile_edge_smoother")*M_PI/180.0),
    _sin3tt(std::sin(3*_tt)),

    _tensile_internal(declareProperty<Real>("tensile_internal")),
    _tensile_internal_old(declarePropertyOld<Real>("tensile_internal")),
    _tensile_max_principal(declareProperty<Real>("tensile_max_principal_stress")),
    _yf(declareProperty<Real>("tensile_yield_function"))
{
  _ccc = (-std::cos(3*_tt)*(std::cos(_tt) - std::sin(_tt)/std::sqrt(3.0)) - 3*std::sin(3*_tt)*(std::sin(_tt) + std::cos(_tt)/std::sqrt(3.0)))/(18*std::pow(std::cos(3*_tt), 3));
  _bbb = (std::sin(6*_tt)*(std::cos(_tt) - std::sin(_tt)/std::sqrt(3.0)) - 6*std::cos(6*_tt)*(std::sin(_tt) + std::cos(_tt)/std::sqrt(3.0)))/(18*std::pow(std::cos(3*_tt), 3));
  _aaa = -std::sin(_tt)/std::sqrt(3.0) - _bbb*std::sin(3*_tt) - _ccc*std::pow(std::sin(3*_tt), 2) + std::cos(_tt);
}

void FiniteStrainTensile::initQpStatefulProperties()
{
  _tensile_internal[_qp] = 0;
  _tensile_internal_old[_qp] = 0;
  _tensile_max_principal[_qp] = 0;
  _yf[_qp] = 0.0;
  FiniteStrainPlasticBase::initQpStatefulProperties();
}


void
FiniteStrainTensile::postReturnMap()
{
  // Record the value of the yield function
  _yf[_qp] = _f[_qp][0];

  // Record the value of the internal parameter
  _tensile_internal[_qp] = _intnl[_qp][0];

  // Record the maximum principal stress
  std::vector<Real> eigvals;
  _stress[_qp].symmetricEigenvalues(eigvals);
  _tensile_max_principal[_qp] = eigvals[2];
}



unsigned int
FiniteStrainTensile::numberOfInternalParameters()
{
  return 1;
}

void
FiniteStrainTensile::yieldFunction(const RankTwoTensor &stress, const std::vector<Real> & intnl, std::vector<Real> & f)
{
  Real mean_stress = stress.trace()/3.0;
  Real sin3Lode = stress.sin3Lode();
  if (sin3Lode <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    stress.symmetricEigenvalues(eigvals);
    f.assign(1, mean_stress + std::sqrt(_small_smoother2 + std::pow(eigvals[2] - mean_stress, 2)) - tensile_strength(intnl[0]));
  }
  else
  {
    // the edge-smoothed version
    Real kk = _aaa + _bbb*sin3Lode + _ccc*std::pow(sin3Lode, 2);
    Real sibar2 = stress.secondInvariant();
    f.assign(1, mean_stress + std::sqrt(_small_smoother2 + sibar2*std::pow(kk, 2)) - tensile_strength(intnl[0]));
  }
}

void
FiniteStrainTensile::dyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & /*intnl*/, std::vector<RankTwoTensor> & df_dstress)
{
  Real mean_stress = stress.trace()/3.0;
  RankTwoTensor dmean_stress = stress.dtrace()/3.0;
  Real sin3Lode = stress.sin3Lode();
  if (sin3Lode <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    std::vector<RankTwoTensor> deigvals;
    stress.dsymmetricEigenvalues(eigvals, deigvals);
    Real denom = std::sqrt(_small_smoother2 + std::pow(eigvals[2] - mean_stress, 2));
    df_dstress.assign(1, dmean_stress + (eigvals[2] - mean_stress)*(deigvals[2] - dmean_stress)/denom);
  }
  else
  {
    // the edge-smoothed version
    Real kk = _aaa + _bbb*sin3Lode + _ccc*std::pow(sin3Lode, 2);
    RankTwoTensor dkk = (_bbb + 2*_ccc*sin3Lode)*stress.dsin3Lode();
    Real sibar2 = stress.secondInvariant();
    RankTwoTensor dsibar2 = stress.dsecondInvariant();
    Real denom = std::sqrt(_small_smoother2 + sibar2*std::pow(kk, 2));
    df_dstress.assign(1, dmean_stress + (0.5*dsibar2*std::pow(kk, 2) + sibar2*kk*dkk)/denom);
  }
}

void
FiniteStrainTensile::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & df_dintnl)
{
  df_dintnl.resize(1);
  df_dintnl[0].assign(1, - dtensile_strength(intnl[0]));
}

void
FiniteStrainTensile::flowPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & r)
{
  // This plasticity is associative so
  dyieldFunction_dstress(stress, intnl, r);
}

void
FiniteStrainTensile::dflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & /*intnl*/, std::vector<RankFourTensor> & dr_dstress)
{
  Real mean_stress = stress.trace()/3.0;
  RankTwoTensor dmean_stress = stress.dtrace()/3.0;
  Real sin3Lode = stress.sin3Lode();
  if (sin3Lode <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    std::vector<RankTwoTensor> deigvals;
    std::vector<RankFourTensor> d2eigvals;
    stress.dsymmetricEigenvalues(eigvals, deigvals);
    stress.d2symmetricEigenvalues(d2eigvals);

    Real denom = std::sqrt(_small_smoother2 + std::pow(eigvals[2] - mean_stress, 2));

    dr_dstress.assign(1, (eigvals[2] - mean_stress)*d2eigvals[2]/denom);
    for (unsigned i = 0 ; i < 3 ; ++i)
      for (unsigned j = 0 ; j < 3 ; ++j)
        for (unsigned k = 0 ; k < 3 ; ++k)
          for (unsigned l = 0 ; l < 3 ; ++l)
            dr_dstress[0](i, j, k, l) += (1 - std::pow((eigvals[2] - mean_stress)/denom, 2))*(deigvals[2](i, j) - dmean_stress(i, j))*(deigvals[2](k, l) - dmean_stress(k, l))/denom;
  }
  else
  {
    // the edge-smoothed version
    RankTwoTensor dsin3Lode = stress.dsin3Lode();
    Real kk = _aaa + _bbb*sin3Lode + _ccc*std::pow(sin3Lode, 2);
    RankTwoTensor dkk = (_bbb + 2*_ccc*sin3Lode)*dsin3Lode;
    RankFourTensor d2kk = (_bbb + 2*_ccc*sin3Lode)*stress.d2sin3Lode();
    for (unsigned i = 0 ; i < 3 ; ++i)
      for (unsigned j = 0 ; j < 3 ; ++j)
        for (unsigned k = 0 ; k < 3 ; ++k)
          for (unsigned l = 0 ; l < 3 ; ++l)
            d2kk(i, j, k, l) += 2*_ccc*dsin3Lode(i, j)*dsin3Lode(k, l);

    Real sibar2 = stress.secondInvariant();
    RankTwoTensor dsibar2 = stress.dsecondInvariant();
    RankFourTensor d2sibar2 = stress.d2secondInvariant();

    Real denom = std::sqrt(_small_smoother2 + sibar2*std::pow(kk, 2));
    dr_dstress.assign(1, (0.5*d2sibar2*std::pow(kk, 2) + sibar2*kk*d2kk)/denom);
    for (unsigned i = 0 ; i < 3 ; ++i)
      for (unsigned j = 0 ; j < 3 ; ++j)
        for (unsigned k = 0 ; k < 3 ; ++k)
          for (unsigned l = 0 ; l < 3 ; ++l)
          {
            dr_dstress[0](i, j, k, l) += (dsibar2(i, j)*dkk(k, l)*kk + dkk(i, j)*dsibar2(k, l)*kk + sibar2*dkk(i, j)*dkk(k, l))/denom;
            dr_dstress[0](i, j, k, l) -= (0.5*dsibar2(i, j)*std::pow(kk, 2) + sibar2*kk*dkk(i, j))*(0.5*dsibar2(k, l)*std::pow(kk, 2) + sibar2*kk*dkk(k, l))/std::pow(denom, 3);
          }
  }

}

void
FiniteStrainTensile::dflowPotential_dintnl(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<RankTwoTensor> > & dr_dintnl)
{
  dr_dintnl.resize(1);
  dr_dintnl[0].assign(1, RankTwoTensor());
}

void
FiniteStrainTensile::hardPotential(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<Real> > & h)
{
  h.resize(1);
  h[0].assign(1, -1.0);
}

void
FiniteStrainTensile::dhardPotential_dstress(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<RankTwoTensor> > & dh_dstress)
{
  dh_dstress.resize(1);
  dh_dstress[0].assign(1, RankTwoTensor());
}

void
FiniteStrainTensile::dhardPotential_dintnl(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<std::vector<Real> > > & dh_dintnl)
{
  dh_dintnl.resize(1);
  dh_dintnl[0].resize(1);
  dh_dintnl[0][0].assign(1, 0.0);
}


Real
FiniteStrainTensile::tensile_strength(const Real internal_param)
{
  return _tensile_strength_residual + (_tensile_strength0 - _tensile_strength_residual)*std::exp(-_tensile_strength_rate*internal_param);
}

Real
FiniteStrainTensile::dtensile_strength(const Real internal_param)
{
  return -_tensile_strength_rate*(_tensile_strength0 - _tensile_strength_residual)*std::exp(-_tensile_strength_rate*internal_param);
}
