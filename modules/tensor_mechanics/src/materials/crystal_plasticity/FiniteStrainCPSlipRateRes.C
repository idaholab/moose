//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FiniteStrainCPSlipRateRes.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", FiniteStrainCPSlipRateRes);

InputParameters
FiniteStrainCPSlipRateRes::validParams()
{
  InputParameters params = FiniteStrainCrystalPlasticity::validParams();
  return params;
}

FiniteStrainCPSlipRateRes::FiniteStrainCPSlipRateRes(const InputParameters & parameters)
  : FiniteStrainCrystalPlasticity(parameters),
    _resid(_nss),
    _slip_rate(_nss),
    _dsliprate_dgss(_nss),
    _jacob(_nss, _nss),
    _dsliprate_dsliprate(_nss, _nss)
{
}

void
FiniteStrainCPSlipRateRes::solveStatevar()
{
  preSolveStress();
  solveStress();
  if (_err_tol)
    return;
  postSolveStress();
}

void
FiniteStrainCPSlipRateRes::preSolveStress()
{
  FiniteStrainCrystalPlasticity::preSolveStress();
  _slip_rate.zero();
}

void
FiniteStrainCPSlipRateRes::solveStress()
{
  Real rnorm, rnorm0, rnorm_prev;
  unsigned int iter = 0;

#ifdef DEBUG
  std::vector<Real> rnormst(_maxiter + 1), slipratest(_maxiter + 1); // Use for Debugging
#endif

  calcResidJacobSlipRate();
  if (_err_tol)
    return;
  rnorm = calcResidNorm();
  rnorm0 = rnorm;

#ifdef DEBUG
  rnormst[iter] = rnorm;
  Real slipratemax = 0.0;
  for (unsigned int i = 0; i < _nss; ++i)
    if (std::abs(_slip_rate(i)) > slipratemax)
      slipratemax = std::abs(_slip_rate(i));
  slipratest[iter] = slipratemax;
#endif

  while (rnorm > _rtol * rnorm0 && rnorm0 > _abs_tol && iter < _maxiter)
  {
    calcUpdate();

    DenseVector<Real> update = _resid;

    _slip_rate -= update;

    calcResidualSlipRate();
    if (_err_tol)
      return;
    rnorm_prev = rnorm;
    rnorm = calcResidNorm();

    if (_use_line_search && rnorm > rnorm_prev && !lineSearchUpdateSlipRate(rnorm_prev, update))
    {
#ifdef DEBUG
      mooseWarning("FiniteStrainCrystalPLasticity: Failed with line search");
#endif
      _err_tol = true;
      return;
    }

    calcJacobianSlipRate();

    if (_use_line_search)
      rnorm = calcResidNorm();
    iter++;

#ifdef DEBUG
    slipratemax = 0.0;
    for (unsigned int i = 0; i < _nss; ++i)
      if (std::abs(_slip_rate(i)) > slipratemax)
        slipratemax = std::abs(_slip_rate(i));
    rnormst[iter] = rnorm;
    slipratest[iter] = slipratemax;
#endif
  }

  if (iter == _maxiter)
  {
#ifdef DEBUG
    mooseWarning("FiniteStrainCPSlipRateRes: NR exceeds maximum iteration ", iter, " ", rnorm);
#endif
    _err_tol = true;
    return;
  }
}

void
FiniteStrainCPSlipRateRes::calcResidJacobSlipRate()
{
  calcResidualSlipRate();
  if (_err_tol)
    return;
  calcJacobianSlipRate();
}

void
FiniteStrainCPSlipRateRes::calcResidualSlipRate()
{
  RankTwoTensor eqv_slip_incr, ce, ee;
  const RankTwoTensor iden(RankTwoTensor::initIdentity);

  _slip_incr = _slip_rate;
  _slip_incr *= _dt;

  for (unsigned int i = 0; i < _nss; ++i)
    eqv_slip_incr += _s0[i] * _slip_incr(i);

  eqv_slip_incr = iden - eqv_slip_incr;

  _fp_inv = _fp_old_inv * eqv_slip_incr;
  _fe = _dfgrd_tmp * _fp_inv;

  ce = _fe.transpose() * _fe;
  ee = ce - iden;
  ee *= 0.5;

  _pk2_tmp = _elasticity_tensor[_qp] * ee;

  for (unsigned int i = 0; i < _nss; ++i)
    _tau(i) = _pk2_tmp.doubleContraction(_s0[i]);

  update_slip_system_resistance();
  getSlipIncrements();

  if (_err_tol)
    return;

  for (unsigned int i = 0; i < _nss; ++i)
    _resid(i) = _slip_rate(i) - _slip_incr(i) / _dt;
}

void
FiniteStrainCPSlipRateRes::calcJacobianSlipRate()
{
  //_dsliprate_dsliprate not reinitialized to zero, hence order is important
  calcDtauDsliprate();
  calcDgssDsliprate();

  for (unsigned int i = 0; i < _nss; ++i)
    for (unsigned int j = 0; j < _nss; ++j)
    {
      _jacob(i, j) = 0.0;
      if (i == j)
        _jacob(i, j) += 1.0;
      _jacob(i, j) -= _dsliprate_dsliprate(i, j);
    }
}

void
FiniteStrainCPSlipRateRes::calcDtauDsliprate()
{
  RankFourTensor dfedfpinv, deedfe, dfpinvdpk2;
  std::vector<RankTwoTensor> dtaudpk2(_nss), dfpinvdsliprate(_nss);

  for (unsigned int i = 0; i < _nss; ++i)
  {
    dtaudpk2[i] = _s0[i];
    dfpinvdsliprate[i] = -_fp_old_inv * _s0[i] * _dt;
  }

  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      for (const auto k : make_range(Moose::dim))
        dfedfpinv(i, j, k, j) = _dfgrd_tmp(i, k);

  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      for (const auto k : make_range(Moose::dim))
      {
        deedfe(i, j, k, i) = deedfe(i, j, k, i) + _fe(k, j) * 0.5;
        deedfe(i, j, k, j) = deedfe(i, j, k, j) + _fe(k, i) * 0.5;
      }

  RankFourTensor dpk2dfpinv;

  dpk2dfpinv = _elasticity_tensor[_qp] * deedfe * dfedfpinv;

  for (unsigned int i = 0; i < _nss; ++i)
    for (unsigned int j = 0; j < _nss; ++j)
      _dsliprate_dsliprate(i, j) =
          _dslipdtau(i) * dtaudpk2[i].doubleContraction(dpk2dfpinv * dfpinvdsliprate[j]);
}

void
FiniteStrainCPSlipRateRes::calcDgssDsliprate()
{
  for (unsigned int i = 0; i < _nss; ++i)
    for (unsigned int j = 0; j < _nss; ++j)
      _dsliprate_dsliprate(i, j) += _dsliprate_dgss(i) * _dgss_dsliprate(i, j);
}

void
FiniteStrainCPSlipRateRes::getSlipIncrements()
{
  FiniteStrainCrystalPlasticity::getSlipIncrements();

  if (_err_tol)
    return;

  _dslipdtau *= 1.0 / _dt;

  for (unsigned int i = 0; i < _nss; ++i)
    _dsliprate_dgss(i) = -_a0(i) / _xm(i) *
                         std::pow(std::abs(_tau(i) / _gss_tmp[i]), 1.0 / _xm(i) - 1.0) * _tau(i) /
                         std::pow(_gss_tmp[i], 2.0);
}

void
FiniteStrainCPSlipRateRes::calcUpdate()
{
  DenseMatrix<Real> A = _jacob;
  DenseVector<Real> r(_nss);
  DenseVector<Real> x(_nss);

  r = _resid;

  A.lu_solve(r, x);

  _resid = x;
}

Real
FiniteStrainCPSlipRateRes::calcResidNorm()
{
  Real rnorm = 0.0;
  for (unsigned int i = 0; i < _nss; ++i)
    rnorm += Utility::pow<2>(_resid(i));
  rnorm = std::sqrt(rnorm) / _nss;

  return rnorm;
}

bool
FiniteStrainCPSlipRateRes::lineSearchUpdateSlipRate(const Real rnorm_prev,
                                                    const DenseVector<Real> & update)
{
  if (_lsrch_method == "CUT_HALF")
  {
    Real rnorm;
    Real step = 1.0;
    do
    {
      for (unsigned int i = 0; i < update.size(); ++i)
        _slip_rate(i) += step * update(i);

      step /= 2.0;

      for (unsigned int i = 0; i < update.size(); ++i)
        _slip_rate(i) -= step * update(i);

      calcResidualSlipRate();
      if (_err_tol)
        return false;
      rnorm = calcResidNorm();
    } while (rnorm > rnorm_prev && step > _min_lsrch_step);

    if (rnorm > rnorm_prev && step <= _min_lsrch_step)
      return false;

    return true;
  }
  else if (_lsrch_method == "BISECTION")
  {
    unsigned int count = 0;
    Real step_a = 0.0;
    Real step_b = 1.0;
    Real step = 1.0;
    Real s_m = 1000.0;
    Real rnorm = 1000.0;

    Real s_b = calcResidDotProdUpdate(update);
    Real rnorm1 = calcResidNorm();

    for (unsigned int i = 0; i < update.size(); ++i)
      _slip_rate(i) += update(i);

    calcResidualSlipRate();
    Real s_a = calcResidDotProdUpdate(update);
    Real rnorm0 = calcResidNorm();

    for (unsigned int i = 0; i < update.size(); ++i)
      _slip_rate(i) -= update(i);

    if ((rnorm1 / rnorm0) < _lsrch_tol || s_a * s_b > 0)
    {
      calcResidualSlipRate();
      return true;
    }

    while ((rnorm / rnorm0) > _lsrch_tol && count < _lsrch_max_iter)
    {

      for (unsigned int i = 0; i < update.size(); ++i)
        _slip_rate(i) += step * update(i);

      step = 0.5 * (step_a + step_b);

      for (unsigned int i = 0; i < update.size(); ++i)
        _slip_rate(i) -= step * update(i);

      calcResidualSlipRate();
      s_m = calcResidDotProdUpdate(update);
      rnorm = calcResidNorm();

      if (s_m * s_a < 0.0)
      {
        step_b = step;
        s_b = s_m;
      }
      if (s_m * s_b < 0.0)
      {
        step_a = step;
        s_a = s_m;
      }
      count++;
    }

    if ((rnorm / rnorm0) < _lsrch_tol && count < _lsrch_max_iter)
      return true;

    return false;
  }
  else
  {
    mooseError("Line search meothod is not provided.");
    return false;
  }
}

Real
FiniteStrainCPSlipRateRes::calcResidDotProdUpdate(const DenseVector<Real> & update)
{
  Real dotprod = 0.0;
  for (unsigned int i = 0; i < _nss; ++i)
    dotprod += _resid(i) * update(i);
  return dotprod;
}
