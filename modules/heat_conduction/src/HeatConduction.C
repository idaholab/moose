#include "HeatConduction.h"

template<>
InputParameters validParams<HeatConduction>()
{
  InputParameters params = validParams<Diffusion>();
  params.addClassDescription("TODO"); // Add a description of what this kernel does
  return params;
}

HeatConduction::HeatConduction(const std::string & name, InputParameters parameters)
  :Diffusion(name, parameters),
   _k(getMaterialProperty<Real>("thermal_conductivity"))
  {}

Real
HeatConduction::computeQpResidual()
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
HeatConduction::computeQpJacobian()
{
  return _k[_qp]*Diffusion::computeQpJacobian();
}
