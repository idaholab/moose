#include "PowerLawCreepMaterial.h"

#include "ElasticityTensor.h"
#include <cmath>

template<>
InputParameters validParams<PowerLawCreepMaterial>()
{
   InputParameters params = validParams<LinearIsotropicMaterial>();
   /*
     Power-law creep material parameters
   */
   params.addRequiredParam<Real>("coefficient", "Leading coefficent in power-law equation");
   params.addRequiredParam<Real>("exponent", "Exponent in power-law equation");
   params.addRequiredParam<Real>("activation_energy", "Activation energy");
   params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");
   /*
    Iteration control parameters
   */
   params.addParam<Real>("tolerance", 1e-5, "Convergence tolerance for sub-newtion iteration");
   params.addParam<unsigned int>("max_its", 10, "Maximum number of sub-newton iterations");
   params.addParam<bool>("output_iteration_info", false, "Set true to output sub-newton iteration information");
   /*
    Coupled variables
   */
   params.addCoupledVar("temp", "Coupled temperature");
   
   return params;
}

PowerLawCreepMaterial::PowerLawCreepMaterial(std::string name,
                                             InputParameters parameters)
  :LinearIsotropicMaterial(name, parameters),
   _coefficient(parameters.get<Real>("coefficient")),
   _exponent(parameters.get<Real>("exponent")),
   _activation_energy(parameters.get<Real>("activation_energy")),
   _gas_constant(parameters.get<Real>("gas_constant")),
   _tolerance(parameters.get<Real>("tolerance")),
   _max_its(parameters.get<unsigned int>("max_its")),
   _output_iteration_info(getParam<bool>("output_iteration_info")),
 
   
   _has_temp(isCoupled("temp")),
   _temp(_has_temp ? coupledValue("temp") : _zero),
   
   _total_strain(declareProperty<ColumnMajorMatrix>("total_strain")),
   _total_strain_old(declarePropertyOld<ColumnMajorMatrix>("total_strain")),
   _stress(declareProperty<RealTensorValue>("stress")),
   _stress_old(declarePropertyOld<RealTensorValue>("stress")),
   _plastic_strain(declareProperty<ColumnMajorMatrix>("plastic_strain")),
   _plastic_strain_old(declarePropertyOld<ColumnMajorMatrix>("plastic_strain"))
{
  _shear_modulus = _youngs_modulus / ((2*(1+_poissons_ratio)));
  _identity.identity();  
}

void
PowerLawCreepMaterial::computeStrain(const ColumnMajorMatrix & total_strain, ColumnMajorMatrix & elastic_strain)
{
  _total_strain[_qp] = total_strain;
  
  ColumnMajorMatrix etotal_strain(total_strain);
  etotal_strain -= _total_strain_old[_qp];  

  ColumnMajorMatrix stress_old_b(_stress_old[_qp]);
  
// convert total_strain from 3x3 to 9x1
  etotal_strain.reshape(LIBMESH_DIM * LIBMESH_DIM, 1);
// trial stress
  ColumnMajorMatrix trial_stress = (*_local_elasticity_tensor) * etotal_strain;
// Change 9x1 to a 3x3
  trial_stress.reshape(LIBMESH_DIM, LIBMESH_DIM);
  trial_stress += stress_old_b;
  
  // deviatoric trial stress
  ColumnMajorMatrix dev_trial_stress(trial_stress);
  dev_trial_stress -= _identity*((trial_stress.tr())/3.0);
// effective trial stress
  Real dts_squared = dev_trial_stress.doubleContraction(dev_trial_stress);
  Real effective_trial_stress = std::sqrt(1.5 * dts_squared);

  // iteration output
  if (_output_iteration_info == true)
    std::cout
      <<std::endl
      <<"time=" <<_t
      <<" tot_strn(0,0)=" << _total_strain[_qp](0,0)
      <<" tot_strn(1,1)=" << _total_strain[_qp](1,1)
      <<" tot_strn(2,2)=" << _total_strain[_qp](2,2) 
      <<" eff_trial_stress=" <<effective_trial_stress
      << std::endl;
      
 
// Use Newton sub-iteration to determine effective creep strain increment
  
    unsigned int it = 0;
    Real plastic_residual = 10.;
    Real plastic_strain_increment = 0.;
    Real norm_residual = 10.;
    
    while(it < _max_its && norm_residual > _tolerance)
    {
 
      Real phi = _coefficient*std::pow(effective_trial_stress - 3.*_shear_modulus*plastic_strain_increment, _exponent)*
        std::exp(-_activation_energy/(_gas_constant*_temp[_qp]));       
      Real dphi_ddelp = -3.*_coefficient*_shear_modulus*_exponent*
        std::pow(effective_trial_stress-3.*_shear_modulus*plastic_strain_increment, _exponent-1.)*
        std::exp(-_activation_energy/(_gas_constant*_temp[_qp]));
      plastic_residual = phi -  plastic_strain_increment/_dt;
      norm_residual = std::abs(plastic_residual);
      plastic_strain_increment = plastic_strain_increment + (plastic_residual / (1/_dt - dphi_ddelp));

      // iteration output
      if (_output_iteration_info == true)
        std::cout
          <<" it=" <<it
          <<" phi=" <<phi
          <<" dphi=" <<dphi_ddelp
          <<" plas_res=" <<plastic_residual
          <<" plas_strn_inc=" <<plastic_strain_increment
          <<std::endl;
      
      it++;
    } 

    if(it == _max_its) mooseError("Max sub-newton iteration hit during creep solve!");

    // Avoid potential divide by zero - how should this be done?   
    if (effective_trial_stress < 0.01) effective_trial_stress = 0.01;
    
    ColumnMajorMatrix matrix_plastic_strain_increment(dev_trial_stress);
    matrix_plastic_strain_increment *= (1.5*plastic_strain_increment/effective_trial_stress);
    
    // update plastic strain
    _plastic_strain[_qp] = _plastic_strain_old[_qp] + matrix_plastic_strain_increment;

    // calculate elastic strain
    elastic_strain = etotal_strain;
    elastic_strain -= matrix_plastic_strain_increment;
    
    //  Jacobian multiplier of the stress    
    _Jacobian_mult[_qp] = *_local_elasticity_tensor;
    
}

//computeStress
void
PowerLawCreepMaterial::computeStress(const RealVectorValue & x, const RealVectorValue & y,
                                     const RealVectorValue & z, RealTensorValue & stress)
{
  ColumnMajorMatrix total_strain(x,y,z);
  
  // 1/2 * (strain + strain^T)
  total_strain += total_strain.transpose();
  total_strain *= 0.5;
  
  // Add in any extra strain components
  ColumnMajorMatrix elastic_strain;

  computeStrain(total_strain, elastic_strain);

  // Save that off as the elastic strain
  _elastic_strain[_qp] = elastic_strain;


  // Create column vector
  elastic_strain.reshape(LIBMESH_DIM * LIBMESH_DIM, 1);

  // C * e
  ColumnMajorMatrix stress_vector = (*_local_elasticity_tensor) * elastic_strain;

  // Change 9x1 to a 3x3
  stress_vector.reshape(LIBMESH_DIM, LIBMESH_DIM);

  ColumnMajorMatrix stress_old_e(_stress_old[_qp]);
  stress_vector += stress_old_e;
  

  // Fill the material properties
  stress_vector.fill(stress);
}

