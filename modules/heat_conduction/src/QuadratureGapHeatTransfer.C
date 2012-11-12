#include "QuadratureGapHeatTransfer.h"

#include "GapConductance.h"
#include "PenetrationLocator.h"
#include "SystemBase.h"

// libmesh
#include "string_to_enum.h"

template<>
InputParameters validParams<QuadratureGapHeatTransfer>()
{
  MooseEnum orders("FIRST, SECOND, THIRD, FOURTH", "FIRST");

  InputParameters params = validParams<IntegratedBC>();
  params.addParam<Real>("min_gap", 1.0e-6, "A minimum gap size");
  params.addParam<Real>("max_gap", 1.0e6, "A maximum gap size");
  params.addRequiredParam<BoundaryName>("paired_boundary", "The boundary to be penetrated");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>("warnings", false, "Whether to output warning messages concerning nodes not being found");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

QuadratureGapHeatTransfer::QuadratureGapHeatTransfer(const std::string & name, InputParameters parameters)
  :IntegratedBC(name, parameters),
   _gap_conductance(getMaterialProperty<Real>("gap_conductance")),
   _gap_conductance_dT(getMaterialProperty<Real>("gap_conductance_dT")),
   _min_gap(getParam<Real>("min_gap")),
   _max_gap(getParam<Real>("max_gap")),
   _gap_temp(0),
   _gap_distance(88888),
   _has_info(false),
   _xdisp_coupled(isCoupled("disp_x")),
   _ydisp_coupled(isCoupled("disp_y")),
   _zdisp_coupled(isCoupled("disp_z")),
   _xdisp_var(_xdisp_coupled ? coupled("disp_x") : 0),
   _ydisp_var(_ydisp_coupled ? coupled("disp_y") : 0),
   _zdisp_var(_zdisp_coupled ? coupled("disp_z") : 0),
   _penetration_locator(getQuadraturePenetrationLocator(parameters.get<BoundaryName>("paired_boundary"), getParam<std::vector<BoundaryName> >("boundary")[0], Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))),
   _serialized_solution(_sys.currentSolution()),
   _dof_map(_sys.dofMap()),
   _warnings(getParam<bool>("warnings"))
{
}


Real
QuadratureGapHeatTransfer::computeQpResidual()
{
  computeGapTempAndDistance();

  if(_has_info)
  {
    Real grad_t = (_u[_qp] - _gap_temp) * _gap_conductance[_qp];

    return _test[_i][_qp]*grad_t;
  }
  else
    return 0;
}

Real
QuadratureGapHeatTransfer::computeSlaveFluxContribution( Real grad_t )
{
  return _coord[_qp] * _JxW[_qp] * _test[_i][_qp] * grad_t;
}

Real
QuadratureGapHeatTransfer::computeQpJacobian()
{
  computeGapTempAndDistance();
  
  return _test[_i][_qp] * ((_u[_qp] - _gap_temp) * _gap_conductance_dT[_qp] + _gap_conductance[_qp]) * _phi[_j][_qp];
}

Real
QuadratureGapHeatTransfer::computeQpOffDiagJacobian( unsigned jvar )
{
  computeGapTempAndDistance();
  
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
    dRdx = -(_u[_qp]-_gap_temp)*_gap_conductance[_qp]/gapL * dgap;
  }
  return _test[_i][_qp] * dRdx * _phi[_j][_qp];
}


Real
QuadratureGapHeatTransfer::gapLength() const
{
//  return GapConductance::gapLength( -(_gap_distance), _min_gap, _max_gap );
  if(!_has_info)
    return 1.0;
  
  Real gap_L = -_gap_distance;

  if(gap_L > _max_gap)
  {
    gap_L = _max_gap;
  }

  gap_L = std::max(_min_gap, gap_L);

  return gap_L;

}

Real
QuadratureGapHeatTransfer::dgapLength( Real normalComponent ) const
{
  const Real gap_L = gapLength();

  Real dgap(0);

  if ( _min_gap <= gap_L && gap_L <= _max_gap)
  {
    dgap = normalComponent;
  }

  return dgap;
}

void
QuadratureGapHeatTransfer::computeGapTempAndDistance()
{
  Node * qnode = _mesh.getQuadratureNode(_current_elem, _current_side, _qp);
  
  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[qnode->id()];

  _gap_temp = 0.0;
  _gap_distance = 88888;
  _has_info = false;

  if (pinfo)
  {
    _gap_distance = pinfo->_distance;
    _has_info = true;
    
    Elem * slave_side = pinfo->_side;
    std::vector<std::vector<Real> > & slave_side_phi = pinfo->_side_phi;
    std::vector<unsigned int> slave_side_dof_indices;

    _dof_map.dof_indices(slave_side, slave_side_dof_indices, _variable->number());

    for(unsigned int i=0; i<slave_side_dof_indices.size(); ++i)
    {
      //The zero index is because we only have one point that the phis are evaluated at
      _gap_temp += slave_side_phi[i][0] * (*_serialized_solution)(slave_side_dof_indices[i]);
    }
  }
  else
  {
    if (_warnings)
    {
      std::stringstream msg;
      msg << "No gap value information found for node ";
      msg << qnode->id();
      msg << " on processor ";
      msg << libMesh::processor_id();
      mooseWarning( msg.str() );
    }
  }
}

