/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RateDepSmearCrackModel.h"

template <>
InputParameters
validParams<RateDepSmearCrackModel>()
{

  InputParameters params = validParams<ConstitutiveModel>();

  params.addRequiredParam<Real>("ref_damage_rate", "Reference damage rate");
  params.addRequiredParam<unsigned int>("nstate", "Number of state variables");
  params.addParam<Real>("exponent", 1.0, "Power law exponent");
  params.addParam<unsigned int>("maxiter", 20, "Constitutive update iteration");
  params.addParam<Real>("tol", 1e-5, "Constitutive update tolerance");
  params.addParam<Real>("zero_tol", 1e-8, "Tolerance for zero");
  params.addParam<Real>(
      "intvar_incr_tol", 0.1, "Allowable relative increment size for state variables");
  params.addParam<bool>("input_random_scaling_var",
                        false,
                        "Flag to specify scaling parameter to generate random stress");
  params.addParam<Real>("random_scaling_var",
                        1e9,
                        "Scaling value: Too large a value can cause "
                        "non-positive definiteness - use 0.1 of young's "
                        "modulus");

  return params;
}

RateDepSmearCrackModel::RateDepSmearCrackModel(const InputParameters & parameters)
  : ConstitutiveModel(parameters),
    _ref_damage_rate(getParam<Real>("ref_damage_rate")),
    _nstate(getParam<unsigned int>("nstate")),
    _exponent(getParam<Real>("exponent")),
    _maxiter(getParam<unsigned int>("maxiter")),
    _tol(getParam<Real>("tol")),
    _zero_tol(getParam<Real>("zero_tol")),
    _intvar_incr_tol(getParam<Real>("intvar_incr_tol")),
    _input_rndm_scale_var(getParam<bool>("input_random_scaling_var")),
    _rndm_scale_var(getParam<Real>("random_scaling_var")),
    _intvar(declareProperty<std::vector<Real>>("intvar")),
    _intvar_old(declarePropertyOld<std::vector<Real>>("intvar")),
    _stress_undamaged(declareProperty<SymmTensor>("stress_undamaged")),
    _stress_undamaged_old(declarePropertyOld<SymmTensor>("stress_undamaged"))
{

  _intvar_incr.resize(_nstate, 0.0);
  _intvar_tmp.resize(_nstate, 0.0);
  _intvar_old_tmp.resize(_nstate, 0.0);
  _resid.resize(_nstate, 0.0);
  _jac.resize(_nstate * _nstate, 0.0);
  _dvar.resize(_nstate, 0.0);

  SymmTensor::initRandom();
}

void
RateDepSmearCrackModel::initStatefulProperties(unsigned int n_points)
{

  for (unsigned int qp = 0; qp < n_points; qp++)
  {

    _intvar[qp].resize(_nstate, 0.0);
    _intvar_old[qp].resize(_nstate, 0.0);
  }
}

void
RateDepSmearCrackModel::computeStress(const Elem & /*current_elem*/,
                                      unsigned qp,
                                      const SymmElasticityTensor & elasticityTensor,
                                      const SymmTensor & stress_old,
                                      SymmTensor & strain_increment,
                                      SymmTensor & stress_new)
{

  _qp = qp;
  _elasticity = elasticityTensor;
  _stress_old = stress_old;
  _dstrain = strain_increment;

  initVariables();
  solve();

  if (!_input_rndm_scale_var)
    _rndm_scale_var = elasticityTensor.valueAtIndex(0);

  if (_nconv || _err_tol)
  {
    mooseWarning("RateDepSmearCrackModel: Constitutive cutback");
    stress_new = SymmTensor::genRandomSymmTensor(_rndm_scale_var);
  }
  else
  {
    postSolveVariables();
    postSolveStress();

    stress_new = _stress_new;
    _stress_undamaged[qp] = _stress0;
  }
}

void
RateDepSmearCrackModel::initVariables()
{

  _dstress0 = _elasticity * _dstrain;
  _stress0 = _stress_undamaged_old[_qp] + _dstress0;

  for (unsigned int i = 0; i < _nstate; ++i)
  {
    _intvar_tmp[i] = _intvar_old[_qp][i];
    _intvar_old_tmp[i] = _intvar_old[_qp][i];
  }
}

void
RateDepSmearCrackModel::solve()
{

  unsigned int iter = 0;
  _err_tol = false;

  calcResidual();

  _nconv = getConvergeVar();

  while (_nconv && iter < _maxiter && !_err_tol)
  {
    calcJacobian();
    updateVariables();
    calcResidual();

    _nconv = getConvergeVar();

    iter++;
  }
}

void
RateDepSmearCrackModel::updateVariables()
{
  int error = matrixInversion(_jac, _nstate);
  if (error != 0)
    mooseError("Error in Matrix  Inversion in RankFourTensor");

  for (unsigned int i = 0; i < _nstate; i++)
  {
    _dvar[i] = 0.0;
    for (unsigned int j = 0; j < _nstate; j++)
      _dvar[i] += _jac[i * _nstate + j] * _resid[j];
  }

  for (unsigned int i = 0; i < _nstate; i++)
    _intvar_tmp[i] -= _dvar[i];
}

bool
RateDepSmearCrackModel::getConvergeVar()
{
  Real vold, r;

  for (unsigned int i = 0; i < _nstate; i++)
  {
    vold = std::abs(_intvar_old_tmp[i]);
    r = std::abs(_resid[i]);

    if (vold > _zero_tol)
    {
      if (r > _tol * vold)
        return true;
    }
    else
    {
      if (r > _zero_tol)
        return true;
    }
  }
  return false;
}

void
RateDepSmearCrackModel::postSolveVariables()
{
  for (unsigned int i = 0; i < _nstate; ++i)
    _intvar[_qp][i] = _intvar_tmp[i];
}

void
RateDepSmearCrackModel::postSolveStress()
{
}

void
RateDepSmearCrackModel::calcResidual()
{
  calcStateIncr();

  if (_err_tol)
    return;

  for (unsigned int i = 0; i < _nstate; ++i)
    _resid[i] = _intvar_tmp[i] - _intvar_old_tmp[i] - _intvar_incr[i];
}

void
RateDepSmearCrackModel::calcStateIncr()
{
}

void
RateDepSmearCrackModel::calcJacobian()
{
}

int
RateDepSmearCrackModel::matrixInversion(std::vector<Real> & A, int n) const
{
  int return_value, buffer_size = n * 64;
  std::vector<PetscBLASInt> ipiv(n);
  std::vector<PetscScalar> buffer(buffer_size);

  // Following does a LU decomposition of "square matrix A"
  // upon return "A = P*L*U" if return_value == 0
  // Here i use quotes because A is actually an array of length n^2, not a matrix of size n-by-n
  LAPACKgetrf_(&n, &n, &A[0], &n, &ipiv[0], &return_value);

  if (return_value != 0)
    // couldn't LU decompose because: illegal value in A; or, A singular
    return return_value;

// get the inverse of A
#if PETSC_VERSION_LESS_THAN(3, 5, 0)
  FORTRAN_CALL(dgetri)(&n, &A[0], &n, &ipiv[0], &buffer[0], &buffer_size, &return_value);
#else
  LAPACKgetri_(&n, &A[0], &n, &ipiv[0], &buffer[0], &buffer_size, &return_value);
#endif

  return return_value;
}

RateDepSmearCrackModel::~RateDepSmearCrackModel() {}
