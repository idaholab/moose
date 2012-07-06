#include "GapHeatTransfer.h"
#include "GapConductance.h"

#include "SystemBase.h"

Threads::spin_mutex slave_flux_mutex;

template<>
InputParameters validParams<GapHeatTransfer>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredCoupledVar("gap_distance", "Distance across the gap");
  params.addRequiredCoupledVar("gap_temp", "Temperature on the other side of the gap");
  params.addRequiredCoupledVar("gap_conductance", "Variable to hold the gap conductance");
  params.addRequiredCoupledVar("gap_conductance_dT", "Variable to hold the derivative of gap conductance wrt temperature");
  params.addParam<Real>("min_gap", 1.0e-6, "A minimum gap size");
  params.addParam<Real>("max_gap", 1.0e6, "A maximum gap size");
  return params;
}

GapHeatTransfer::GapHeatTransfer(const std::string & name, InputParameters parameters)
  :IntegratedBC(name, parameters),
   _slave_flux(_sys.getVector("slave_flux")),
   _gap_distance(coupledValue("gap_distance")),
   _gap_temp(coupledValue("gap_temp")),
   _gap_conductance(getMaterialProperty<Real>("gap_conductance")),
   _gap_conductance_dT(getMaterialProperty<Real>("gap_conductance_dT")),
   _min_gap(getParam<Real>("min_gap")),
   _max_gap(getParam<Real>("max_gap")),
   _xdisp_coupled(isCoupled("disp_x")),
   _ydisp_coupled(isCoupled("disp_y")),
   _zdisp_coupled(isCoupled("disp_z")),
   _xdisp_var(_xdisp_coupled ? coupled("disp_x") : 0),
   _ydisp_var(_ydisp_coupled ? coupled("disp_y") : 0),
   _zdisp_var(_zdisp_coupled ? coupled("disp_z") : 0)
{
}


Real
GapHeatTransfer::computeQpResidual()
{
  Real grad_t = (_u[_qp] - _gap_temp[_qp]) * _gap_conductance[_qp];

  // This is keeping track of this residual contribution so it can be used as the flux on the other side of the gap.
  {
    Threads::spin_mutex::scoped_lock lock(slave_flux_mutex);
    const Real slave_flux = computeSlaveFluxContribution(grad_t);
    _slave_flux.add(_var.dofIndices()[_i], slave_flux);
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
GapHeatTransfer::computeSlaveFluxContribution( Real grad_t )
{
  return _coord[_qp] * _JxW[_qp] * _test[_i][_qp] * grad_t;
}

Real
GapHeatTransfer::computeQpJacobian()
{
  return _test[_i][_qp] * ((_u[_qp] - _gap_temp[_qp]) * _gap_conductance_dT[_qp] + _gap_conductance[_qp]) * _phi[_j][_qp];
}

Real
GapHeatTransfer::computeQpOffDiagJacobian( unsigned jvar )
{
  unsigned coupled_component(0);
  bool active(false);
  if ( _xdisp_coupled && jvar == _xdisp_var )
  {
    coupled_component = 0;
    active = true;
  }
  else if ( _ydisp_coupled && jvar == _ydisp_var )
  {
    coupled_component = 1;
    active = true;
  }
  else if ( _zdisp_coupled && jvar == _zdisp_var )
  {
    coupled_component = 2;
    active = true;
  }

  Real dRdx(0);
  if ( active )
  {
    // Compute dR/du_[xyz]
    // Residual is based on
    //   h_gap = h_conduction() + h_contact() + h_radiation();
    //   grad_t = (_u[_qp] - _gap_temp[_qp]) * h_gap;
    // So we need
    //   (_u[_qp] - _gap_temp[_qp]) * (dh_gap/du_[xyz]);
    // Assuming dh_contact/du_[xyz] = dh_radiation/du_[xyz] = 0,
    //   we need dh_conduction/du_[xyz]
    // Given
    //   h_conduction = gapK / gapLength, then
    //   dh_conduction/du_[xyz] = -gapK/gapLength^2 * dgapLength/du_[xyz]
    // Given
    //   gapLength = ((u_x-m_x)^2+(u_y-m_y)^2+(u_z-m_z)^2)^1/2
    // where m_[xyz] is the master coordinate, then
    //   dGapLength/du_[xyz] = 1/2*((u_x-m_x)^2+(u_y-m_y)^2+(u_z-m_z)^2)^(-1/2)*2*(u_[xyz]-m_[xyz])
    //                       = (u_[xyz]-m_[xyz])/gapLength
    // This is the normal vector.

    const Real gapL = gapLength();

    // THIS IS NOT THE NORMAL WE NEED.
    // WE NEED THE NORMAL FROM THE CONSTRAINT, THE NORMAL FROM THE
    // MASTER SURFACE.  HOWEVER, THIS IS TRICKY SINCE THE NORMAL
    // FROM THE MASTER SURFACE WAS COMPUTED FOR A POINT ASSOCIATED
    // WITH A SLAVE NODE.  NOW WE ARE AT A SLAVE INTEGRATION POINT.
    //
    // HOW DO WE GET THE NORMAL WE NEED?
    //
    // Until we have the normal we need,
    //   we'll hope that the one we have is close to the negative of the one we need.
    const Point & normal( _normals[_qp] );

    const Real dgap = dgapLength( -normal(coupled_component) );
    dRdx = -(_u[_qp]-_gap_temp[_qp])*_gap_conductance[_qp]/gapL * dgap;
  }
  return _test[_i][_qp] * dRdx * _phi[_j][_qp];
}


Real
GapHeatTransfer::gapLength() const
{
  return GapConductance::gapLength( -(_gap_distance[_qp]), _min_gap, _max_gap );
}

Real
GapHeatTransfer::dgapLength( Real normalComponent ) const
{
  const Real gap_L = gapLength();

  Real dgap(0);

  if ( _min_gap <= gap_L && gap_L <= _max_gap)
  {
    dgap = normalComponent;
  }

  return dgap;
}

