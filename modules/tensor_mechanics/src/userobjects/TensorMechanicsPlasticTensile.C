#include "TensorMechanicsPlasticTensile.h"
#include <math.h> // for M_PI

template<>
InputParameters validParams<TensorMechanicsPlasticTensile>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addRequiredRangeCheckedParam<Real>("tensile_strength", "tensile_strength>=0", "Tensile strength");
  params.addParam<Real>("tensile_strength_residual", "Tensile strength at infinite hardening.  If not given, this defaults to tensile_strength, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("tensile_strength_rate", 0, "tensile_strength_rate>=0", "Tensile strength = tensile_strength_residual + (tensile_strength - tensile_strength_residual)*exp(-tensile_strength_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRangeCheckedParam<Real>("tensile_edge_smoother", 25.0, "tensile_edge_smoother>=0 & tensile_edge_smoother<=30", "Smoothing parameter: the edges of the cone are smoothed by the given amount.");
  params.addRequiredRangeCheckedParam<Real>("tensile_tip_smoother", "tensile_tip_smoother>=0", "Smoothing parameter: the cone vertex at mean = Tensile strength, will be smoothed by the given amount.  Typical value is 0.1*tensile_strength");
  params.addParam<Real>("tensile_lode_cutoff", "If the second invariant of stress is less than this amount, the Lode angle is assumed to be zero.  This is to gaurd against precision-loss problems, and this parameter should be set small.  Default = 0.00001*((yield_Function_tolerance)^2)");
  params.addClassDescription("Non-associative tensile plasticity with hardening/softening");

  return params;
}

TensorMechanicsPlasticTensile::TensorMechanicsPlasticTensile(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters),
    _tensile_strength0(getParam<Real>("tensile_strength")),
    _tensile_strength_residual(parameters.isParamValid("tensile_strength_residual") ? getParam<Real>("tensile_strength_residual") : _tensile_strength0),
    _tensile_strength_rate(getParam<Real>("tensile_strength_rate")),
    _small_smoother2(std::pow(getParam<Real>("tensile_tip_smoother"), 2)),
    _tt(getParam<Real>("tensile_edge_smoother")*M_PI/180.0),
    _sin3tt(std::sin(3*_tt)),
    _lode_cutoff(parameters.isParamValid("tensile_lode_cutoff") ? getParam<Real>("tensile_lode_cutoff") : 1.0E-5*std::pow(_f_tol, 2))

{
  if (_lode_cutoff < 0)
    mooseError("tensile_lode_cutoff must not be negative");
  _ccc = (-std::cos(3*_tt)*(std::cos(_tt) - std::sin(_tt)/std::sqrt(3.0)) - 3*std::sin(3*_tt)*(std::sin(_tt) + std::cos(_tt)/std::sqrt(3.0)))/(18*std::pow(std::cos(3*_tt), 3));
  _bbb = (std::sin(6*_tt)*(std::cos(_tt) - std::sin(_tt)/std::sqrt(3.0)) - 6*std::cos(6*_tt)*(std::sin(_tt) + std::cos(_tt)/std::sqrt(3.0)))/(18*std::pow(std::cos(3*_tt), 3));
  _aaa = -std::sin(_tt)/std::sqrt(3.0) - _bbb*std::sin(3*_tt) - _ccc*std::pow(std::sin(3*_tt), 2) + std::cos(_tt);
}


Real
TensorMechanicsPlasticTensile::yieldFunction(const RankTwoTensor & stress, const Real & intnl) const
{
  Real mean_stress = stress.trace()/3.0;
  Real sin3Lode = stress.sin3Lode(_lode_cutoff, 0);
  if (sin3Lode <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    stress.symmetricEigenvalues(eigvals);
    return mean_stress + std::sqrt(_small_smoother2 + std::pow(eigvals[2] - mean_stress, 2)) - tensile_strength(intnl);
  }
  else
  {
    // the edge-smoothed version
    Real kk = _aaa + _bbb*sin3Lode + _ccc*std::pow(sin3Lode, 2);
    Real sibar2 = stress.secondInvariant();
    return mean_stress + std::sqrt(_small_smoother2 + sibar2*std::pow(kk, 2)) - tensile_strength(intnl);
  }
}


RankTwoTensor
TensorMechanicsPlasticTensile::dyieldFunction_dstress(const RankTwoTensor & stress, const Real & /*intnl*/) const
{
  Real mean_stress = stress.trace()/3.0;
  RankTwoTensor dmean_stress = stress.dtrace()/3.0;
  Real sin3Lode = stress.sin3Lode(_lode_cutoff, 0);
  if (sin3Lode <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    std::vector<RankTwoTensor> deigvals;
    stress.dsymmetricEigenvalues(eigvals, deigvals);
    Real denom = std::sqrt(_small_smoother2 + std::pow(eigvals[2] - mean_stress, 2));
    return dmean_stress + (eigvals[2] - mean_stress)*(deigvals[2] - dmean_stress)/denom;
  }
  else
  {
    // the edge-smoothed version
    Real kk = _aaa + _bbb*sin3Lode + _ccc*std::pow(sin3Lode, 2);
    RankTwoTensor dkk = (_bbb + 2*_ccc*sin3Lode)*stress.dsin3Lode(_lode_cutoff);
    Real sibar2 = stress.secondInvariant();
    RankTwoTensor dsibar2 = stress.dsecondInvariant();
    Real denom = std::sqrt(_small_smoother2 + sibar2*std::pow(kk, 2));
    return dmean_stress + (0.5*dsibar2*std::pow(kk, 2) + sibar2*kk*dkk)/denom;
  }
}


Real
TensorMechanicsPlasticTensile::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/, const Real & intnl) const
{
  return -dtensile_strength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticTensile::flowPotential(const RankTwoTensor & stress, const Real & intnl) const
{
  // This plasticity is associative so
  return dyieldFunction_dstress(stress, intnl);
}

RankFourTensor
TensorMechanicsPlasticTensile::dflowPotential_dstress(const RankTwoTensor & stress, const Real & /*intnl*/) const
{
  Real mean_stress = stress.trace()/3.0;
  RankTwoTensor dmean_stress = stress.dtrace()/3.0;
  Real sin3Lode = stress.sin3Lode(_lode_cutoff, 0);
  if (sin3Lode <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    std::vector<RankTwoTensor> deigvals;
    std::vector<RankFourTensor> d2eigvals;
    stress.dsymmetricEigenvalues(eigvals, deigvals);
    stress.d2symmetricEigenvalues(d2eigvals);

    Real denom = std::sqrt(_small_smoother2 + std::pow(eigvals[2] - mean_stress, 2));

    RankFourTensor dr_dstress = (eigvals[2] - mean_stress)*d2eigvals[2]/denom;
    for (unsigned i = 0 ; i < 3 ; ++i)
      for (unsigned j = 0 ; j < 3 ; ++j)
        for (unsigned k = 0 ; k < 3 ; ++k)
          for (unsigned l = 0 ; l < 3 ; ++l)
            dr_dstress(i, j, k, l) += (1 - std::pow((eigvals[2] - mean_stress)/denom, 2))*(deigvals[2](i, j) - dmean_stress(i, j))*(deigvals[2](k, l) - dmean_stress(k, l))/denom;
    return dr_dstress;
  }
  else
  {
    // the edge-smoothed version
    RankTwoTensor dsin3Lode = stress.dsin3Lode(_lode_cutoff);
    Real kk = _aaa + _bbb*sin3Lode + _ccc*std::pow(sin3Lode, 2);
    RankTwoTensor dkk = (_bbb + 2*_ccc*sin3Lode)*dsin3Lode;
    RankFourTensor d2kk = (_bbb + 2*_ccc*sin3Lode)*stress.d2sin3Lode(_lode_cutoff);
    for (unsigned i = 0 ; i < 3 ; ++i)
      for (unsigned j = 0 ; j < 3 ; ++j)
        for (unsigned k = 0 ; k < 3 ; ++k)
          for (unsigned l = 0 ; l < 3 ; ++l)
            d2kk(i, j, k, l) += 2*_ccc*dsin3Lode(i, j)*dsin3Lode(k, l);

    Real sibar2 = stress.secondInvariant();
    RankTwoTensor dsibar2 = stress.dsecondInvariant();
    RankFourTensor d2sibar2 = stress.d2secondInvariant();

    Real denom = std::sqrt(_small_smoother2 + sibar2*std::pow(kk, 2));
    RankFourTensor dr_dstress = (0.5*d2sibar2*std::pow(kk, 2) + sibar2*kk*d2kk)/denom;
    for (unsigned i = 0 ; i < 3 ; ++i)
      for (unsigned j = 0 ; j < 3 ; ++j)
        for (unsigned k = 0 ; k < 3 ; ++k)
          for (unsigned l = 0 ; l < 3 ; ++l)
          {
            dr_dstress(i, j, k, l) += (dsibar2(i, j)*dkk(k, l)*kk + dkk(i, j)*dsibar2(k, l)*kk + sibar2*dkk(i, j)*dkk(k, l))/denom;
            dr_dstress(i, j, k, l) -= (0.5*dsibar2(i, j)*std::pow(kk, 2) + sibar2*kk*dkk(i, j))*(0.5*dsibar2(k, l)*std::pow(kk, 2) + sibar2*kk*dkk(k, l))/std::pow(denom, 3);
          }
    return dr_dstress;
  }
}

RankTwoTensor
TensorMechanicsPlasticTensile::dflowPotential_dintnl(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}


Real
TensorMechanicsPlasticTensile::tensile_strength(const Real internal_param) const
{
  return _tensile_strength_residual + (_tensile_strength0 - _tensile_strength_residual)*std::exp(-_tensile_strength_rate*internal_param);
}

Real
TensorMechanicsPlasticTensile::dtensile_strength(const Real internal_param) const
{
  return -_tensile_strength_rate*(_tensile_strength0 - _tensile_strength_residual)*std::exp(-_tensile_strength_rate*internal_param);
}
