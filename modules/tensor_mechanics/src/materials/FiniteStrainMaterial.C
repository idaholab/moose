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
    : TensorMechanicsMaterial(name, parameters)
{
}

void FiniteStrainMaterial::computeQpStrain()
{
  //Method from Rashid, 1993
  //Deformation gradient
  RankTwoTensor A(_grad_disp_x[_qp],_grad_disp_y[_qp],_grad_disp_z[_qp]); //Deformation gradient
  RankTwoTensor Fbar(_grad_disp_x_old[_qp],_grad_disp_y_old[_qp],_grad_disp_z_old[_qp]); //Deformation gradient

  A -= Fbar; //A = gradU - gradUold

  Fbar.addIa(1.0); //Fbar = ( I + gradUold)
  
  //Incremental deformation gradient Fhat = I + A Fbar^-1
  RankTwoTensor Fhat = A*Fbar.inverse();
  Fhat.addIa(1.0);

   //Cinv - I = B B^T - B - B^T;
  RankTwoTensor B(-Fbar.inverse()); //B = I - Fbarinv
  B.addIa(1.0);
  RankTwoTensor Cinv_I = B*B.transpose() - B - B.transpose();

  //strain rate D from Taylor expansion, D = 1/dt(-1/2(Chat^-1 - I) + 1/4*(Chat^-1 - I)^2 + ...
  RankTwoTensor D = (-Cinv_I/2.0 + Cinv_I*Cinv_I/4.0)/_t_step;

  //Calculate rotation Rhat
  RankTwoTensor U(Fhat.inverse());
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
}

void FiniteStrainMaterial::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp]*_elastic_strain[_qp];
}
