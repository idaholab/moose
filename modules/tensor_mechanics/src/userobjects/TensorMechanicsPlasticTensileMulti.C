/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsPlasticTensileMulti.h"

// Following is for perturbing eigvenvalues.  This looks really bodgy, but works quite well!
#include "MooseRandom.h"

template<>
InputParameters validParams<TensorMechanicsPlasticTensileMulti>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addClassDescription("Associative tensile plasticity with hardening/softening");
  params.addRequiredParam<UserObjectName>("tensile_strength", "A TensorMechanicsHardening UserObject that defines hardening of the tensile strength");
  params.addParam<Real>("shift", "Yield surface is shifted by this amount to avoid problems with defining derivatives when eigenvalues are equal.  If this is larger than f_tol, a warning will be issued.  Default = f_tol.");
  return params;
}

TensorMechanicsPlasticTensileMulti::TensorMechanicsPlasticTensileMulti(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters),
    _strength(getUserObject<TensorMechanicsHardeningModel>("tensile_strength")),
    _shift(parameters.isParamValid("shift") ? getParam<Real>("shift") : _f_tol)
{
  if (_shift < 0)
    mooseError("Value of 'shift' in TensorMechanicsPlasticTensileMulti must not be negative\n");
  if (_shift > _f_tol)
    _console << "WARNING: value of 'shift' in TensorMechanicsPlasticTensileMulti is probably set too high\n";
  MooseRandom::seed(0);
}

unsigned int
TensorMechanicsPlasticTensileMulti::numberSurfaces() const
{
  return 3;
}


void
TensorMechanicsPlasticTensileMulti::yieldFunctionV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & f) const
{
  std::vector<Real> eigvals;
  stress.symmetricEigenvalues(eigvals);
  Real str = tensile_strength(intnl);

  f.resize(3);
  f[0] = eigvals[0] + _shift - str;
  f[1] = eigvals[1] - str;
  f[2] = eigvals[2] - _shift - str;
}


void
TensorMechanicsPlasticTensileMulti::dyieldFunction_dstressV(const RankTwoTensor & stress, const Real & /*intnl*/, std::vector<RankTwoTensor> & df_dstress) const
{
  std::vector<Real> eigvals;
  stress.dsymmetricEigenvalues(eigvals, df_dstress);

  if (eigvals[0] > eigvals[1] - 0.1*_shift || eigvals[1] > eigvals[2] - 0.1*_shift)
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
      shifted_stress.dsymmetricEigenvalues(eigvals, df_dstress);
    }
  }
}


void
TensorMechanicsPlasticTensileMulti::dyieldFunction_dintnlV(const RankTwoTensor & /*stress*/, const Real & intnl, std::vector<Real> & df_dintnl) const
{
  df_dintnl.assign(3, -dtensile_strength(intnl));
}


void
TensorMechanicsPlasticTensileMulti::flowPotentialV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & r) const
{
  // This plasticity is associative so
  dyieldFunction_dstressV(stress, intnl, r);
}


void
TensorMechanicsPlasticTensileMulti::dflowPotential_dstressV(const RankTwoTensor & stress, const Real & /*intnl*/, std::vector<RankFourTensor> & dr_dstress) const
{
  stress.d2symmetricEigenvalues(dr_dstress);
}


void
TensorMechanicsPlasticTensileMulti::dflowPotential_dintnlV(const RankTwoTensor & /*stress*/, const Real & /*intnl*/, std::vector<RankTwoTensor> & dr_dintnl) const
{
  dr_dintnl.assign(3, RankTwoTensor());
}

Real
TensorMechanicsPlasticTensileMulti::tensile_strength(const Real internal_param) const
{
  return _strength.value(internal_param);
}

Real
TensorMechanicsPlasticTensileMulti::dtensile_strength(const Real internal_param) const
{
  return _strength.derivative(internal_param);
}


void
TensorMechanicsPlasticTensileMulti::activeConstraints(const std::vector<Real> & f, const RankTwoTensor & stress, const Real & intnl, const RankFourTensor & Eijkl, std::vector<bool> & act, RankTwoTensor & returned_stress) const
{
  act.assign(3, false);

  if (f[0] <= _f_tol && f[1] <= _f_tol && f[2] <= _f_tol)
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

  Real str = tensile_strength(intnl);
  std::vector<Real> v(3);
  v[0] = eigvals[0] - str;
  v[1] = eigvals[1] - str;
  v[2] = eigvals[2] - str;

  // these are the normals to the 3 yield surfaces
  std::vector<std::vector<Real> > n(3);
  n[0].resize(3);
  n[0][0] = 1 ; n[0][1] = 0 ; n[0][2] = 0;
  n[1].resize(3);
  n[1][0] = 0 ; n[1][1] = 1 ; n[1][2] = 0;
  n[2].resize(3);
  n[2][0] = 0 ; n[2][1] = 0 ; n[2][2] = 1;

  // the flow directions are these n multiplied by Eijkl.
  // I re-use the name "n" for the flow directions
  // In the following I assume that the Eijkl is
  // for an isotropic situation.  This is the most
  // common when using TensileMulti, and remember
  // that the returned_stress need not be perfect
  // anyway.
  // I divide by E(0,0,0,0) so the n remain of order 1
  Real ratio = Eijkl(1,1,0,0)/Eijkl(0,0,0,0);
  n[0][1] = n[0][2] = ratio;
  n[1][0] = n[1][2] = ratio;
  n[2][0] = n[2][1] = ratio;


  // 111 (tip)
  // For tip-return to satisfy Kuhn-Tucker, we need
  // v = alpha*n[0] + beta*n[1] * gamma*n[2]
  // with alpha, beta, and gamma all being non-negative (they are
  // the plasticity multipliers)
  Real denom = triple(n[0], n[1], n[2]);
  if (triple(v, n[0], n[1])/denom >= 0 && triple(v, n[1], n[2])/denom >= 0 && triple(v, n[2], n[0])/denom >= 0)
  {
    act[0] = act[1] = act[2] = true;
    returned_stress(0, 0) = returned_stress(1, 1) = returned_stress(2, 2) = str;
    returned_stress = eigvecs*returned_stress*(eigvecs.transpose());
    return;
  }

  // 011 (edge)
  std::vector<Real> n1xn2(3);
  n1xn2[0] = n[1][1]*n[2][2] - n[1][2]*n[2][1];
  n1xn2[1] = n[1][2]*n[2][0] - n[1][0]*n[2][2];
  n1xn2[2] = n[1][0]*n[2][1] - n[1][1]*n[2][0];
  // work out the point to which we would return, "a".  It is defined by
  // f1 = 0 = f2, and that (p - a).(n1 x n2) = 0, where "p" is the
  // starting position (p = eigvals).
  // In the following a = (a0, str, str)
  Real pdotn1xn2 = dot(eigvals, n1xn2);
  Real a0 = (-str*n1xn2[1] - str*n1xn2[2] + pdotn1xn2)/n1xn2[0];
  // we need p - a = alpha*n1 + beta*n2, where alpha and beta are non-negative
  // for Kuhn-Tucker to be satisfied
  std::vector<Real> pminusa(3);
  pminusa[0] = eigvals[0] - a0;
  pminusa[1] = v[1];
  pminusa[2] = v[2];
  if ((pminusa[2] - pminusa[0])/(1.0 - ratio) >= 0 && (pminusa[1] - pminusa[0])/(1.0 - ratio) >= 0)
  {
    returned_stress(0, 0) = a0;
    returned_stress(1, 1) = str;
    returned_stress(2, 2) = str;
    returned_stress = eigvecs*returned_stress*(eigvecs.transpose());
    act[1] = act[2] = true;
    return;
  }

  // 001 (plane)
  // the returned point, "a", is defined by f2=0 and
  // a = p - alpha*n2
  Real alpha = (eigvals[2] - str)/n[2][2];
  act[2] = true;
  returned_stress(0, 0) = eigvals[0] - alpha*n[2][0];
  returned_stress(1, 1) = eigvals[1] - alpha*n[2][1];
  returned_stress(2, 2) = str;
  returned_stress = eigvecs*returned_stress*(eigvecs.transpose());
  return;
}

Real
TensorMechanicsPlasticTensileMulti::dot(const std::vector<Real> & a, const std::vector<Real> & b) const
{
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

Real
TensorMechanicsPlasticTensileMulti::triple(const std::vector<Real> & a, const std::vector<Real> & b, const std::vector<Real> & c) const
{
  return a[0]*(b[1]*c[2] - b[2]*c[1]) - a[1]*(b[0]*c[2] - b[2]*c[0]) + a[2]*(b[0]*c[1] - b[1]*c[0]);
}

std::string
TensorMechanicsPlasticTensileMulti::modelName() const
{
  return "TensileMulti";
}
