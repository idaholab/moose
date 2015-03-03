/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ChemicalOutFlowBC.h"
#include "Material.h"

template<>
InputParameters validParams<ChemicalOutFlowBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  return params;
}

ChemicalOutFlowBC::ChemicalOutFlowBC(const std::string & name, InputParameters parameters)
  :IntegratedBC(name, parameters),
   _diff(getMaterialProperty<Real>("diffusivity")),
   _porosity(getMaterialProperty<Real>("porosity"))
{
}

Real
ChemicalOutFlowBC::computeQpResidual()
{
//    RealGradient _Darcy_vel = -_cond*_grad_p[_qp];
//    Moose::out<<"Darcy velocity" << _Darcy_vel(1);
//    Moose::out << "porosity,diffusivity, cond " << _porosity <<" " << _diff <<" "<< _cond << std::endl;

  Real var = -_test[_i][_qp]*_porosity[_qp]*_diff[_qp]*_grad_u[_qp]*_normals[_qp];

//    if (var <= 1.0e-12)
//      var=0.0;

//    Moose::out << "utlet_flux " << var << std::endl;

  return var;
}


Real
ChemicalOutFlowBC::computeQpJacobian()
{
//    RealGradient _Darcy_vel = -_cond*_grad_p[_qp];
  Real var = -_test[_i][_qp]*_porosity[_qp]*_diff[_qp]*_grad_phi[_j][_qp]*_normals[_qp];

  return var;

}
