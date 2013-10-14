// Name RadialReturn
//   or ReturnMappingModel ...

#include "ConstitutiveModel.h"

template<>
InputParameters validParams<ConstitutiveModel>()
{
  InputParameters params = validParams<Material>();

  // Sub-Newton Iteration control parameters
  params.addParam<unsigned int>("max_its", 30, "Maximum number of sub-newton iterations");
  params.addParam<bool>("output_iteration_info", false, "Set true to output sub-newton iteration information");
  params.addParam<Real>("relative_tolerance", 1e-5, "Relative convergence tolerance for sub-newtion iteration");
  params.addParam<Real>("absolute_tolerance", 1e-20, "Absolute convergence tolerance for sub-newtion iteration");
  params.addCoupledVar("temp", "Coupled Temperature");

  return params;
}


ConstitutiveModel::ConstitutiveModel( const std::string & name,
                                      InputParameters parameters )
  :Material( name, parameters ),
   _max_its(parameters.get<unsigned int>("max_its")),
   _output_iteration_info(getParam<bool>("output_iteration_info")),
   _relative_tolerance(parameters.get<Real>("relative_tolerance")),
   _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
   _has_temp(isCoupled("temp")),
   _temperature(_has_temp ? coupledValue("temp") : _zero),
   _temperature_old(_has_temp ? coupledValueOld("temp") : _zero)
{}

void
ConstitutiveModel::computeStress( unsigned qp,
                                  const SymmElasticityTensor & elasticityTensor,
                                  const SymmTensor & strain_increment,
                                  const SymmTensor & stress_old,
                                  SymmTensor & inelastic_strain_increment,
                                  SymmTensor & stress_new )
{

  // compute deviatoric trial stress
  SymmTensor dev_trial_stress(stress_new);
  dev_trial_stress.addDiag( -dev_trial_stress.trace()/3.0 );

  // compute effective trial stress
  Real dts_squared = dev_trial_stress.doubleContraction(dev_trial_stress);
  Real effective_trial_stress = std::sqrt(1.5 * dts_squared);

  computeStressInitialize(qp, effective_trial_stress, elasticityTensor);

  // Use Newton sub-iteration to determine inelastic strain increment

  Real scalar = 0;
  unsigned int it = 0;
  Real residual = 10;
  Real norm_residual = 10;
  Real first_norm_residual = 10;

  while(it < _max_its &&
        norm_residual > _absolute_tolerance &&
        (norm_residual/first_norm_residual) > _relative_tolerance)
  {
    iterationInitialize( qp, scalar );

    residual = computeResidual(qp, effective_trial_stress, scalar);
    norm_residual = std::abs(residual);
    if (it == 0)
    {
      first_norm_residual = norm_residual;
    }

    scalar -= residual / computeDerivative(qp, effective_trial_stress, scalar);

    if (_output_iteration_info == true)
    {
      std::cout
      << " it="       << it
      << " trl_strs=" << effective_trial_stress
      << " scalar="   << scalar
      << " rel_res="  << norm_residual/first_norm_residual
      << " rel_tol="  << _relative_tolerance
      << " abs_res="  << norm_residual
      << " abs_tol="  << _absolute_tolerance
      << std::endl;
    }

    iterationFinalize( qp, scalar );

    ++it;
  }


  if(it == _max_its &&
     norm_residual > _absolute_tolerance &&
     (norm_residual/first_norm_residual) > _relative_tolerance)
  {
    mooseError("Max sub-newton iteration hit during nonlinear constitutive model solve!");
  }

  // compute creep and elastic strain increments (avoid potential divide by zero - how should this be done)?
  if (effective_trial_stress < 0.01)
  {
    effective_trial_stress = 0.01;
  }

  inelastic_strain_increment = dev_trial_stress;
  inelastic_strain_increment *= (1.5*scalar/effective_trial_stress);

  SymmTensor elastic_strain_increment(strain_increment);
  elastic_strain_increment -= inelastic_strain_increment;

  // compute stress increment
  stress_new = elasticityTensor * elastic_strain_increment;

  // update stress
  stress_new += stress_old;

  computeStressFinalize(qp, inelastic_strain_increment);

}
