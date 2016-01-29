/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LinearIsotropicMaterialPD.h"
#include "Material.h"
#include "NonlinearSystem.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<LinearIsotropicMaterialPD>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<NonlinearVariableName>("disp_x","Variable containing the x displacement");
  params.addParam<NonlinearVariableName>("disp_y","Variable containing the y displacement");
  params.addParam<NonlinearVariableName>("disp_z","Variable containing the z displacement");

  params.addRequiredParam<int>("pddim","Peridynamic dimension is required in Materials Block");
  params.addParam<Real>("youngs_modulus", 1.0,"Young's Modulus");
  params.addParam<Real>("poissons_ratio", 0.25,"Poisson's Ratio");
  params.addParam<Real>("mesh_spacing", 1.0,"mesh_spacing");
  params.addParam<Real>("domain_thickness", 1.0, "domain_thickness");

  params.addParam<Real>("critical_stretch", 1.0, "critical_stretch");
  params.addParam<Real>("standard_deviation", 0.0, "standard_deviation");

  params.addParam<NonlinearVariableName>("temp", "Variable containing the temperature for coupled problem");
  params.addParam<Real>("reference_temp", 0.0, "The reference temperature at which this material has zero strain");
  params.addParam<Real>("thermal_expansion", 0.0, "The thermal expansion coefficient");

  return params;
}

LinearIsotropicMaterialPD::LinearIsotropicMaterialPD(const InputParameters & parameters)
  :Material(parameters),
  _bond_force(declareProperty<Real>("bond_force")),
  _bond_force_dif(declareProperty<Real>("bond_force_dif")),

  _bond_status(declareProperty<Real>("bond_status")),
  _bond_status_old(declarePropertyOld<Real>("bond_status")),
  _critical_stretch(declareProperty<Real>("critical_stretch")),

  _pddim(isParamValid("pddim") ? getParam<int>("pddim") : 3),
  _youngs_modulus(isParamValid("youngs_modulus") ? getParam<Real>("youngs_modulus") : 0),
  _poissons_ratio(isParamValid("poissons_ratio") ? getParam<Real>("poissons_ratio") : 0),
  _mesh_spacing(isParamValid("mesh_spacing") ? getParam<Real>("mesh_spacing") : 1.0),
  _domain_thickness(isParamValid("domain_thickness") ? getParam<Real>("domain_thickness") : 1.0),
  _my_critical_stretch(isParamValid("critical_stretch") ? getParam<Real>("critical_stretch") : 0),
  _standard_deviation(isParamValid("standard_deviation") ? getParam<Real>("standard_deviation") : 0),

  _temp_ref(isParamValid("reference_temp") ? getParam<Real>("reference_temp") : 0),
  _thermal_expansion(isParamValid("thermal_expansion") ? getParam<Real>("thermal_expansion") : 0)
{

// obtain the _disp_?_var
  NonlinearVariableName disp_x = parameters.get<NonlinearVariableName>("disp_x");
  _disp_x_var = &_fe_problem.getVariable(_tid,disp_x);
  _disp_y_var = NULL;
  _disp_z_var = NULL;
  _temp_var = NULL;

  if (parameters.isParamValid("disp_y"))
  {
    NonlinearVariableName disp_y = parameters.get<NonlinearVariableName>("disp_y");
    _disp_y_var = &_fe_problem.getVariable(_tid,disp_y);

    if (parameters.isParamValid("disp_z"))
    {
      NonlinearVariableName disp_z = parameters.get<NonlinearVariableName>("disp_z");
      _disp_z_var = &_fe_problem.getVariable(_tid,disp_z);
    }
  }

  _has_temp = parameters.isParamValid("temp");
//if has temp, obtain the _temp_var
  if (_has_temp)
  {
    NonlinearVariableName temp = parameters.get<NonlinearVariableName>("temp");
    _temp_var = &_fe_problem.getVariable(_tid,temp);
  }
//random generator
  setRandomResetFrequency(EXEC_INITIAL);
}

LinearIsotropicMaterialPD::~LinearIsotropicMaterialPD()
{
}

void
LinearIsotropicMaterialPD::initQpStatefulProperties()
{
//initiate the stateful bond status
  _bond_status[_qp] = 1.0;
// Generate randomized critical stretch by Box-Muller method
  _critical_stretch[_qp] = std::sqrt(- 2.0 * std::log(getRandomReal())) * std::cos(2.0 * 3.14159265358 * getRandomReal());
  _critical_stretch[_qp] /= 1.0/_standard_deviation;
  _critical_stretch[_qp] += _my_critical_stretch;
}
void
LinearIsotropicMaterialPD::computeProperties()
{

//calculate the original bond length
  const Node* const node0 = _current_elem->get_node(0);
  const Node* const node1 = _current_elem->get_node(1);
  Real dx = (*node1)(0) - (*node0)(0);
  Real dy = 0;
  Real dz = 0;
  if (_pddim > 1)
  {
    dy = (*node1)(1) - (*node0)(1);
    if (_pddim > 2)
    {
      dz = (*node1)(2) - (*node0)(2);
    }
  }
  Real origin_length = std::sqrt( dx*dx + dy*dy + dz*dz );

//obtain the displacement solutions
  NonlinearSystem & nonlinear_sys = _fe_problem.getNonlinearSystem();
  const NumericVector<Number>& ghosted_solution = *nonlinear_sys.currentSolution();
  VectorValue<unsigned int> disp_dofs0(node0->dof_number(nonlinear_sys.number(), _disp_x_var->number(), 0),
                           (_disp_y_var ? node0->dof_number(nonlinear_sys.number(), _disp_y_var->number(), 0) : 0),
                           (_disp_z_var ? node0->dof_number(nonlinear_sys.number(), _disp_z_var->number(), 0) : 0));
  VectorValue<unsigned int> disp_dofs1(node1->dof_number(nonlinear_sys.number(), _disp_x_var->number(), 0),
                           (_disp_y_var ? node1->dof_number(nonlinear_sys.number(), _disp_y_var->number(), 0) : 0),
                           (_disp_z_var ? node1->dof_number(nonlinear_sys.number(), _disp_z_var->number(), 0) : 0));

  RealVectorValue disp_node0, disp_node1;
  for (int i = 0; i < _pddim; ++i)
  {
    disp_node0(i) = ghosted_solution(disp_dofs0(i));
    disp_node1(i) = ghosted_solution(disp_dofs1(i));
  }

//calculate the new bond length
  Real ddx = dx + disp_node1(0) - disp_node0(0);
  Real ddy = 0;
  Real ddz = 0;
  if (_pddim > 1)
  {
    ddy = dy + disp_node1(1) - disp_node0(1);
    if (_pddim > 2)
    {
      ddz = dz + disp_node1(2) - disp_node0(2);
    }
  }
  Real new_length = std::sqrt(ddx*ddx + ddy*ddy + ddz*ddz);

  Real strain = new_length/origin_length - 1.0;
  Real mechanics_strain = 0.0;

  for (unsigned int _qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
//if has temp, calculate the thermal strain 
    if (_has_temp)
    {
      unsigned int temp_dof0(node0->dof_number(nonlinear_sys.number(), _temp_var->number(), 0));
      unsigned int temp_dof1(node1->dof_number(nonlinear_sys.number(), _temp_var->number(), 0));
      Real temp_node0 = ghosted_solution(temp_dof0);
      Real temp_node1 = ghosted_solution(temp_dof1);
      Real temp_avg = (temp_node0 + temp_node1) / 2.0;
      mechanics_strain = strain - _thermal_expansion * (temp_avg - _temp_ref);
    }
    else
    {
      mechanics_strain = strain;
    }

// determine the bond status, this will have some issue when crack closes
    if(std::abs(_bond_status_old[_qp] - 1.0) < 0.0001)
    {
      if (std::abs(mechanics_strain) > _critical_stretch[_qp])
      {
        _bond_status[_qp] = 0.0;
      }
      else
      {
        _bond_status[_qp] = 1.0;
      }
    }
    else
    {
      _bond_status[_qp] = 0.0;
    }

//calculate the bond_response and stiff_elem
    if (_pddim == 2)
    {
      Real node_volume = std::pow(_mesh_spacing, 2) * _domain_thickness;
      Real bulk_modulus = _youngs_modulus / 2.0 / (1.0 - _poissons_ratio);
      _bond_force[_qp] = 12.0 * bulk_modulus / (3.14159265358 * _domain_thickness * std::pow(3.0 * _mesh_spacing, 3)) * mechanics_strain * node_volume * _bond_status[_qp];
      _bond_force_dif[_qp] = 12.0 * bulk_modulus / (3.14159265358 * _domain_thickness * std::pow(3.0 * _mesh_spacing, 3)) / origin_length * node_volume * _bond_status[_qp];
    }
    else if (_pddim == 3)
    {
      Real node_volume = std::pow(_mesh_spacing, 3);
      Real bulk_modulus = _youngs_modulus / 3.0 / (1.0 - 2.0 * _poissons_ratio);
      _bond_force[_qp] = 18.0 * bulk_modulus / (3.14159265358 * std::pow(3.0 * _mesh_spacing, 4)) * mechanics_strain * node_volume * _bond_status[_qp];
      _bond_force_dif[_qp] = 18.0 * bulk_modulus / (3.14159265358 * std::pow(3.0 * _mesh_spacing, 4)) / origin_length * node_volume * _bond_status[_qp];
    }
  }
}
