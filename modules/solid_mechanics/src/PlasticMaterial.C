#include "PlasticMaterial.h"

template<>
InputParameters validParams<PlasticMaterial>()
{
  InputParameters params = validParams<LinearIsotropicMaterial>();
  params.addRequiredParam<Real>("yield_stress", "The point at which plastic strain begins accumulating");
  params.addRequiredCoupledVar("delta_gamma", "The plastic strain increment.");
  return params;
}

PlasticMaterial::PlasticMaterial(std::string name,
                                 MooseSystem & moose_system,
                                 InputParameters parameters)
  :LinearIsotropicMaterial(name, moose_system, parameters),
   _input_yield_stress(parameters.get<Real>("yield_stress")),
   _yield_stress(declareRealProperty("yield_stress")),
   _shear_modulus(declareRealProperty("shear_modulus")),
   
   _accumulated_plastic_strain(declareRealProperty("accumulated_plastic_strain")),
   _accumulated_plastic_strain_old(declareRealPropertyOld("accumulated_plastic_strain")),
   
   _plastic_strain(declareColumnMajorMatrixProperty("plastic_strain")),
   _plastic_strain_old(declareColumnMajorMatrixPropertyOld("plastic_strain")),
   
   _von_mises_stress(declareRealProperty("von_mises_stress")),
   
   _delta_gamma(coupledVal("delta_gamma"))
{
  _input_shear_modulus = _youngs_modulus / ((2*(1+_poissons_ratio)));
}

void
PlasticMaterial::computeStrain(ColumnMajorMatrix & strain)
{
  _yield_stress[_qp] = _input_yield_stress;

  _shear_modulus[_qp] = _input_shear_modulus;

  
  Real volumetric_strain_invariant = strain.tr();

  
  ColumnMajorMatrix identity;

  identity.identity();

  
  ColumnMajorMatrix deviatoric_strain(strain);
  
  deviatoric_strain -= identity * (1.0/3.0 * volumetric_strain_invariant);
  

  ColumnMajorMatrix deviatoric_stress = deviatoric_strain * (2.0 * _shear_modulus[_qp]);

  Real second_invariant_of_deviatoric_stress = 0.5 * (deviatoric_stress*deviatoric_stress).tr();

  _von_mises_stress[_qp] = std::sqrt(3.0 * second_invariant_of_deviatoric_stress);

  
  //_plastic_strain[_qp] = (_von_mises_stress[_qp] - _yield_stress > 0.0 ? _plastic_strain_old[_qp] + ( (_von_mises_stress[_qp] - _yield_stress) / (3.0 * _shear_modulus) ) : _plastic_strain_old[_qp]);
  _accumulated_plastic_strain[_qp] = _accumulated_plastic_strain_old[_qp] + _delta_gamma[_qp];

  Real deviatoric_norm = deviatoric_stress.norm();
  
  if(deviatoric_norm)
  {
    deviatoric_stress *= (_delta_gamma[_qp] * sqrt(1.5)) / deviatoric_norm;

    _plastic_strain[_qp] = _plastic_strain_old[_qp] + deviatoric_stress;

//    _plastic_strain[_qp] += deviatoric_stress;
    /*
    if(_accumulated_plastic_strain[_qp] > 0)
    {
      std::cout<<"deviatoric   "<<deviatoric_stress.norm()<<std::endl;
      std::cout<<"full norm    "<<strain.norm()<<std::endl;
      std::cout<<"plastic old  "<<_plastic_strain_old[_qp].norm()<<std::endl;
      std::cout<<"plastic norm "<<_plastic_strain[_qp].norm()<<std::endl;
    }
    */
    strain -= _plastic_strain[_qp];
    /*
    if(_accumulated_plastic_strain[_qp] > 0)
      std::cout<<"modified norm"<<strain.norm()<<std::endl;
    */
  }
}
