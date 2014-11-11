#include "TensorMechanicsPlasticTensileMulti2.h"

// Following is for perturbing eigvenvalues.  This looks really bodgy, but works quite well!
#include "MooseRandom.h"

template<>
InputParameters validParams<TensorMechanicsPlasticTensileMulti2>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addClassDescription("Associative tensile plasticity with hardening/softening");
  params.addRequiredParam<UserObjectName>("tensile_strength", "A TensorMechanicsHardening UserObject that defines hardening of the tensile strength");
  params.addParam<Real>("shift", "Yield surface is shifted by this amount to avoid problems with defining derivatives when eigenvalues are equal.  If this is larger than f_tol, a warning will be issued.  Default = 0.1*f_tol.");
  return params;
}

TensorMechanicsPlasticTensileMulti2::TensorMechanicsPlasticTensileMulti2(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters),
    _strength(getUserObject<TensorMechanicsHardeningModel>("tensile_strength")),
    _shift(parameters.isParamValid("shift") ? getParam<Real>("shift") : 0.1*_f_tol)
{
  if (_shift < 0)
    mooseError("Value of 'shift' in TensorMechanicsPlasticTensileMulti2 must not be negative\n");
  if (_shift > _f_tol)
    _console << "WARNING: value of 'shift' in TensorMechanicsPlasticTensileMulti2 is probably set too high\n";
  MooseRandom::seed(0);
}

unsigned int
TensorMechanicsPlasticTensileMulti2::numberSurfaces() const
{
  return 1;
}


void
TensorMechanicsPlasticTensileMulti2::yieldFunctionV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & f) const
{
  std::vector<Real> eigvals;
  stress.symmetricEigenvalues(eigvals);
  Real str = tensile_strength(intnl);

  f.resize(1);
  f[0] = eigvals[2] - _shift - str;
}


void
TensorMechanicsPlasticTensileMulti2::dyieldFunction_dstressV(const RankTwoTensor & stress, const Real & /*intnl*/, std::vector<RankTwoTensor> & df_dstress) const
{
  std::vector<Real> eigvals;
  std::vector<RankTwoTensor> df;
  stress.dsymmetricEigenvalues(eigvals, df);
  df_dstress.resize(1);
  df_dstress[0] = df[2];

}


void
TensorMechanicsPlasticTensileMulti2::dyieldFunction_dintnlV(const RankTwoTensor & /*stress*/, const Real & intnl, std::vector<Real> & df_dintnl) const
{
  df_dintnl.assign(1, -dtensile_strength(intnl));
}


void
TensorMechanicsPlasticTensileMulti2::flowPotentialV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & r) const
{
  // This plasticity is associative so
  dyieldFunction_dstressV(stress, intnl, r);
}


void
TensorMechanicsPlasticTensileMulti2::dflowPotential_dstressV(const RankTwoTensor & stress, const Real & /*intnl*/, std::vector<RankFourTensor> & dr_dstress) const
{
  std::vector<RankFourTensor> dr;
  stress.d2symmetricEigenvalues(dr);
  dr_dstress.resize(1);
  dr_dstress[0] = dr[2];
}


void
TensorMechanicsPlasticTensileMulti2::dflowPotential_dintnlV(const RankTwoTensor & /*stress*/, const Real & /*intnl*/, std::vector<RankTwoTensor> & dr_dintnl) const
{
  dr_dintnl.assign(1, RankTwoTensor());
}


Real
TensorMechanicsPlasticTensileMulti2::tensile_strength(const Real internal_param) const
{
  return _strength.value(internal_param);
}

Real
TensorMechanicsPlasticTensileMulti2::dtensile_strength(const Real internal_param) const
{
  return _strength.derivative(internal_param);
}
