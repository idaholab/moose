#include "HeatConduction.h"

template<>
InputParameters validParams<HeatConductionKernel>()
{
  InputParameters params = validParams<Diffusion>();
  params.addClassDescription("Compute thermal conductivity "); // Add a description of what this kernel does
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

HeatConductionKernel::HeatConductionKernel(const std::string & name, InputParameters parameters) :
  Diffusion(name, parameters),
  _dim(_subproblem.mesh().dimension()),
  _k(getMaterialProperty<Real>("thermal_conductivity")),
  _k_dT(hasMaterialProperty<Real>("thermal_conductivity_dT") ? &getMaterialProperty<Real>("thermal_conductivity_dT") : NULL)
{
}

Real
HeatConductionKernel::computeQpResidual()
{
  Real r(0);
//   r = _k[_qp]*Diffusion::computeQpResidual();
//   if (!libmesh_isnan(r))
//   {
//   }
//   else
//   {
//     Moose::err << "NaN found at " << __LINE__ << " in " << __FILE__ << "!\n"
//               << "Processor: " << libMesh::processor_id() << "\n"
//               << "_k[_qp]: " << _k[_qp] << "\n"
//               << "Diffusion resid: " << Diffusion::computeQpResidual() << "\n"
//               << "Elem: " << _current_elem->id() << "\n"
//               << "Qp: " << _qp << "\n"
//               << "Qpoint: " << _q_point[_qp] << "\n"
//               << std::endl;
//   }
//   return r;
  r = _k[_qp]*Diffusion::computeQpResidual();
  return r;
}



Real
HeatConductionKernel::computeQpJacobian()
{
  Real jac(0);
  jac = _k[_qp] * Diffusion::computeQpJacobian();
  if ( _k_dT )
  {
    jac += (*_k_dT)[_qp] * _phi[_j][_qp] * Diffusion::computeQpResidual();
  }
  return jac;
}
