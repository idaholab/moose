// Original class author: A.M. Jokisaari, O. Heinonen

#include "FiniteStrainMaterial.h"

/**
 * FiniteStrainMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<FiniteStrainMaterial>()
{
  InputParameters params = validParams<TensorMechanicsMaterial>();

  return params;
}

FiniteStrainMaterial::FiniteStrainMaterial(const std::string & name, 
                                             InputParameters parameters)
    : TensorMechanicsMaterial(name, parameters),      
      _strain_rate(declareProperty<RankTwoTensor>("strain_rate")),  
      _strain_increment(declareProperty<RankTwoTensor>("strain_increment")),     
      _rotation_increment(declareProperty<RankTwoTensor>("rotation_increment"))
{
}

void FiniteStrainMaterial::computeStrain()
{
  //Method from Rashid, 1993
  std::vector<RankTwoTensor> Fhat;
  Fhat.resize(_qrule->n_points());
  RankTwoTensor ave_Fhat;
  Real volume(0);
  
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    //Deformation gradient
    RankTwoTensor A(_grad_disp_x[_qp],_grad_disp_y[_qp],_grad_disp_z[_qp]); //Deformation gradient
    RankTwoTensor Fbar(_grad_disp_x_old[_qp],_grad_disp_y_old[_qp],_grad_disp_z_old[_qp]); //Old Deformation gradient 
  
    A -= Fbar; //A = gradU - gradUold
    
    Fbar.addIa(1.0); //Fbar = ( I + gradUold)
  
    //Incremental deformation gradient Fhat = I + A Fbar^-1
    Fhat[_qp] = A*Fbar.inverse();
    Fhat[_qp].addIa(1.0);

    ave_Fhat += Fhat[_qp]*_JxW[_qp];
    volume += _JxW[_qp];
  }

  ave_Fhat /= volume;
  
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    Real factor( std::pow( ave_Fhat.det()/Fhat[_qp].det(), 1.0/3.0));
    Fhat[_qp] *= factor;
    
    computeQpStrain(Fhat[_qp]);
  }
  
}

void FiniteStrainMaterial::computeQpStrain(RankTwoTensor Fhat)
{

  //Cinv - I = B B^T - B - B^T;
  RankTwoTensor A(-Fhat.inverse()); //B = I - Fhatinv
  A.addIa(1.0);
  RankTwoTensor Cinv_I = A*A.transpose() - A - A.transpose();

  //strain rate D from Taylor expansion, D = 1/dt(-1/2(Chat^-1 - I) + 1/4*(Chat^-1 - I)^2 + ...
  RankTwoTensor D = (-Cinv_I/2.0 + Cinv_I*Cinv_I/4.0)/_t_step;
  _strain_rate[_qp] = D;
  _strain_increment[_qp] = D*_t_step; //This assumes a specific time integration

  //Calculate rotation Rhat
  RankTwoTensor U(-A);
  Real ax = U(1,2) - U(2,1);
  Real ay = U(2,0) - U(0,2);
  Real az = U(0,1) - U(1,0);
  Real q = (ax*ax + ay*ay + az*az)/4.0;
  Real p = (U.trace() - 1.0)*(U.trace() - 1.0)/4.0;
  Real y = 1.0/((q + p)*(q + p)*(q + p));

  Real C1 = std::sqrt(p * (1 + (p*(q+q+(q+p))) * (1-(q+p)) * y));
  Real C2 = 0.125 + q * 0.03125 * (p*p - 12*(p-1)) / (p*p);
  Real C3 = 0.5 * std::sqrt( (p*q*(3-q) + p*p*p + q*q) / (p+q)*(p+q)*(p+q) );
  
  RankTwoTensor R_incr;
  R_incr(0,0) = C1 + (C2*ax)*ax;
  R_incr(0,1) =      (C2*ay)*ax + (C3*az);
  R_incr(0,2) =      (C2*az)*ax - (C3*ay);
  R_incr(1,0) =      (C2*ax)*ay - (C3*az);
  R_incr(1,1) = C1 + (C2*ay)*ay;
  R_incr(1,2) =      (C2*az)*ay + (C3*ax);
  R_incr(2,0) =      (C2*ax)*az + (C3*ay);
  R_incr(2,1) =      (C2*ay)*az - (C3*ax);
  R_incr(2,2) = C1 + (C2*az)*az;
  
  _rotation_increment[_qp] = R_incr;
}

void FiniteStrainMaterial::computeQpStress()
{
  // stress = C * e
  RankTwoTensor unrotated_stress = _elasticity_tensor[_qp]*_strain_increment[_qp];
  //Rotate the stress to the current configuration T = R T0 R^T
  _stress[_qp] = _rotation_increment[_qp]*unrotated_stress*_rotation_increment[_qp].transpose();
  
}
