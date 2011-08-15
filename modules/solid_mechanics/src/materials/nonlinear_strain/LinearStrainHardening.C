#include "LinearStrainHardening.h"

#include "SymmIsotropicElasticityTensor.h"


template<>
InputParameters validParams<LinearStrainHardening>()
{
  InputParameters params = validParams<MaterialModel>();
   /*
    Iteration control parameters
   */
   params.addParam<Real>("tolerance", 1e-5, "Convergence tolerance for sub-newtion iteration");
   params.addParam<unsigned int>("max_its", 10, "Maximum number of sub-newton iterations");
   params.addParam<bool>("output_iteration_info", false, "Set true to output sub-newton iteration information");


   /*
     Linear strain hardening parameters
    */
   params.addRequiredParam<Real>("yield_stress", "The point at which plastic strain begins accumulating");
   params.addRequiredParam<Real>("hardening_constant", "Hardening slope");
   params.addParam<Real>("tolerance", 1e-5, "Sub-BiLin iteration tolerance");
   params.addParam<unsigned int>("max_its", 10, "Maximum number of Sub-newton iterations");


  return params;
}


LinearStrainHardening::LinearStrainHardening( const std::string & name,
                                              InputParameters parameters )
  :MaterialModel( name, parameters ),
   _tolerance(parameters.get<Real>("tolerance")),
   _max_its(parameters.get<unsigned int>("max_its")),
   _output_iteration_info(getParam<bool>("output_iteration_info")),

   _yield_stress(parameters.get<Real>("yield_stress")),
   _hardening_constant(parameters.get<Real>("hardening_constant")),

   _plastic_strain(declareProperty<SymmTensor>("plastic_strain")),
   _plastic_strain_old(declarePropertyOld<SymmTensor>("plastic_strain")),

   _hardening_variable(declareProperty<Real>("hardening_variable")),
   _hardening_variable_old(declarePropertyOld<Real>("hardening_variable"))

{
}


void
LinearStrainHardening::computeStress()
{
  // compute trial stress
  SymmTensor stress_new = *elasticityTensor() * _strain_increment;
  stress_new += _stress_old;

  // compute deviatoric trial stress
  SymmTensor dev_trial_stress(stress_new);
  dev_trial_stress.addDiag( -stress_new.trace()/3.0 );

  // effective trial stress
  Real dts_squared = dev_trial_stress.doubleContraction(dev_trial_stress);
  Real effective_trial_stress = std::sqrt(1.5 * dts_squared);

  // determine if yield condition is satisfied
  Real yield_condition = effective_trial_stress - _hardening_variable_old[_qp] - _yield_stress;

  if (yield_condition > 0.)  //then use newton iteration to determine effective plastic strain increment
  {
    unsigned int it = 0;
    Real scalar_plastic_strain_increment = 0.;
    Real norm_residual = 10.;

    _hardening_variable[_qp] = _hardening_variable_old[_qp];
    while(it < _max_its && norm_residual > _tolerance)
    {
      Real plastic_residual = effective_trial_stress - (3. * _shear_modulus * scalar_plastic_strain_increment) - _hardening_variable[_qp] - _yield_stress;
      norm_residual = std::abs(plastic_residual);

      scalar_plastic_strain_increment += ((plastic_residual) / (3. * _shear_modulus + _hardening_constant));

      _hardening_variable[_qp] = _hardening_variable_old[_qp] + (_hardening_constant * scalar_plastic_strain_increment);
      ++it;

    }

    if(it == _max_its)
    {
      mooseError("Max sub-newton iteration hit during plasticity increment solve!");
    }


    // compute plastic and elastic strain increments (avoid potential divide by zero - how should this be done)?
    if (effective_trial_stress < 0.01)
    {
      effective_trial_stress = 0.01;
    }
    SymmTensor plastic_strain_increment(dev_trial_stress);
    plastic_strain_increment *= (1.5*scalar_plastic_strain_increment/effective_trial_stress);

    SymmTensor elastic_strain_increment( _strain_increment );
    elastic_strain_increment -= plastic_strain_increment;

    // update stress and plastic strain
    // compute stress increment
    _stress[_qp] =  *elasticityTensor() * elastic_strain_increment;

    _stress[_qp] += _stress_old;
    _plastic_strain[_qp] = plastic_strain_increment;
    _plastic_strain[_qp] += _plastic_strain_old[_qp];

  } // end of if statement
  else
  {
    SymmTensor elastic_strain_increment(_strain_increment);

    // compute stress increment
    stress_new =  *elasticityTensor() * elastic_strain_increment;

    // update stress and plastic strain
    _stress[_qp] = stress_new;
    _stress[_qp] += _stress_old;

  } // end of else

} // end of computeStress
