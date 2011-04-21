#include "GapHeatTransfer.h"
#include "SystemBase.h"

Threads::spin_mutex slave_flux_mutex;

template<>
InputParameters validParams<GapHeatTransfer>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredCoupledVar("gap_distance", "Distance across the gap");
  params.addRequiredCoupledVar("gap_temp", "Temperature on the other side of the gap");
  params.addParam<Real>("gap_conductivity", 1.0, "The thermal conductivity of the gap material");
  params.addParam<Real>("min_gap", 1.0e-6, "A minimum gap size");
  params.addParam<Real>("max_gap", 1.0e6, "A maximum gap size");
  return params;
}

GapHeatTransfer::GapHeatTransfer(const std::string & name, InputParameters parameters)
  :IntegratedBC(name, parameters),
   _slave_flux(_sys.getVector("slave_flux")),
   _gap_distance(coupledValue("gap_distance")),
   _gap_temp(coupledValue("gap_temp")),
   _gap_conductivity(getParam<Real>("gap_conductivity")),
   _min_gap(getParam<Real>("min_gap")),
   _max_gap(getParam<Real>("max_gap"))
{
}


Real
GapHeatTransfer::computeQpResidual()
{
  Real h_gap = 0.0;
  Real grad_t = 0.0;

  h_gap = h_conduction() + h_contact() + h_radiation();
  grad_t = (_u[_qp] - _gap_temp[_qp]) * h_gap;

  // This is keeping track of this residual contribution so it can be used as the flux on the other side of the gap.
  {
    Threads::spin_mutex::scoped_lock lock(slave_flux_mutex);
    _slave_flux.add(_var.dofIndices()[_i], _JxW[_qp]*_phi[_i][_qp]*grad_t);
  }

//   if (!libmesh_isnan(grad_t))
//   {
//   }
//   else
//   {
//     std::cerr << "NaN found at " << __LINE__ << " in " << __FILE__ << "!\n"
//               << "Processor: " << libMesh::processor_id() << "\n"
//               << "_u[_qp]: " << _u[_qp] << "\n"
//               << "_gap_temp[_qp]: " << _gap_temp[_qp] << "\n"
//               << "h_gap: " << h_gap << "\n"
//               << "h_conduction(): " << h_conduction() << "\n"
//               << "Elem: " << _current_elem->id() << "\n"
//               << "Qp: " << _qp << "\n"
//               << "Qpoint: " << _q_point[_qp] << "\n"
//               << std::endl;
//   }

  return _test[_i][_qp]*grad_t;
}


Real
GapHeatTransfer::computeQpJacobian()
{
  Real h_gap = h_conduction() + h_contact() + h_radiation();
//   Real dh_gap = dh_conduction() + dh_contact() + dh_radiation();
//   Real dgrad_t = ((_u[_qp] - _gap_temp[_qp]) * dh_gap + h_gap) * _phi[_j][_qp];

//   return _test[_i][_qp]*dgrad_t;
  return _test[_i][_qp] * h_gap * _phi[_j][_qp];
}


Real
GapHeatTransfer::h_conduction()
{
  Real gap_L = gapLength();

  return gapK()/gap_L;  
}


Real
GapHeatTransfer::h_contact()
{
  return 0.0;
}


Real
GapHeatTransfer::h_radiation()
{
  return 0.0;
}


Real
GapHeatTransfer::dh_conduction()
{
  return 0;
}


Real
GapHeatTransfer::dh_contact()
{
  return 0.0;
}


Real
GapHeatTransfer::dh_radiation()
{
  return 0.0;
}


Real
GapHeatTransfer::gapLength()
{
  Real gap_L = -(_gap_distance[_qp]);

  if(gap_L > _max_gap)
  {
    gap_L = _max_gap;
  }
  
  gap_L = std::max(_min_gap, gap_L);
  
  return gap_L;
}


Real
GapHeatTransfer::gapK()
{
  return _gap_conductivity;
}

