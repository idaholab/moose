#include "TensorMechanicsPlasticTensileMulti.h"

// Following is for perturbing eigvenvalues.  This looks really bodgy, but works quite well!
#include "MooseRandom.h"

template<>
InputParameters validParams<TensorMechanicsPlasticTensileMulti>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addClassDescription("Associative tensile plasticity with hardening/softening");
  params.addRequiredParam<UserObjectName>("tensile_strength", "A TensorMechanicsHardening UserObject that defines hardening of the tensile strength");
  params.addParam<Real>("shift", "Yield surface is shifted by this amount to avoid problems with defining derivatives when eigenvalues are equal.  If this is larger than f_tol, a warning will be issued.  Default = 0.1*f_tol.");
  return params;
}

TensorMechanicsPlasticTensileMulti::TensorMechanicsPlasticTensileMulti(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters),
    _strength(getUserObject<TensorMechanicsHardeningModel>("tensile_strength")),
    _shift(parameters.isParamValid("shift") ? getParam<Real>("shift") : 0.1*_f_tol)
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

std::string
TensorMechanicsPlasticTensileMulti::modelName() const
{
  return "TensileMulti";
}
