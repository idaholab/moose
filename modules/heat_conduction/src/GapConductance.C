/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GapConductance.h"

// Moose Includes
#include "PenetrationLocator.h"

// libMesh Includes
#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<GapConductance>()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<Material>();
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");

  params.addRequiredCoupledVar("variable", "Temperature variable");

  // Node based
  params.addCoupledVar("gap_distance", "Distance across the gap");
  params.addCoupledVar("gap_temp", "Temperature on the other side of the gap");
  params.addParam<Real>("gap_conductivity", 1.0, "The thermal conductivity of the gap material");
  params.addParam<FunctionName>("gap_conductivity_function", "Thermal conductivity of the gap material as a function.  Multiplied by gap_conductivity.");
  params.addCoupledVar("gap_conductivity_function_variable", "Variable to be used in the gap_conductivity_function in place of time");


  // Quadrature based
  params.addParam<bool>("quadrature", false, "Whether or not to do quadrature point based gap heat transfer.  If this is true then gap_distance and gap_temp should NOT be provided (and will be ignored); however, paired_boundary and variable are then required.");
  params.addParam<BoundaryName>("paired_boundary", "The boundary to be penetrated");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>("warnings", false, "Whether to output warning messages concerning nodes not being found");

  // Common
  params.addRangeCheckedParam<Real>("min_gap", 1e-6, "min_gap>=0", "A minimum gap (denominator) size");
  params.addRangeCheckedParam<Real>("max_gap", 1e6, "max_gap>=0", "A maximum gap (denominator) size");

  MooseEnum coord_types("default XYZ", "default");
  params.addParam<MooseEnum>("coord_type", coord_types, "Gap calculation type (default or XYZ).");

  params.addParam<Real>("stefan_boltzmann", 5.669e-8, "The Stefan-Boltzmann constant");
  params.addRangeCheckedParam<Real>("emissivity_1", 0.0, "emissivity_1>=0 & emissivity_1<=1", "The emissivity of the fuel surface");
  params.addRangeCheckedParam<Real>("emissivity_2", 0.0, "emissivity_2>=0 & emissivity_2<=1", "The emissivity of the cladding surface");

  params.addParam<bool>("use_displaced_mesh", true, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");

  return params;
}

GapConductance::GapConductance(const std::string & name, InputParameters parameters)
  :Material(name, parameters),
   _appended_property_name( getParam<std::string>("appended_property_name") ),
   _temp(coupledValue("variable")),
   _gap_type_set(false),
   _gap_type(Moose::COORD_XYZ),
   _quadrature(getParam<bool>("quadrature")),
   _gap_temp(0),
   _gap_distance(88888),
   _radius(0),
   _r1(0),
   _r2(0),
   _has_info(false),
   _gap_distance_value(_quadrature ? _zero : coupledValue("gap_distance")),
   _gap_temp_value(_quadrature ? _zero : coupledValue("gap_temp")),
   _gap_conductance(declareProperty<Real>("gap_conductance"+_appended_property_name)),
   _gap_conductance_dT(declareProperty<Real>("gap_conductance"+_appended_property_name+"_dT")),
   _gap_conductivity(getParam<Real>("gap_conductivity")),
   _gap_conductivity_function(isParamValid("gap_conductivity_function") ? &getFunction("gap_conductivity_function") : NULL),
   _gap_conductivity_function_variable(isCoupled("gap_conductivity_function_variable") ? &coupledValue("gap_conductivity_function_variable") : NULL),
   _stefan_boltzmann(getParam<Real>("stefan_boltzmann")),
   _emissivity( getParam<Real>("emissivity_1") != 0 && getParam<Real>("emissivity_2") != 0 ?
                1/getParam<Real>("emissivity_1") + 1/getParam<Real>("emissivity_2") - 1 : 0 ),
   _min_gap(getParam<Real>("min_gap")),
   _max_gap(getParam<Real>("max_gap")),
   _temp_var(_quadrature ? getVar("variable",0) : NULL),
   _penetration_locator(NULL),
   _serialized_solution(_quadrature ? &_temp_var->sys().currentSolution() : NULL),
   _dof_map(_quadrature ? &_temp_var->sys().dofMap() : NULL),
   _warnings(getParam<bool>("warnings"))
{
  if (_quadrature)
  {
    if (!parameters.isParamValid("paired_boundary"))
      mooseError(std::string("No 'paired_boundary' provided for ") + _name);
  }
  else
  {
    if (!isCoupled("gap_distance"))
      mooseError(std::string("No 'gap_distance' provided for ") + _name);

    if (!isCoupled("gap_temp"))
      mooseError(std::string("No 'gap_temp' provided for ") + _name);
  }


  if (_quadrature)
  {
    _penetration_locator = &_subproblem.geomSearchData().getQuadraturePenetrationLocator(parameters.get<BoundaryName>("paired_boundary"),
                                                                                         getParam<std::vector<BoundaryName> >("boundary")[0],
                                                                                         Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")));
  }

}

void
GapConductance::computeProperties()
{
  if (!_gap_type_set)
  {
    _gap_type_set = true;
    if (getParam<MooseEnum>("coord_type") == "XYZ")
    {
      _gap_type = Moose::COORD_XYZ;
    }
    else
    {
      _gap_type = _coord_sys;
    }
    if (_gap_type == Moose::COORD_XYZ && _coord_sys == Moose::COORD_RSPHERICAL)
      mooseError("The 'XYZ' coord_type and a spherical coordinate system are not compatible.");
  }

  Material::computeProperties();
}

void
GapConductance::computeQpProperties()
{
  computeGapValues();
  computeQpConductance();
}

void
GapConductance::computeQpConductance()
{
  if (_has_info)
  {
    _gap_conductance[_qp] = h_conduction() + h_radiation();
    _gap_conductance_dT[_qp] = dh_conduction();
  }
  else
  {
    _gap_conductance[_qp] = 0;
    _gap_conductance_dT[_qp] = 0;
  }
}

Real
GapConductance::h_conduction()
{
  return gapK() / gapLength(_gap_type, _radius, _r1, _r2, _min_gap, _max_gap);
}


Real
GapConductance::dh_conduction()
{
  return 0;
}


Real
GapConductance::h_radiation()
{
  /*
   Gap conductance due to radiation is based on the diffusion approximation:

      qr = sigma*Fe*(Tf^4 - Tc^4) ~ hr(Tf - Tc)
         where sigma is the Stefan-Boltztmann constant, Fe is an emissivity function, Tf and Tc
         are the fuel and clad absolute temperatures, respectively, and hr is the radiant gap
         conductance. Solving for hr,

      hr = sigma*Fe*(Tf^4 - Tc^4) / (Tf - Tc)
         which can be factored to give:

      hr = sigma*Fe*(Tf^2 + Tc^2) * (Tf + Tc)

   Approximating the fuel-clad gap as infinite parallel planes, the emissivity function is given by:

      Fe = 1 / (1/ef + 1/ec - 1)
  */

  if (_emissivity == 0)
  {
    return 0.0;
  }

  const Real temp_func = (_temp[_qp]*_temp[_qp] + _gap_temp*_gap_temp) * (_temp[_qp] + _gap_temp);
  return _stefan_boltzmann*temp_func/_emissivity;
}

Real
GapConductance::dh_radiation()
{
  if (_emissivity == 0)
  {
    return 0.0;
  }
  const Real temp_func = 3*_temp[_qp]*_temp[_qp] + _gap_temp * ( 2*_temp[_qp] + _gap_temp );
  return _stefan_boltzmann*temp_func/_emissivity;
}

Real
GapConductance::gapLength(const Moose::CoordinateSystemType & system,
                          Real radius, Real r1, Real r2, Real min_gap, Real max_gap)
{
  if (system == Moose::COORD_RZ)
    return gapCyl(radius, r1, r2, min_gap, max_gap);
  else if (system == Moose::COORD_RSPHERICAL)
    return gapSphere(radius, r1, r2, min_gap, max_gap);
  else
    return gapRect(r2-r1, min_gap, max_gap);
}

Real
GapConductance::gapRect(Real distance, Real min_gap, Real max_gap)
{
  Real gap_L = distance;

  if (gap_L > max_gap)
  {
    gap_L = max_gap;
  }

  gap_L = std::max(min_gap, gap_L);

  return gap_L;
}

Real
GapConductance::gapCyl(Real radius, Real r1, Real r2, Real min_denom, Real max_denom)
{
  Real denominator = radius*std::log(r2/r1);

  if (denominator > max_denom)
  {
    denominator = max_denom;
  }
  else if (denominator < min_denom)
  {
    denominator =  min_denom;
  }

  return denominator;
}

Real
GapConductance::gapSphere(Real radius, Real r1, Real r2, Real min_denom, Real max_denom)
{
  Real denominator = std::pow(radius,2)*((1/r1)-(1/r2));

  if (denominator > max_denom)
  {
    denominator = max_denom;
  }
  else if (denominator < min_denom)
  {
    denominator =  min_denom;
  }

  return denominator;
}

Real
GapConductance::gapK()
{
  Real gap_conductivity = _gap_conductivity;
  if (_gap_conductivity_function)
  {
    if (_gap_conductivity_function_variable)
    {
      gap_conductivity *= _gap_conductivity_function->value( (*_gap_conductivity_function_variable)[_qp], _q_point[_qp] );
    }
    else
    {
      gap_conductivity *= _gap_conductivity_function->value( _t, _q_point[_qp] );
    }
  }

  return gap_conductivity;
}

void
GapConductance::computeGapValues()
{
  if (!_quadrature)
  {
    _has_info = true;
    _gap_temp = _gap_temp_value[_qp];
    _gap_distance = _gap_distance_value[_qp];
  }
  else
  {
    Node * qnode = _mesh.getQuadratureNode(_current_elem, _current_side, _qp);
    PenetrationInfo * pinfo = _penetration_locator->_penetration_info[qnode->id()];

    _gap_temp = 0.0;
    _gap_distance = 88888;
    _has_info = false;

    if (pinfo)
    {
      _gap_distance = pinfo->_distance;
      _has_info = true;

      Elem * slave_side = pinfo->_side;
      std::vector<std::vector<Real> > & slave_side_phi = pinfo->_side_phi;
      std::vector<dof_id_type> slave_side_dof_indices;

      _dof_map->dof_indices(slave_side, slave_side_dof_indices, _temp_var->number());

      for (unsigned int i=0; i<slave_side_dof_indices.size(); ++i)
      {
        //The zero index is because we only have one point that the phis are evaluated at
        _gap_temp += slave_side_phi[i][0] * (*(*_serialized_solution))(slave_side_dof_indices[i]);
      }
    }
    else
    {
      if (_warnings)
        mooseWarning("No gap value information found for node " << qnode->id() << " on processor " << processor_id() << " at coordinate " << Point(*qnode));
    }
  }

  if (_gap_type == Moose::COORD_RZ || _gap_type == Moose::COORD_RSPHERICAL)
  {
    if (_normals[_qp](0) > 0)
    {
      _r1 = _q_point[_qp](0);
      _r2 = _q_point[_qp](0) - _gap_distance; // note, _gap_distance is negative
      _radius = _r1;
    }
    else if (_normals[_qp](0) < 0)
    {
      _r1 = _q_point[_qp](0) + _gap_distance;
      _r2 = _q_point[_qp](0);
      _radius = _r2;
    }
    else
      mooseError( "Issue with cylindrical or spherical flux calc. normals. \n");
  }
  else
  {
    _r2 = -_gap_distance;
    _r1 = 0;
    _radius = 0;
  }
}
