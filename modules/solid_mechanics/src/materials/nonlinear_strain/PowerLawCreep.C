#include "PowerLawCreep.h"

#include "IsotropicElasticityTensor.h"

template<>
InputParameters validParams<PowerLawCreep>()
{
  InputParameters params = validParams<MaterialModel>();

  //   Power-law creep material parameters
   params.addRequiredParam<Real>("coefficient", "Leading coefficent in power-law equation");
   params.addRequiredParam<Real>("exponent", "Exponent in power-law equation");
   params.addRequiredParam<Real>("activation_energy", "Activation energy");
   params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");

  //  Sub-Newton Iteration control parameters
   params.addParam<Real>("tolerance", 1e-5, "Convergence tolerance for sub-newtion iteration");
   params.addParam<unsigned int>("max_its", 10, "Maximum number of sub-newton iterations");
   params.addParam<bool>("output_iteration_info", false, "Set true to output sub-newton iteration information");

  return params;
}


PowerLawCreep::PowerLawCreep( const std::string & name,
                              InputParameters parameters )
  :MaterialModel( name, parameters ),
   _coefficient(parameters.get<Real>("coefficient")),
   _exponent(parameters.get<Real>("exponent")),
   _activation_energy(parameters.get<Real>("activation_energy")),
   _gas_constant(parameters.get<Real>("gas_constant")),
   _tolerance(parameters.get<Real>("tolerance")),
   _max_its(parameters.get<unsigned int>("max_its")),
   _output_iteration_info(getParam<bool>("output_iteration_info")),

   _creep_strain(declareProperty<RealTensorValue>("creep_strain")),
   _creep_strain_old(declarePropertyOld<RealTensorValue>("creep_strain"))
{

/*  Below lines are only needed if defining a different elasticity tensor
  IsotropicElasticityTensor * iso =  new IsotropicElasticityTensor;
  iso->setLambda( _lambda );
  iso->setShearModulus( _shear_modulus );
  iso->calculate(0);
  elasticityTensor( iso );
*/
}

void
PowerLawCreep::computeStress()
{
  // Given the stretching, compute the stress increment and add it to the old stress. Also update the creep strain
  // stress = stressOld + stressIncrement
  // creep_strain = creep_strainOld + creep_strainIncrement
  //
  //
  // This is more work than needs to be done.  The strain and stress tensors are symmetric, and so we are carrying
  //   a third more memory than is required.  We are also running a 9x9 * 9x1 matrix-vector multiply when at most
  //   a 6x6 * 6x1 matrix vector multiply is needed.  For the most common case, isotropic elasticity, only two
  //   constants are needed and a matrix vector multiply can be avoided entirely.
  //

  const ColumnMajorMatrix stress_old(_stress_old[_qp]);

  // compute trial stress
  _strain_increment.reshape(9, 1);
  ColumnMajorMatrix stress_new( *elasticityTensor() * _strain_increment );
  _strain_increment.reshape(3, 3);
  stress_new.reshape(3, 3);
  stress_new *= _dt;
  stress_new += stress_old;

  // compute deviatoric trial stress
  ColumnMajorMatrix dev_trial_stress(stress_new);
  dev_trial_stress.addDiag( -dev_trial_stress.tr()/3.0 );

  // compute effective trial stress
  Real dts_squared = dev_trial_stress.doubleContraction(dev_trial_stress);
  Real effective_trial_stress = std::sqrt(1.5 * dts_squared);

  // Use Newton sub-iteration to determine effective creep strain increment

  unsigned int it = 0;
  Real del_p = 0.;
  Real norm_residual = 10.;

  while(it < _max_its && norm_residual > _tolerance)
  {

    Real phi = _coefficient*std::pow(effective_trial_stress - 3.*_shear_modulus*del_p, _exponent)*
      std::exp(-_activation_energy/(_gas_constant*_temperature[_qp]));
    Real dphi_ddelp = -3.*_coefficient*_shear_modulus*_exponent*
      std::pow(effective_trial_stress-3.*_shear_modulus*del_p, _exponent-1.)*
      std::exp(-_activation_energy/(_gas_constant*_temperature[_qp]));
    Real creep_residual = phi -  del_p/_dt;
    norm_residual = std::abs(creep_residual);
    del_p = del_p + (creep_residual / (1/_dt - dphi_ddelp));

    // iteration output
    if (_output_iteration_info == true)
      std::cout
        <<" it=" <<it
        <<" temperature=" << _temperature[_qp]
        <<" trial stress=" <<effective_trial_stress
        <<" phi=" <<phi
        <<" dphi=" <<dphi_ddelp
        <<" creep_res=" <<creep_residual
        <<" del_p=" <<del_p
        <<std::endl;

    ++it;
  }

  if(it == _max_its) mooseError("Max sub-newton iteration hit during creep solve!");


  // compute creep and elastic strain increments (avoid potential divide by zero - how should this be done)?
  if (effective_trial_stress < 0.01)
  {
    effective_trial_stress = 0.01;
  }
  ColumnMajorMatrix creep_strain_increment(dev_trial_stress);
  creep_strain_increment *= (1.5*del_p/effective_trial_stress);

  ColumnMajorMatrix elastic_strain_increment(_strain_increment*_dt);
  elastic_strain_increment -= creep_strain_increment;

  //  compute stress increment
  elastic_strain_increment.reshape(9, 1);
  stress_new =  *elasticityTensor() * elastic_strain_increment;

  // update stress and creep strain
  stress_new.fill(_stress[_qp]);
  _stress[_qp] += _stress_old[_qp];
  creep_strain_increment.fill(_creep_strain[_qp]);
  _creep_strain[_qp] += _creep_strain_old[_qp];

}
