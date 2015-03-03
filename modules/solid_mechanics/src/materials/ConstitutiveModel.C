/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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

  params.addParam<Real>("thermal_expansion", "The thermal expansion coefficient.");
  params.addParam<FunctionName>("thermal_expansion_function", "Thermal expansion coefficient as a function of temperature.");
  params.addParam<Real>("stress_free_temperature", "The stress-free temperature.  If not specified, the initial temperature is used.");

  return params;
}


ConstitutiveModel::ConstitutiveModel( const std::string & name,
                                      InputParameters parameters )
  :Material( name, parameters ),
   _has_temp(isCoupled("temp")),
   _temperature(_has_temp ? coupledValue("temp") : _zero),
   _temperature_old(_has_temp ? coupledValueOld("temp") : _zero),
   _alpha(parameters.isParamValid("thermal_expansion") ? getParam<Real>("thermal_expansion") : 0.),
   _alpha_function( parameters.isParamValid("thermal_expansion_function") ? &getFunction("thermal_expansion_function") : NULL),
   _has_stress_free_temp(isParamValid("stress_free_temperature")),
   _stress_free_temp(_has_stress_free_temp ? getParam<Real>("stress_free_temperature") : 0.0)
{}

void
ConstitutiveModel::computeStress( const Elem & /*current_elem*/,
                                  unsigned /*qp*/,
                                  const SymmElasticityTensor & elasticityTensor,
                                  const SymmTensor & stress_old,
                                  SymmTensor & strain_increment,
                                  SymmTensor & stress_new )
{
  stress_new = elasticityTensor * strain_increment;
  stress_new += stress_old;
}

void
ConstitutiveModel::initStatefulProperties( unsigned int /*n_points*/ )
{
}

bool
ConstitutiveModel::applyThermalStrain(unsigned qp,
                                      SymmTensor & strain_increment,
                                      SymmTensor & d_strain_dT)
{
  bool modified = false;
  if ( _has_temp && _t_step != 0 )
  {
    Real tStrain;
    Real alpha(_alpha);
    if (_alpha_function)
    {
      Point p;
      alpha = _alpha_function->value(_temperature[qp],p);
    }
    if (alpha != 0)
    {
      modified = true;
    }
    if (_t_step == 1 && _has_stress_free_temp)
    {
      tStrain = alpha * (_temperature[qp] - _stress_free_temp);
    }
    else
    {
      tStrain = alpha * (_temperature[qp] - _temperature_old[qp]);
    }
    strain_increment.addDiag( -tStrain );
    d_strain_dT.addDiag( -alpha );
  }
  return modified;
}
