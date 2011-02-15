#include "LSHPlasticMaterial.h"

#include "ElasticityTensor.h"
#include <cmath>

template<>
InputParameters validParams<LSHPlasticMaterial>()
{
   InputParameters params = validParams<LinearIsotropicMaterial>();
   params.addRequiredParam<Real>("yield_stress", "The point at which plastic strain begins accumulating");
   params.addRequiredParam<Real>("hardening_constant", "Hardening slope");
   params.addParam<Real>("tolerance", 1e-5, "Sub-BiLin iteration tolerance");
   params.addParam<unsigned int>("max_its", 10, "Maximum number of Sub-newton iterations");
   params.addParam<bool>("print_debug_info", false, "Whether or not to print debugging information");
   return params;
}

LSHPlasticMaterial::LSHPlasticMaterial(std::string name,
                                             InputParameters parameters)
  :LinearIsotropicMaterial(name, parameters),
   _yield_stress(parameters.get<Real>("yield_stress")),
   _hardening_constant(parameters.get<Real>("hardening_constant")),
   _tolerance(parameters.get<Real>("tolerance")),
   _max_its(parameters.get<unsigned int>("max_its")),
   _print_debug_info(getParam<bool>("print_debug_info")),
   _total_strain(declareProperty<ColumnMajorMatrix>("total_strain")),
   _total_strain_old(declarePropertyOld<ColumnMajorMatrix>("total_strain")),
   _stress(declareProperty<RealTensorValue>("stress")),
   _stress_old(declarePropertyOld<RealTensorValue>("stress")),
   _hardening_variable(declareProperty<Real>("hardening_variable")),
   _hardening_variable_old(declarePropertyOld<Real>("hardening_variable")),
   _plastic_strain(declareProperty<RealTensorValue>("plastic_strain")),
   _plastic_strain_old(declarePropertyOld<RealTensorValue>("plastic_strain"))
{
  _shear_modulus = _youngs_modulus / ((2*(1+_poissons_ratio)));

  _identity.identity();

  _ebulk3 = _youngs_modulus/(1. - 2.*_poissons_ratio);

  _K = _ebulk3/3.;
  
}

void
LSHPlasticMaterial::computeStrain(const ColumnMajorMatrix & total_strain, ColumnMajorMatrix & elastic_strain)
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
  
// determine if yield condition is satisfied
  Real yield_condition = effective_trial_stress - _hardening_variable_old[_qp] - _yield_stress;

  if (yield_condition > 0.)  //then use newton iteration to determine effective plastic strain increment
  {    
    unsigned int it = 0;
    Real plastic_residual = 10.;
    Real plastic_strain_increment = 0.;
    Real norm_residual = 10.;
    
    
    
    while(it < _max_its && norm_residual > _tolerance)
    {
      plastic_residual = effective_trial_stress - (3. * _shear_modulus * plastic_strain_increment) - _hardening_variable[_qp] - _yield_stress;
      norm_residual = std::abs(plastic_residual);
      
      plastic_strain_increment = plastic_strain_increment + ((plastic_residual) / (3. * _shear_modulus + _hardening_constant));
      
      _hardening_variable[_qp] = _hardening_variable_old[_qp] + (_hardening_constant * plastic_strain_increment);
      it++;
       
    }
    

    if(it == _max_its)
      mooseError("Max sub-newton iteration hit during plasticity increment solve!");

    ColumnMajorMatrix matrix_plastic_strain_increment(dev_trial_stress);
    matrix_plastic_strain_increment *= (1.5*plastic_strain_increment/effective_trial_stress);
    
    // update plastic strain
  matrix_plastic_strain_increment.fill(_plastic_strain[_qp]);
  _plastic_strain[_qp] += _plastic_strain_old[_qp];

    // calculate elastic strain
    elastic_strain = etotal_strain;
    elastic_strain.reshape(LIBMESH_DIM, LIBMESH_DIM);
    elastic_strain -= matrix_plastic_strain_increment;
    

    

//calculate Jacobian
//calculate R
    double R = (effective_trial_stress - 3.*_shear_modulus*plastic_strain_increment)/effective_trial_stress;

//calculate Q
    double Q = 1.5*(1/(1+(3*_shear_modulus/_hardening_constant))-R);
    unsigned int i, j, k, l;
    i = j = k = l = 0;
    double devdev[3][3][3][3];
        for (l=0;l<3;l++)
          for (k=0;k<3;k++)
            for (j=0;j<3;j++)
              for (i=0;i<3;i++)
              {
                devdev[i][j][k][l] = dev_trial_stress(i,j) * dev_trial_stress(k,l);
              }

 //define identity matrix
     double xiden[3][3];
         for (i=0;i<3;i++)
           for (j=0;j<3;j++)
           {
             i==j ? xiden[i][j] = 1. : xiden[i][j] = 0.;
           }

//now the 3x3 identity X the 3x3 identity Iikjl
    double IIdent[3][3][3][3];
        for (l=0;l<3;l++)
          for (k=0;k<3;k++)
            for (j=0;j<3;j++)
              for (i=0;i<3;i++)
              {
               IIdent[i][j][k][l] = xiden[i][j]*xiden[k][l];            
              }

//define bold I identity matrix
    double boldI[3][3][3][3];
        for (l=0;l<3;l++)
          for (k=0;k<3;k++)
            for (j=0;j<3;j++)
              for (i=0;i<3;i++)
              {
                i==k && j==l ? boldI[i][j][k][l] = 1. : boldI[i][j][k][l] = 0.;
              }

//the Jacobian partial del sig wrt partial del eps pds/pde
    double Jac[3][3][3][3];
        for (l=0;l<3;l++)
          for (k=0;k<3;k++)
            for (j=0;j<3;j++)
              for (i=0;i<3;i++)
              {
                Jac[i][j][k][l] = (2*_shear_modulus*Q/(effective_trial_stress*effective_trial_stress)) * devdev[i][j][k][l] + 2*_shear_modulus*R*boldI[i][j][k][l] + (_K - 2*_shear_modulus*R/3)*IIdent[i][j][k][l];
              }

//now define _Jacobian_mult[_qp], which is a colummajormatrix, in terms of Jac[i][j][k][l]
    double Jac9x9[9][9];
    _Jacobian_mult[_qp].reshape(LIBMESH_DIM * LIBMESH_DIM, LIBMESH_DIM * LIBMESH_DIM);
            for (l=0;l<3;l++)
              for (k=0;k<3;k++)
                for (j=0;j<3;j++)          
                  for (i=0;i<3;i++)
                  {
                    unsigned int n = l*3 + k;
                    unsigned int m = j*3 + i;
                    Jac9x9[n][m] = Jac[i][j][k][l];
                    _Jacobian_mult[_qp](n,m) = Jac[i][j][k][l];
                  }
//    _Jacobian_mult[_qp] = *_local_elasticity_tensor;
            
//end of if    
  }
  
  else
  {
    elastic_strain = etotal_strain;
    _hardening_variable[_qp] = 0.0;
    ColumnMajorMatrix A(3,3);
    A.zero();
    A.fill(_plastic_strain[_qp]);
    _Jacobian_mult[_qp] = *_local_elasticity_tensor;
  }

  
//end of computeStrain  
}

//computeStress
void
LSHPlasticMaterial::computeStress(const RealVectorValue & x, const RealVectorValue & y, const RealVectorValue & z, RealTensorValue & stress)
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

