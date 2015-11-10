/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Compute2DFiniteStrain.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<Compute2DFiniteStrain>()
{
  InputParameters params = validParams<ComputeFiniteStrain>();
  params.addClassDescription("Compute a strain increment and rotation increment for finite strains in 2D geometries.");
  return params;
}

Compute2DFiniteStrain::Compute2DFiniteStrain(const InputParameters & parameters) :
    ComputeFiniteStrain(parameters)
{
}

void
Compute2DFiniteStrain::computeProperties()
{
  //Method from Rashid, 1993
  std::vector<RankTwoTensor> Fhat;
  Fhat.resize(_qrule->n_points());
  RankTwoTensor ave_Fhat;
  Real volume(0);
  Real ave_dfgrd_det;

  ave_dfgrd_det=0.0;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    //Deformation gradient calculation in cylinderical coordinates
    RankTwoTensor A; //Deformation gradient
    RankTwoTensor Fbar; //Old Deformation gradient
    A.zero();
    Fbar.zero();

    //Step through calculating the current and old deformation gradients
    //  Note: x_disp is the radial displacement, y_disp is the axial displacement
    for ( unsigned int j = 0; j < 2; ++j)
    {
      A(0,j) = (*_grad_disp[0])[_qp](j);
      Fbar(0,j) = (*_grad_disp_old[0])[_qp](j);
      A(1,j) = (*_grad_disp[1])[_qp](j);
      Fbar(1,j) = (*_grad_disp_old[1])[_qp](j);
    }
    A(2,2) = computeDeformGradZZ();
    Fbar(2,2) = computeDeformGradZZold();

    _deformation_gradient[_qp] = A;
    _deformation_gradient[_qp].addIa(1.0);//Gauss point deformation gradient

    A -= Fbar; //very nearly A = gradU - gradUold, adapted to cylinderical coords

    Fbar.addIa(1.0); //Fbar = ( I + gradUold)

    //Incremental deformation gradient Fhat = I + A Fbar^-1
    Fhat[_qp] = A * Fbar.inverse();
    Fhat[_qp].addIa(1.0);

    //Calculate average Fhat for volumetric locking correction
    ave_Fhat += Fhat[_qp] * _JxW[_qp];
    volume += _JxW[_qp];

    ave_dfgrd_det += _deformation_gradient[_qp].det() * _JxW[_qp]; //Average deformation gradient
  }

  ave_Fhat /= volume; //This is needed for volumetric locking correction
  ave_dfgrd_det /=volume; //Average deformation gradient

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    Real factor( std::pow( ave_Fhat.det() / Fhat[_qp].det(), 1.0/3.0));
    Fhat[_qp] *= factor; //Finalize volumetric locking correction

    computeQpStrain(Fhat[_qp]);

    factor = std::pow(ave_dfgrd_det / _deformation_gradient[_qp].det(), 1.0/3.0);//Volumetric locking correction
    _deformation_gradient[_qp] *= factor;//Volumetric locking correction
  }
}
