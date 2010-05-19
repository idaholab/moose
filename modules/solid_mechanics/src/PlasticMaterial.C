#include "PlasticMaterial.h"

template<>
InputParameters validParams<PlasticMaterial>()
{
  InputParameters params = validParams<LinearIsotropicMaterial>();
  return params;
}

PlasticMaterial::PlasticMaterial(std::string name,
                                 MooseSystem & moose_system,
                                 InputParameters parameters)
  :LinearIsotropicMaterial(name, moose_system, parameters)
{}

void
PlasticMaterial::computeStrain(ColumnMajorMatrix & strain)
{/*
  Real volumetric_strain_invariant = strain.tr();
  
  ColumnMajorMatrix identity(1,0,0
                             0,1,0
                             0,0,1);
  
  ColumnMajorMatrix deviatoric_strain = strain - identity * (1.0/3.0 * volumetric_strain_invariant);

  ColumnMajorMatrix deviatoric_stress = deviatoric_strain * (2.0 * shear_modulus);

  Real second_invariant_of_deviatoric_stress = 0.5 * (deviatoric_stress*deviatori_stress).tr();

  Real von_mises_stress = std::sqrt(3.0 * second_invariant_of_deviatoric_stress);

  strain -= (von_mises_stress - yield_stress > 0.0 ? old_strain + ( (von_mises_stress - yield_stress) / (3.0 * shear_modulus) ) : 0);
 */
}
