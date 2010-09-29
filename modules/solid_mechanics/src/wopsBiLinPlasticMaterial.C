#include "wopsBiLinPlasticMaterial.h"

#include "ElasticityTensor.h"
#include <cmath>

template<>
InputParameters validParams<wopsBiLinPlasticMaterial>()
{
   InputParameters params = validParams<LinearIsotropicMaterial>();
   params.addRequiredParam<Real>("yield_stress", "The point at which plastic strain begins accumulating");
   params.addRequiredParam<Real>("hardening_constant", "Hardening slope");
   params.addParam<Real>("tolerance", 1e-5, "Sub-BiLin iteration tolerance");
   params.addParam<unsigned int>("max_its", 10, "Maximum number of Sub-newton iterations");
   params.addParam<bool>("print_debug_info", false, "Whether or not to print debugging information");
   return params;
}

wopsBiLinPlasticMaterial::wopsBiLinPlasticMaterial(std::string name,
                                             MooseSystem & moose_system,
                                             InputParameters parameters)
  :LinearIsotropicMaterial(name, moose_system, parameters),
   _yield_stress(parameters.get<Real>("yield_stress")),
   _hardening_constant(parameters.get<Real>("hardening_constant")),
   _tolerance(parameters.get<Real>("tolerance")),
   _max_its(parameters.get<unsigned int>("max_its")),
   _print_debug_info(getParam<bool>("print_debug_info")),
   _total_strain(declareProperty<ColumnMajorMatrix>("total_strain")),
   _hardening_variable(declareProperty<Real>("hardening_variable")),
   _hardening_variable_old(declarePropertyOld<Real>("hardening_variable")),
   _plastic_strain(declareProperty<ColumnMajorMatrix>("plastic_strain")),
   _plastic_strain_old(declarePropertyOld<ColumnMajorMatrix>("plastic_strain"))
{
  _shear_modulus = _youngs_modulus / ((2*(1+_poissons_ratio)));

  _identity.identity();
}

void
wopsBiLinPlasticMaterial::computeStrain(const ColumnMajorMatrix & total_strain, ColumnMajorMatrix & elastic_strain)
{
  _total_strain[_qp] = total_strain;
            
  ColumnMajorMatrix etotal_strain(total_strain);
  
// convert total_strain from 3x3 to 9x1
  etotal_strain.reshape(LIBMESH_DIM * LIBMESH_DIM, 1);
// trial stress
  ColumnMajorMatrix trial_stress = (*_local_elasticity_tensor) * etotal_strain;
// Change 9x1 to a 3x3
  trial_stress.reshape(LIBMESH_DIM, LIBMESH_DIM);
// deviatoric trial stress
  ColumnMajorMatrix dev_trial_stress(trial_stress);
  dev_trial_stress -= _identity*((trial_stress.tr())/3.0);
// effective trial stress
  Real dts_squared = dev_trial_stress.doubleContraction(dev_trial_stress);
  Real effective_trial_stress = std::sqrt(1.5 * dts_squared);
  
// determine if yield condition is satisfied
  Real yield_condition = effective_trial_stress - _hardening_variable_old[_qp] - _yield_stress;

  if (yield_condition > 0.)  //then use newton iteration to determine effective plastic strain increment
  {    
    unsigned int it = 0;
    Real plastic_residual = 1.;
    Real plastic_strain_increment = 0;
    Real norm_residual = 10.0;
    
    while(it < _max_its && norm_residual > _tolerance)
    {
      plastic_residual = effective_trial_stress - 3. * _shear_modulus * plastic_strain_increment - _hardening_variable[_qp] - _yield_stress;
      norm_residual = std::abs(plastic_residual);
      
      plastic_strain_increment = plastic_strain_increment + plastic_residual / (3. * _shear_modulus + _hardening_constant);
      _hardening_variable[_qp] = _hardening_variable_old[_qp] + _hardening_constant * plastic_strain_increment;
      it++;
    }

    if(it == _max_its)
      mooseError("Max sub-newton iteration hit during plasticity increment solve!");

    ColumnMajorMatrix matrix_plastic_strain_increment(dev_trial_stress);
    matrix_plastic_strain_increment *= (1.5*plastic_strain_increment/effective_trial_stress);
    // update plastic strain
    _plastic_strain[_qp] = _plastic_strain_old[_qp] + matrix_plastic_strain_increment;

    // calculate elastic strain
    elastic_strain = total_strain;
    elastic_strain -= _plastic_strain[_qp];
  }
  else
  {
    elastic_strain = total_strain;
    _hardening_variable[_qp] = 0.0;
    ColumnMajorMatrix A(3,3);
    A.zero();
    _plastic_strain[_qp] = A;
  }
  
}
