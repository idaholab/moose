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
  Real h_gap = 0.0;
  Real grad_t = 0.0;

  h_gap = h_conduction() + h_contact() + h_radiation();
  grad_t = (_u[_qp] - _gap_temp[_qp]) * h_gap;

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
  return _JxW[_qp] * _test[_i][_qp] * grad_t;
}

Real
GapHeatTransfer::computeQpJacobian()
{
  Real h_gap = h_conduction() + h_contact() + h_radiation();
  Real dh_gap = dh_conduction() + dh_contact() + dh_radiation();
  Real dgrad_t = ((_u[_qp] - _gap_temp[_qp]) * dh_gap + h_gap) * _phi[_j][_qp];

  return _test[_i][_qp]*dgrad_t;
//   return _test[_i][_qp] * h_gap * _phi[_j][_qp];
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
    // THIS IS NOT THE NORMAL WE WANT.
    // WE NEED THE NORMAL FROM THE CONSTRAINT, THE NORMAL FROM THE MASTER SURFACE.
    // HOW TO GET IT?
    //
    // Until we have the normal we want,
    //   we'll hope that the one we have is close to the negative of the one we want.
    const Point & normal( _normals[_qp] );
    dRdx = -gapK()/(gapL*gapL) * (-(normal(coupled_component)));
  }
  return _test[_i][_qp] * dRdx * _phi[_i][_qp];
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
GapHeatTransfer::gapLength() const
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

