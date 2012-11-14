#include "GapConductance.h"

// Moose Includes
#include "PenetrationLocator.h"

// libMesh Includes
#include "string_to_enum.h"

template<>
InputParameters validParams<GapConductance>()
{
  MooseEnum orders("FIRST, SECOND, THIRD, FOURTH", "FIRST");

  InputParameters params = validParams<Material>();

  // Node based
  params.addCoupledVar("gap_distance", "Distance across the gap");
  params.addCoupledVar("gap_temp", "Temperature on the other side of the gap");
  params.addParam<Real>("gap_conductivity", 1.0, "The thermal conductivity of the gap material");

  // Quadrature based
  params.addCoupledVar("temp", "The temperature variable");
  params.addParam<bool>("quadrature", false, "Whether or not to do Quadrature point based gap heat transfer.  If this is true then gap_distance and gap_temp shoul NOT be provided (and will be ignored) however paired_boundary IS then required and so is 'temp'.");
  params.addParam<BoundaryName>("paired_boundary", "The boundary to be penetrated");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>("warnings", false, "Whether to output warning messages concerning nodes not being found");

  // Common
  params.addParam<Real>("min_gap", 1.0e-6, "A minimum gap size");
  params.addParam<Real>("max_gap", 1.0e6, "A maximum gap size");
  return params;
}

GapConductance::GapConductance(const std::string & name, InputParameters parameters)
  :Material(name, parameters),
   _quadrature(getParam<bool>("quadrature")),
   _gap_temp(0),
   _gap_distance(88888),
   _has_info(false),
   _gap_distance_value(_quadrature ? _zero : coupledValue("gap_distance")),
   _gap_temp_value(_quadrature ? _zero : coupledValue("gap_temp")),
   _gap_conductance(declareProperty<Real>("gap_conductance")),
   _gap_conductance_dT(declareProperty<Real>("gap_conductance_dT")),
   _gap_conductivity(getParam<Real>("gap_conductivity")),
   _min_gap(getParam<Real>("min_gap")),
   _max_gap(getParam<Real>("max_gap")),
   _temp_var(_quadrature ? getVar("temp",0) : NULL),
   _penetration_locator(NULL),
   _current_mesh(NULL),
   _serialized_solution(_quadrature ? &_temp_var->sys().currentSolution() : NULL),
   _dof_map(_quadrature ? &_temp_var->sys().dofMap() : NULL),
   _warnings(getParam<bool>("warnings"))
{
  if(_quadrature)
  {
    if(!parameters.isParamValid("paired_boundary"))
      mooseError(std::string("No 'paired_boundary' provided for ") + _name);

    if(!isCoupled("temp"))
      mooseError(std::string("No 'temp' provided for ") + _name);
  }
  else
  {
    if(!isCoupled("gap_distance"))
      mooseError(std::string("No 'gap_distance' provided for ") + _name);

    if(!isCoupled("gap_temp"))
      mooseError(std::string("No 'gap_temp' provided for ") + _name);
  }


  if(_quadrature)
  {
    if(_displaced_subproblem)
    {
      _penetration_locator = &_displaced_subproblem->geomSearchData().getQuadraturePenetrationLocator(parameters.get<BoundaryName>("paired_boundary"),
                                                                                                     getParam<std::vector<BoundaryName> >("boundary")[0],
                                                                                                     Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")));
      _current_mesh = &_displaced_subproblem->mesh();
    }
    else
    {
      _penetration_locator = &_subproblem.geomSearchData().getQuadraturePenetrationLocator(parameters.get<BoundaryName>("paired_boundary"),
                                                                                           getParam<std::vector<BoundaryName> >("boundary")[0],
                                                                                           Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")));
      _current_mesh = &_subproblem.mesh();
    }
  }
}


void
GapConductance::computeQpProperties()
{
  computeGapTempAndDistance();
  computeQpConductance();
}

void
GapConductance::computeQpConductance()
{
  _gap_conductance[_qp] = h_conduction();
  _gap_conductance_dT[_qp] = dh_conduction();
}

Real
GapConductance::h_conduction()
{
  return gapK()/gapLength(-(_gap_distance), _min_gap, _max_gap);
}


Real
GapConductance::dh_conduction()
{
  return 0;
}

Real
GapConductance::gapLength(Real distance, Real min_gap, Real max_gap)
{
  Real gap_L = distance;

  if(gap_L > max_gap)
  {
    gap_L = max_gap;
  }

  gap_L = std::max(min_gap, gap_L);

  return gap_L;
}


Real
GapConductance::gapK()
{
  return _gap_conductivity;
}

void
GapConductance::computeGapTempAndDistance()
{
  if(!_quadrature)
  {
    _has_info = true;
    _gap_temp = _gap_temp_value[_qp];
    _gap_distance = _gap_distance_value[_qp];
    return;
  }

  Node * qnode = _current_mesh->getQuadratureNode(_current_elem, _current_side, _qp);

  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator->_penetration_info[qnode->id()];

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

    _dof_map->dof_indices(slave_side, slave_side_dof_indices, _temp_var->number());

    for(unsigned int i=0; i<slave_side_dof_indices.size(); ++i)
    {
      //The zero index is because we only have one point that the phis are evaluated at
      _gap_temp += slave_side_phi[i][0] * (*(*_serialized_solution))(slave_side_dof_indices[i]);
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
