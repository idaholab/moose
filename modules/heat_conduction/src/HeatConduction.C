#include "HeatConduction.h"

template<>
InputParameters validParams<HeatConductionKernel>()
{
  InputParameters params = validParams<Diffusion>();
  params.addClassDescription("TODO"); // Add a description of what this kernel does
  return params;
}

HeatConductionKernel::HeatConductionKernel(const std::string & name, InputParameters parameters)
  :Diffusion(name, parameters),
   _k(getMaterialProperty<Real>("thermal_conductivity")),
   _has_k_dT(hasMaterialProperty<Real>("thermal_conductivity_dT")),
   _k_dT(_has_k_dT ? &getMaterialProperty<Real>("thermal_conductivity_dT") : NULL)
{}

Real
HeatConductionKernel::computeQpResidual()
{
//   const Real r = _k[_qp]*Diffusion::computeQpResidual();
//   if (!libmesh_isnan(r))
//   {
//   }
//   else
//   {
//     std::cerr << "NaN found at " << __LINE__ << " in " << __FILE__ << "!\n"
//               << "Processor: " << libMesh::processor_id() << "\n"
//               << "_k[_qp]: " << _k[_qp] << "\n"
//               << "Diffusion resid: " << Diffusion::computeQpResidual() << "\n"
//               << "Elem: " << _current_elem->id() << "\n"
//               << "Qp: " << _qp << "\n"
//               << "Qpoint: " << _q_point[_qp] << "\n"
//               << std::endl;
//   }
//   return r;
  return _k[_qp]*Diffusion::computeQpResidual();
}



Real
HeatConductionKernel::computeQpJacobian()
{
  Real jac = _k[_qp] * Diffusion::computeQpJacobian();
  if ( _has_k_dT )
  {
    jac += (*_k_dT)[_qp] * _phi[_j][_qp] * Diffusion::computeQpResidual();
  }
  return jac;
}
