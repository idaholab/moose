#include "PLC_LSH.h"

#include "SymmIsotropicElasticityTensor.h"

template<>
InputParameters validParams<PLC_LSH>()
{
  InputParameters params = validParams<SolidModel>();

   // Power-law creep material parameters
   params.addRequiredParam<Real>("coefficient", "Leading coefficent in power-law equation");
   params.addRequiredParam<Real>("n_exponent", "Exponent on effective stress in power-law equation");
   params.addParam<Real>("m_exponent", 0.0, "Exponent on time in power-law equation");
   params.addRequiredParam<Real>("activation_energy", "Activation energy");
   params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");

   // Linear strain hardening parameters
   params.addRequiredParam<Real>("yield_stress", "The point at which plastic strain begins accumulating");
   params.addRequiredParam<Real>("hardening_constant", "Hardening slope");

   // Sub-Newton Iteration control parameters
   params.addParam<unsigned int>("max_its", 30, "Maximum number of sub-newton iterations");
   params.addParam<bool>("output_iteration_info", false, "Set true to output sub-newton iteration information");
   params.addParam<Real>("relative_tolerance", 1e-5, "Relative convergence tolerance for sub-newtion iteration");
   params.addParam<Real>("absolute_tolerance", 1e-20, "Absolute convergence tolerance for sub-newtion iteration");

   // Control of combined plasticity-creep iterarion
   params.addParam<Real>("stress_tolerance", 1e-5, "Convergence tolerance for combined plasticity-creep stress iteration");

  return params;
}


PLC_LSH::PLC_LSH( const std::string & name,
                  InputParameters parameters )
  :SolidModel( name, parameters ),
   _coefficient(parameters.get<Real>("coefficient")),
   _n_exponent(parameters.get<Real>("n_exponent")),
   _m_exponent(parameters.get<Real>("m_exponent")),
   _activation_energy(parameters.get<Real>("activation_energy")),
   _gas_constant(parameters.get<Real>("gas_constant")),

   _yield_stress(parameters.get<Real>("yield_stress")),
   _hardening_constant(parameters.get<Real>("hardening_constant")),

   _max_its(parameters.get<unsigned int>("max_its")),
   _output_iteration_info(getParam<bool>("output_iteration_info")),
   _relative_tolerance(parameters.get<Real>("relative_tolerance")),
   _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),

   _stress_tolerance(parameters.get<Real>("stress_tolerance")),

   _creep_strain(declareProperty<SymmTensor>("creep_strain")),
   _creep_strain_old(declarePropertyOld<SymmTensor>("creep_strain")),

   _plastic_strain(declareProperty<SymmTensor>("plastic_strain")),
   _plastic_strain_old(declarePropertyOld<SymmTensor>("plastic_strain")),

   _hardening_variable(declareProperty<Real>("hardening_variable")),
   _hardening_variable_old(declarePropertyOld<Real>("hardening_variable")),

   _del_p(declareProperty<Real>("del_p"))

{
}

void
PLC_LSH::computeStress()
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

  // compute trial stress
  SymmTensor stress_new( *elasticityTensor() * _strain_increment );
  stress_new += _stress_old;

  SymmTensor creep_strain_increment;
  SymmTensor plastic_strain_increment;
  SymmTensor elastic_strain_increment;
  SymmTensor stress_new_last( stress_new );
  Real delS(_stress_tolerance+1);
  unsigned int counter(0);

  while (delS > _stress_tolerance && counter++ < _max_its)
  {
    elastic_strain_increment = _strain_increment;
    elastic_strain_increment -= plastic_strain_increment;
    stress_new = *elasticityTensor() * elastic_strain_increment;
    stress_new += _stress_old;
    computeCreep( creep_strain_increment, stress_new );

   // now use stress_new to calculate a new effective_trial_stress and determine if
   // yield has occured and if so, calculate the corresponding plastic strain

    computeLSH( creep_strain_increment, plastic_strain_increment, stress_new );

    elastic_strain_increment = _strain_increment;
    elastic_strain_increment -= plastic_strain_increment;
    elastic_strain_increment -= creep_strain_increment;

    // now check convergence
    SymmTensor deltaS(stress_new_last - stress_new);
    delS = deltaS.doubleContraction(deltaS);
    stress_new_last = stress_new;

  }

  if(counter++ == _max_its)
  {
    mooseError("Max stress iteration hit during plasticity-creep solve!");
  }

  _strain_increment = elastic_strain_increment;
  _stress[_qp] = stress_new;

}

void
PLC_LSH::computeCreep( SymmTensor & creep_strain_increment,
                       SymmTensor & stress_new )
{
  creep_strain_increment.zero();

// compute deviatoric trial stress
  SymmTensor dev_trial_stress(stress_new);
  dev_trial_stress.addDiag( -dev_trial_stress.trace()/3.0 );

// compute effective trial stress
  Real dts_squared = dev_trial_stress.doubleContraction(dev_trial_stress);
  Real effective_trial_stress = std::sqrt(1.5 * dts_squared);

// Use Newton sub-iteration to determine effective creep strain increment

  Real exponential(1);
  SymmTensor stress_last(_stress_old);
  SymmTensor stress_next;

  Real del_p(0);
  unsigned int it(0);
  Real creep_residual(10);
  Real norm_creep_residual = 10.;
  Real first_norm_creep_residual = 10.;

  while(it < _max_its && norm_creep_residual > _absolute_tolerance && (norm_creep_residual/first_norm_creep_residual) > _relative_tolerance)
  {

    if (_has_temp)
    {
      exponential = std::exp(-_activation_energy/(_gas_constant *_temperature[_qp]));
    }

    Real phi = _coefficient*std::pow(effective_trial_stress - 3.*_shear_modulus*del_p, _n_exponent)*
      exponential*std::pow(_t,_m_exponent);
    Real dphi_ddelp = -3.*_coefficient*_shear_modulus*_n_exponent*
      std::pow(effective_trial_stress-3.*_shear_modulus*del_p, _n_exponent-1.)*exponential*std::pow(_t,_m_exponent);

    creep_residual = phi -  del_p/_dt;
    norm_creep_residual = std::abs(creep_residual);
    if(it==0) first_norm_creep_residual = norm_creep_residual;

    del_p = del_p + (creep_residual / (1/_dt - dphi_ddelp));

    // iteration output
    if (_output_iteration_info == true)
    {

      std::cout
        << " it=" << it
        << " temperature=" << _temperature[_qp]
        << " trial stress=" << effective_trial_stress
        << " phi=" << phi
        << " dphi=" << dphi_ddelp
        << " creep_res=" << norm_creep_residual
        << " del_p=" << del_p
        <<" relative tolerance=" << _relative_tolerance
        <<" absolute tolerance=" << _absolute_tolerance
      << std::endl;
    }

  it++;
  }


  if(it == _max_its && (norm_creep_residual/first_norm_creep_residual) > _relative_tolerance && norm_creep_residual > _absolute_tolerance)
  {
    mooseError("Max sub-newton iteration hit during creep solve!");
  }

// compute creep and elastic strain increments (avoid potential divide by zero - how should this be done)?
  if (effective_trial_stress < 0.01)
  {
    effective_trial_stress = 0.01;
  }

  SymmTensor creep_strain_sub_increment( dev_trial_stress );
  creep_strain_sub_increment *= (1.5*del_p/effective_trial_stress);

  SymmTensor elastic_strain_increment(_strain_increment);
  elastic_strain_increment -= creep_strain_sub_increment;

// compute stress increment
  stress_next = *elasticityTensor() * elastic_strain_increment;

// update stress and creep strain
  stress_next += stress_last;
  stress_last = stress_next;

  creep_strain_increment += creep_strain_sub_increment;
  _del_p[_qp] += del_p;

  _creep_strain[_qp] = creep_strain_increment;
  _creep_strain[_qp] += _creep_strain_old[_qp];
  stress_new = stress_next;

}

void
PLC_LSH::computeLSH( const SymmTensor & creep_strain_increment,
                     SymmTensor & plastic_strain_increment,
                     SymmTensor & stress_new )
{

  if (_t_step == 1)
  {
    _hardening_variable[_qp] =
      _hardening_variable_old[_qp] = 0;
    _plastic_strain[_qp] =
      _plastic_strain_old[_qp] = 0;
  }


// compute deviatoric trial stress
  SymmTensor dev_trial_stress_p(stress_new);
  dev_trial_stress_p.addDiag( -stress_new.trace()/3.0 );

// effective trial stress
  Real dts_squared_p = dev_trial_stress_p.doubleContraction(dev_trial_stress_p);
  Real effective_trial_stress_p = std::sqrt(1.5 * dts_squared_p);

// determine if yield condition is satisfied
  Real yield_condition = effective_trial_stress_p - _hardening_variable_old[_qp] - _yield_stress;

  if (yield_condition > 0.)  //then use newton iteration to determine effective plastic strain increment
  {
    unsigned int jt = 0;
    Real plastic_residual = 10.;
    Real norm_plas_residual = 10.;
    Real first_norm_plas_residual = 10.;
    Real scalar_plastic_strain_increment = 0.;

    _hardening_variable[_qp] = _hardening_variable_old[_qp];
    while(jt < _max_its && norm_plas_residual > _absolute_tolerance && (norm_plas_residual/first_norm_plas_residual) > _relative_tolerance)
    {
      plastic_residual = effective_trial_stress_p - (3. * _shear_modulus * scalar_plastic_strain_increment) -
        _hardening_variable[_qp] - _yield_stress;
      norm_plas_residual = std::abs(plastic_residual);
      if(jt==0) first_norm_plas_residual = norm_plas_residual;

      scalar_plastic_strain_increment = scalar_plastic_strain_increment + ((plastic_residual) / (3. * _shear_modulus + _hardening_constant));

      _hardening_variable[_qp] = _hardening_variable_old[_qp] + (_hardening_constant * scalar_plastic_strain_increment);

      ++jt;

    }

    if(jt == _max_its && (norm_plas_residual/first_norm_plas_residual) > _relative_tolerance && norm_plas_residual > _absolute_tolerance)
    {
      mooseError("Max sub-newton iteration hit during plasticity increment solve!");
    }


// compute plastic and elastic strain increments (avoid potential divide by zero -
// how should this be done)?
// JDH: If we are in this portion of the code, effective_trial_stress_p must be
// non-zero.  The check is unnecessary.
    if (effective_trial_stress_p < 0.01) effective_trial_stress_p = 0.01;
    plastic_strain_increment = dev_trial_stress_p;
    plastic_strain_increment *= (1.5*scalar_plastic_strain_increment/effective_trial_stress_p);

    SymmTensor elastic_strain_increment(_strain_increment);
    elastic_strain_increment -= plastic_strain_increment;
    elastic_strain_increment -= creep_strain_increment;

//compute stress increment
    stress_new = *elasticityTensor() * elastic_strain_increment;

// update stress and plastic strain
    stress_new += _stress_old;
    _plastic_strain[_qp] = plastic_strain_increment;
    _plastic_strain[_qp] += _plastic_strain_old[_qp];

  }//end of if statement

}
