/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HeatConduction.h"

template<>
InputParameters validParams<HeatConductionKernel>()
{
  InputParameters params = validParams<Diffusion>();
  params.addClassDescription("Compute thermal conductivity "); // Add a description of what this kernel does
  params.addParam<std::string>("diffusion_coefficient_name", "thermal_conductivity", "Property name of the diffusivity (Default: thermal_conductivity");
  params.addParam<std::string>("diffusion_coefficient_dT_name", "thermal_conductivity_dT", "Property name of the derivative of the diffusivity with respect to the variable (Default: thermal_conductivity_dT");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

HeatConductionKernel::HeatConductionKernel(const std::string & name, InputParameters parameters) :
  Diffusion(name, parameters),
  _dim(_subproblem.mesh().dimension()),
  _diffusion_coefficient(getMaterialProperty<Real>(getParam<std::string>("diffusion_coefficient_name"))),
  _diffusion_coefficient_dT(hasMaterialProperty<Real>(getParam<std::string>("diffusion_coefficient_dT_name")) ? &getMaterialProperty<Real>(getParam<std::string>("diffusion_coefficient_dT_name")) : NULL)
{
}

Real
HeatConductionKernel::computeQpResidual()
{
  Real r(0);
//   r = diffusion_coefficient[_qp]*Diffusion::computeQpResidual();
//   if (!libmesh_isnan(r))
//   {
//   }
//   else
//   {
//     Moose::err << "NaN found at " << __LINE__ << " in " << __FILE__ << "!\n"
//               << "Processor: " << libMesh::processor_id() << "\n"
//               << "_diffusion_coefficient[_qp]: " << _diffusion_coefficient[_qp] << "\n"
//               << "Diffusion resid: " << Diffusion::computeQpResidual() << "\n"
//               << "Elem: " << _current_elem->id() << "\n"
//               << "Qp: " << _qp << "\n"
//               << "Qpoint: " << _q_point[_qp] << "\n"
//               << std::endl;
//   }
//   return r;
  r = _diffusion_coefficient[_qp]*Diffusion::computeQpResidual();
  return r;
}

Real
HeatConductionKernel::computeQpJacobian()
{
  Real jac(0);
  jac = _diffusion_coefficient[_qp] * Diffusion::computeQpJacobian();
  if ( _diffusion_coefficient_dT )
  {
    jac += (*_diffusion_coefficient_dT)[_qp] * _phi[_j][_qp] * Diffusion::computeQpResidual();
  }
  return jac;
}
