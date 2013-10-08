#include "CLSHPlasticMaterial.h"

#include "SymmIsotropicElasticityTensor.h"
#include <cmath>

template<>
InputParameters validParams<CLSHPlasticMaterial>()
{
   InputParameters params = validParams<SolidModel>();
   params.addRequiredParam<Real>("yield_stress", "The point at which plastic strain begins accumulating");
   params.addRequiredParam<Real>("hardening_constant", "Hardening slope");
   params.addRequiredParam<Real>("c_alpha", "creep constant");
   params.addRequiredParam<Real>("c_beta", "creep constant");
   params.addParam<Real>("tolerance", 1e-5, "Sub-BiLin iteration tolerance");
   params.addParam<unsigned int>("max_its", 10, "Maximum number of Sub-newton iterations");
   params.addParam<bool>("print_debug_info", false, "Whether or not to print debugging information");
   return params;
}

CLSHPlasticMaterial::CLSHPlasticMaterial(std::string name,
                                         InputParameters parameters)
  :SolidModel(name, parameters),
   _yield_stress(parameters.get<Real>("yield_stress")),
   _hardening_constant(parameters.get<Real>("hardening_constant")),
   _c_alpha(parameters.get<Real>("c_alpha")),
   _c_beta(parameters.get<Real>("c_beta")),
   _tolerance(parameters.get<Real>("tolerance")),
   _max_its(parameters.get<unsigned int>("max_its")),
   _print_debug_info(getParam<bool>("print_debug_info")),
   _hardening_variable(declareProperty<Real>("hardening_variable")),
   _hardening_variable_old(declarePropertyOld<Real>("hardening_variable")),
   _plastic_strain(declareProperty<SymmTensor>("plastic_strain")),
   _plastic_strain_old(declarePropertyOld<SymmTensor>("plastic_strain"))
{
  _shear_modulus = _youngs_modulus / ((2*(1+_poissons_ratio)));

  _ebulk3 = _youngs_modulus/(1. - 2.*_poissons_ratio);

  _K = _ebulk3/3.;

}

void
CLSHPlasticMaterial::initQpStatefulProperties()
{
  _hardening_variable[_qp] = 0;
  SolidModel::initQpStatefulProperties();
}

void
CLSHPlasticMaterial::computeStress()
{
// trial stress
  SymmTensor trial_stress = *elasticityTensor() * _strain_increment;
  trial_stress += _stress_old;

// deviatoric trial stress
  SymmTensor dev_trial_stress(trial_stress);
  dev_trial_stress.addDiag( -trial_stress.trace()/3.0 );
// effective trial stress
  Real dts_squared = dev_trial_stress.doubleContraction(dev_trial_stress);
  Real effective_trial_stress = std::sqrt(1.5 * dts_squared);

// determine if yield condition is satisfied
  Real yield_condition = effective_trial_stress - _hardening_variable_old[_qp] - _yield_stress;

  if (yield_condition > 0.)  //then use newton iteration to determine effective plastic strain increment
  {
    unsigned int it = 0;
    Real plastic_residual = 10.;
    Real plastic_strain_increment = 0.;
    Real norm_residual = 10.;
    Real xflow = 1.;
    Real xphi = 1.;
    Real xphidp = 1.;
    Real xphir = 1.;
    Real xresidual = 1.;

    while(it < _max_its && norm_residual > _tolerance)
    {
      xflow = _c_beta*(effective_trial_stress - (3. * _shear_modulus * plastic_strain_increment) - _hardening_variable[_qp] - _yield_stress);
      xphi = _c_alpha*std::sinh(xflow);
      xphidp = -3.*_shear_modulus*_c_alpha*_c_beta*std::cosh(xflow);
      xphir = -_c_alpha*_c_beta*std::cosh(xflow);
      xresidual = xphi - (plastic_strain_increment/_dt);
      plastic_residual = xresidual/((1./_dt) - xphidp - (_hardening_constant*xphir));

      norm_residual = std::abs(plastic_residual);

      plastic_strain_increment = plastic_strain_increment + plastic_residual;

      _hardening_variable[_qp] = _hardening_variable_old[_qp] + (_hardening_constant * plastic_strain_increment);
      it++;

    }


    if(it == _max_its)
      mooseError("Max sub-newton iteration hit during plasticity increment solve!");

    SymmTensor matrix_plastic_strain_increment(dev_trial_stress);
    matrix_plastic_strain_increment *= (1.5*plastic_strain_increment/effective_trial_stress);

    _strain_increment -= matrix_plastic_strain_increment;

    // update stress and plastic strain
    // compute stress increment
    _stress[_qp] =  *elasticityTensor() * _strain_increment;
    _stress[_qp] += _stress_old;

    // update plastic strain
    _plastic_strain[_qp] = matrix_plastic_strain_increment;
    _plastic_strain[_qp] += _plastic_strain_old[_qp];

    _Jacobian_mult[_qp] = *elasticityTensor();

//end of if
  }

  else
  {
    // update stress and plastic strain
    _stress[_qp] = trial_stress;

    _hardening_variable[_qp] = 0.0;
    _plastic_strain[_qp].zero();
    _Jacobian_mult[_qp] = *elasticityTensor();
  }


}
