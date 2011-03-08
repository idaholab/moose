#include "PLC_LSH.h"

#include "IsotropicElasticityTensor.h"

template<>
InputParameters validParams<PLC_LSH>()
{
  InputParameters params = validParams<MaterialModel>();

   // Power-law creep material parameters
   params.addRequiredParam<Real>("coefficient", "Leading coefficent in power-law equation");
   params.addRequiredParam<Real>("exponent", "Exponent in power-law equation");
   params.addRequiredParam<Real>("activation_energy", "Activation energy");
   params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");
   params.addParam<Real>("tau", 0.01, "Creep sub-iteration control parameter");

   // Sub-Newton Iteration control parameters
   params.addParam<Real>("tolerance", 1e-5, "Convergence tolerance for sub-newtion iteration");
   params.addParam<unsigned int>("max_its", 10, "Maximum number of sub-newton iterations");
   params.addParam<bool>("output_iteration_info", false, "Set true to output sub-newton iteration information");


   // Linear strain hardening parameters
   params.addRequiredParam<Real>("yield_stress", "The point at which plastic strain begins accumulating");
   params.addRequiredParam<Real>("hardening_constant", "Hardening slope");
   params.addParam<Real>("tolerance", 1e-5, "Sub-BiLin iteration tolerance");
   params.addParam<unsigned int>("max_its", 10, "Maximum number of Sub-newton iterations");


  return params;
}


PLC_LSH::PLC_LSH( const std::string & name,
                  InputParameters parameters )
  :MaterialModel( name, parameters ),
   _coefficient(parameters.get<Real>("coefficient")),
   _exponent(parameters.get<Real>("exponent")),
   _activation_energy(parameters.get<Real>("activation_energy")),
   _gas_constant(parameters.get<Real>("gas_constant")),
   _tau(parameters.get<Real>("tau")),
   _tolerance(parameters.get<Real>("tolerance")),

   _yield_stress(parameters.get<Real>("yield_stress")),
   _hardening_constant(parameters.get<Real>("hardening_constant")),

   _max_its(parameters.get<unsigned int>("max_its")),
   _output_iteration_info(getParam<bool>("output_iteration_info")),

   _creep_strain(declareProperty<RealTensorValue>("creep_strain")),
   _creep_strain_old(declarePropertyOld<RealTensorValue>("creep_strain")),

   _plastic_strain(declareProperty<RealTensorValue>("plastic_strain")),
   _plastic_strain_old(declarePropertyOld<RealTensorValue>("plastic_strain")),

   _total_strain(declareProperty<RealTensorValue>("total_strain")),
   _total_strain_old(declarePropertyOld<RealTensorValue>("total_strain")),

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
  const ColumnMajorMatrix stress_old(_stress_old[_qp]);

// compute trial stress
  _strain_increment.reshape(9, 1);
  ColumnMajorMatrix stress_new( *elasticityTensor() * _strain_increment );
  _strain_increment.reshape(3, 3);
  stress_new.reshape(3, 3);
  stress_new *= _dt;
  stress_new += stress_old;

  ColumnMajorMatrix creep_strain_increment;
  ColumnMajorMatrix stress_new_last( stress_new );
  Real delS(_tolerance+1);
  unsigned int counter(0);

  _total_strain_increment.fill( _total_strain[_qp] );
  _total_strain[_qp] *= _dt;
  _total_strain[_qp] += _total_strain_old[_qp];

  while (delS > _tolerance && counter++ < _max_its)
  {
    computeCreep( stress_old, stress_new, creep_strain_increment );

// now use stress_new to calculate a new effective_trial_stress and determine if
// yield has occured and if so, calculate the corresponding plastic strain

    computeLSH( stress_new, creep_strain_increment );

    // now check convergence
    ColumnMajorMatrix deltaS(stress_new_last - stress_new);
    stress_new_last = stress_new;
    delS = deltaS.doubleContraction(deltaS);
  }

  stress_new.fill(_stress[_qp]);

}

void
PLC_LSH::computeCreep( const ColumnMajorMatrix & stress_old,
                       ColumnMajorMatrix & stress_new,
                       ColumnMajorMatrix & creep_strain_increment )
{
  creep_strain_increment.zero();

// compute deviatoric trial stress
  ColumnMajorMatrix dev_trial_stress(stress_new);
  dev_trial_stress.addDiag( -stress_new.tr()/3.0 );

// compute effective trial stress
  Real dts_squared = dev_trial_stress.doubleContraction(dev_trial_stress);
  Real effective_trial_stress = std::sqrt(1.5 * dts_squared);

// Use Newton sub-iteration to determine effective creep strain increment

  Real exponential(1);
  Real delta_temp(_temperature[_qp]-_temperature_old[_qp]);

  Real phiTest( _coefficient*std::pow(effective_trial_stress, _exponent) );
  ColumnMajorMatrix dev_strain(_total_strain[_qp]);
  dev_strain.addDiag( -dev_strain.tr()/3 );
  Real epsTotal( dev_strain.doubleContraction(dev_strain) );
  const Real ratio( epsTotal > 0 ? _dt * ( phiTest / ( _tau * epsTotal ) ) + 1 : 1 );

  unsigned int num_steps( std::floor(ratio) );
  num_steps = std::min( num_steps, unsigned(10) );

  const Real fraction( 1./num_steps );
  const Real dt( _dt * fraction );
  ColumnMajorMatrix delta_stress(stress_new);
  delta_stress -= stress_old;
  delta_stress *= 0.5*fraction;
  ColumnMajorMatrix stress_last(stress_old);
  ColumnMajorMatrix stress_ave(stress_last);
  ColumnMajorMatrix stress_next;

  Real del_p(0);
  if (_t_step > 1)
  {
    del_p = fraction * _del_p[_qp] * _dt / (_dt_old > 0 ? _dt_old : 1) ;
  }
  _del_p[_qp] = 0;

  for ( unsigned int i_step(0); i_step < num_steps; ++i_step )
  {
    const Real factor(Real(i_step+1) * fraction);

    if (_has_temp)
    {
      exponential = std::exp(-_activation_energy/(_gas_constant *
                                                  (_temperature_old[_qp] + factor*delta_temp)));
    }

    stress_ave = stress_last;
    stress_ave += delta_stress;

// compute deviatoric trial stress
    ColumnMajorMatrix dev_trial_stress(stress_ave);
    dev_trial_stress.addDiag( -stress_ave.tr()/3.0 );

// compute effective trial stress
    Real dts_squared = dev_trial_stress.doubleContraction(dev_trial_stress);
    Real effective_trial_stress = std::sqrt(1.5 * dts_squared);

    unsigned int it(0);
    Real creep_residual(10);
    while(std::abs(creep_residual) > _tolerance && it++ < _max_its)
    {

      Real phi = _coefficient*std::pow(effective_trial_stress - 3.*_shear_modulus*del_p, _exponent)*
        exponential;
      creep_residual = phi -  del_p/dt;

      Real dphi_ddelp = -3.*_coefficient*_shear_modulus*_exponent*
        std::pow(effective_trial_stress-3.*_shear_modulus*del_p, _exponent-1.)*
        exponential;
      del_p = del_p + (creep_residual / (1/dt - dphi_ddelp));

      // iteration output
      if (_output_iteration_info == true)
        std::cout
          << " it=" << it
          << " temperature=" << _temperature_old[_qp] + factor*delta_temp
          << " trial stress=" << effective_trial_stress
          << " phi=" << phi
          << " dphi=" << dphi_ddelp
          << " creep_res=" << creep_residual
          << " del_p=" << del_p
          << std::endl;

      ++it;
    }

    if(it == _max_its)
    {
      mooseError("Max sub-newton iteration hit during creep solve!");
    }

// compute creep and elastic strain increments (avoid potential divide by zero - how should this be done)?
    if (effective_trial_stress < 0.01)
    {
      effective_trial_stress = 0.01;
    }
    ColumnMajorMatrix creep_strain_sub_increment( dev_trial_stress );
    creep_strain_sub_increment *= (1.5*del_p/effective_trial_stress);

    ColumnMajorMatrix elastic_strain_increment(_strain_increment*dt - creep_strain_sub_increment);

// compute stress increment
    elastic_strain_increment.reshape(9, 1);
    stress_next = *elasticityTensor() * elastic_strain_increment;

// update stress and creep strain
    stress_next.reshape(3, 3);
    stress_next += stress_last;
    stress_last = stress_next;

    creep_strain_increment += creep_strain_sub_increment;
    _del_p[_qp] += del_p;
  }
  creep_strain_increment.fill(_creep_strain[_qp]);
  _creep_strain[_qp] += _creep_strain_old[_qp];

  stress_new = stress_next;
}

void
PLC_LSH::computeLSH( ColumnMajorMatrix & stress_new,
                     ColumnMajorMatrix & creep_strain_increment )
{

  if (_t_step == 1)
  {
    _hardening_variable[_qp] =
      _hardening_variable_old[_qp] = 0;
    _plastic_strain[_qp] =
      _plastic_strain_old[_qp] = 0;
  }

// compute deviatoric trial stress
  ColumnMajorMatrix dev_trial_stress_p(stress_new);
  dev_trial_stress_p.addDiag( -stress_new.tr()/3.0 );

// effective trial stress
  Real dts_squared_p = dev_trial_stress_p.doubleContraction(dev_trial_stress_p);
  Real effective_trial_stress_p = std::sqrt(1.5 * dts_squared_p);

// determine if yield condition is satisfied
  Real yield_condition = effective_trial_stress_p - _hardening_variable_old[_qp] - _yield_stress;

  if (yield_condition > 0.)  //then use newton iteration to determine effective plastic strain increment
  {
    unsigned int jt = 0;
    Real plastic_residual = 10.;
    Real scalar_plastic_strain_increment = 0.;
    Real norm_residual = 10.;


    _hardening_variable[_qp] = _hardening_variable_old[_qp];
    while(jt < _max_its && norm_residual > _tolerance)
    {
      plastic_residual = effective_trial_stress_p - (3. * _shear_modulus * scalar_plastic_strain_increment) - _hardening_variable[_qp] - _yield_stress;
      norm_residual = std::abs(plastic_residual);

      scalar_plastic_strain_increment = scalar_plastic_strain_increment + ((plastic_residual) / (3. * _shear_modulus + _hardening_constant));

      _hardening_variable[_qp] = _hardening_variable_old[_qp] + (_hardening_constant * scalar_plastic_strain_increment);
      ++jt;

    }


    if(jt == _max_its)
    {
      mooseError("Max sub-newton iteration hit during plasticity increment solve!");
    }


// compute plastic and elastic strain increments (avoid potential divide by zero -
// how should this be done)?
// JDH: If we are in this portion of the code, effective_trial_stress_p must be
// non-zero.  The check is unnecessary.
    if (effective_trial_stress_p < 0.01) effective_trial_stress_p = 0.01;
    ColumnMajorMatrix plastic_strain_increment(dev_trial_stress_p);
    plastic_strain_increment *= (1.5*scalar_plastic_strain_increment/effective_trial_stress_p);

    ColumnMajorMatrix elastic_strain_increment_p;
    elastic_strain_increment_p = _strain_increment*_dt - plastic_strain_increment - creep_strain_increment;

//compute stress increment
    elastic_strain_increment_p.reshape(9, 1);
    stress_new = *elasticityTensor() * elastic_strain_increment_p;

// update stress and plastic strain
    stress_new.reshape(3, 3);
    stress_new += _stress_old[_qp];
    plastic_strain_increment.fill(_plastic_strain[_qp]);
    _plastic_strain[_qp] += _plastic_strain_old[_qp];

  }//end of if statement

}
