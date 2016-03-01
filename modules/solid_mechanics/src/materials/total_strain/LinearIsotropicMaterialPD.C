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

double lambda2D(double poissons_ratio)
{
  int m, n;
  double e1 = 0.0001, e2, s;
  double ftemp1, cn, dx, dy;
  double tol = 0.0001;
  double lambda = 0.0;
  e2 = - poissons_ratio * e1;
  for(m = -3; m < 4; m++)
  {
    for(n = 1; n < 4; n++)
    {
      dx = m;
      dy = n;
      ftemp1 = std::sqrt(dx * dx + dy * dy);
      if(ftemp1 <= 3.0 + tol)
      {
        cn = dy / ftemp1;
        s = (std::sqrt(pow(dy * (1.0 + e1),2) + pow(dx * (1.0 + e2),2)) - std::sqrt(dx * dx + dy * dy)) / std::sqrt(dx * dx + dy * dy);
        lambda += n * (std::exp(-ftemp1 / 3.0) * cn * s);
      }
    }
  }
  lambda /= e1;
  return lambda;
}

double lambda3D(double poissons_ratio)
{
  int m, n, p;
  double e1 = 0.0001, e2, e3, s;
  double ftemp1, cn, dx, dy, dz;
  double tol = 0.0001;
  double lambda = 0.0;
  e2 = - poissons_ratio * e1;
  e3 = - poissons_ratio * e1;
  for(p = -3; p < 4; p++)
  {
    for(m = -3; m < 4; m++)
    {
      for(n = 1; n < 4; n++)
      {
        dx = m;
        dy = n;
        dz = p;
        ftemp1 = std::sqrt(dx * dx + dy * dy + dz * dz);
        if(ftemp1 <= 3.0 + tol)
        {
          cn = dy / ftemp1;
          s = (std::sqrt(std::pow(dy * (1.0 + e1),2) + std::pow(dx * (1.0 + e2),2.0) + std::pow(dz * (1.0 + e3),2)) - std::sqrt(dx * dx + dy * dy + dz * dz)) / std::sqrt(dx * dx + dy * dy + dz * dz);
          lambda += n * (std::exp(-ftemp1 / 3.0) * cn * s);
        }
      }
    }
  }
  lambda /= e1;
  return lambda;
}

template<>
InputParameters validParams<LinearIsotropicMaterialPD>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("appended_property_name","","Name appended to material properties to make them unique");
  params.addRequiredParam<NonlinearVariableName>("disp_x","Variable containing the x displacement");
  params.addParam<NonlinearVariableName>("disp_y","Variable containing the y displacement");
  params.addParam<NonlinearVariableName>("disp_z","Variable containing the z displacement");

  params.addParam<std::string>("plane_strain","specify plane strain if it will be used");
  params.addParam<std::string>("vary_stiffness","specify varying bond stiffness if it will be used");
  params.addRequiredParam<int>("pddim","Peridynamic dimension is required in Materials Block");
  params.addParam<Real>("youngs_modulus",1,"Young's Modulus");
  params.addParam<Real>("poissons_ratio",0.33,"Poisson's Ratio");
  params.addParam<Real>("mesh_spacing",1,"mesh_spacing");

  params.addParam<Real>("critical_strain",1, "critical_strain");
  params.addParam<Real>("standard_deviation",0.0001, "standard_deviation");

  params.addCoupledVar("temp","Variable containing the temperature for coupled problem");
  params.addParam<Real>("reference_temp",0,"The reference temperature at which this material has zero strain");
  params.addParam<Real>("thermal_expansion",0,"The thermal expansion coefficient");

  return params;
}

LinearIsotropicMaterialPD::LinearIsotropicMaterialPD(const InputParameters & parameters)
  :Material(parameters),
  _bond_force(declareProperty<Real>("bond_force")),
  _bond_force_dif_disp(declareProperty<Real>("bond_force_dif_disp")),
  _bond_force_dif_temp(declareProperty<Real>("bond_force_dif_temp")),
  _bond_mechanic_strain(declareProperty<Real>("bond_mechanic_strain")),
  _bond_critical_strain(declareProperty<Real>("bond_critical_strain")),
  _bond_critical_strain_old(declarePropertyOld<Real>("bond_critical_strain")),

  _pddim(isParamValid("pddim") ? getParam<int>("pddim") : 3),
  _youngs_modulus(isParamValid("youngs_modulus") ? getParam<Real>("youngs_modulus") : 0),
  _poissons_ratio(isParamValid("poissons_ratio") ? getParam<Real>("poissons_ratio") : 0),
  _mesh_spacing(isParamValid("mesh_spacing") ? getParam<Real>("mesh_spacing") : 1),
  _critical_strain(isParamValid("critical_strain") ? getParam<Real>("critical_strain") : 1),
  _standard_deviation(isParamValid("standard_deviation") ? getParam<Real>("standard_deviation") : 0.0001),

  _has_temp(isCoupled("temp")),
  _temp(_has_temp ? coupledValue("temp") : _zero),
  _temp_ref(isParamValid("reference_temp") ? getParam<Real>("reference_temp") : 0),
  _thermal_expansion(isParamValid("thermal_expansion") ? getParam<Real>("thermal_expansion") : 0)
{
// plane strain case or not
  _is_plane_strain = parameters.isParamValid("plane_strain");

// vary bond stiffness or not
  _is_vary_stiffness = parameters.isParamValid("vary_stiffness");

// obtain the _disp_?_var
  NonlinearVariableName disp_x = parameters.get<NonlinearVariableName>("disp_x");
  _disp_x_var = &_fe_problem.getVariable(_tid,disp_x);
  _disp_y_var = NULL;
  _disp_z_var = NULL;

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
}

LinearIsotropicMaterialPD::~LinearIsotropicMaterialPD()
{
}

void
LinearIsotropicMaterialPD::initQpStatefulProperties()
{
// Generate randomized critical stretch by Box-Muller method
  setRandomResetFrequency(EXEC_INITIAL);
  _bond_critical_strain[_qp] = std::sqrt(- 2.0 * std::log(getRandomReal())) * std::cos(2.0 * 3.14159265358 * getRandomReal()) * _standard_deviation + _critical_strain;
}

void
LinearIsotropicMaterialPD::computeProperties()
{
//get the original bond length
  Real origin_length =  _current_elem->volume();

  const Node* const node0 = _current_elem->get_node(0);
  const Node* const node1 = _current_elem->get_node(1);

//obtain the displacements solution
  NonlinearSystem & nonlinear_sys = _fe_problem.getNonlinearSystem();
  const NumericVector<Number> & ghosted_solution = *nonlinear_sys.currentSolution();
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
  Real new_length = ((_current_elem->point(1) + disp_node1) - (_current_elem->point(0) + disp_node0)).size();

  Real strain = new_length/origin_length - 1.0;

  Real node_volume, bulk_modulus; 
  for (unsigned int _qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    if (_has_temp)
    {
//bond temperature is taken as the average of two end nodes
      _bond_mechanic_strain[_qp] = strain - _thermal_expansion * ((_temp[0] + _temp[1]) / 2.0 - _temp_ref);
    }
    else
      _bond_mechanic_strain[_qp] = strain;

//calculate the bond_force and bond_force_dif_disp, bond_force_dif_temp
    if (_pddim == 2)
    {
      node_volume = std::pow(_mesh_spacing, 2);
      if (_is_vary_stiffness)
      {
        _bond_force[_qp] = (_youngs_modulus * std::exp(-origin_length / (3.0 * _mesh_spacing)) / _mesh_spacing / lambda2D(_poissons_ratio)) * _bond_mechanic_strain[_qp] * node_volume;
        _bond_force_dif_disp[_qp] = (_youngs_modulus * std::exp(-origin_length / (3.0 * _mesh_spacing)) / _mesh_spacing / lambda2D(_poissons_ratio)) / origin_length * node_volume; 
        _bond_force_dif_temp[_qp] = - (_youngs_modulus * std::exp(-origin_length / (3.0 * _mesh_spacing)) / _mesh_spacing / lambda2D(_poissons_ratio)) * _thermal_expansion / 2.0 * node_volume;  
      }
      else
      {
        if (_is_plane_strain)
//plane strain case      
          bulk_modulus = _youngs_modulus / 2.0 / (1.0 + _poissons_ratio) / (1.0 - 2.0 * _poissons_ratio);
        else
//plane stress case      
          bulk_modulus = _youngs_modulus / 2.0 / (1.0 - _poissons_ratio);

        _bond_force[_qp] = 12.0 * bulk_modulus / (3.14159265358 * std::pow(3.0 * _mesh_spacing, 3)) * _bond_mechanic_strain[_qp] * node_volume * node_volume;
        _bond_force_dif_disp[_qp] = 12.0 * bulk_modulus / (3.14159265358 * std::pow(3.0 * _mesh_spacing, 3)) / origin_length * node_volume * node_volume;
        _bond_force_dif_temp[_qp] = - 12.0 * bulk_modulus / (3.14159265358 * std::pow(3.0 * _mesh_spacing, 3)) * _thermal_expansion / 2.0 * node_volume * node_volume;
      }
    }
    else if (_pddim == 3)
    {
      node_volume = std::pow(_mesh_spacing, 3);
      if (_is_vary_stiffness)
      {
        _bond_force[_qp] = (_youngs_modulus * std::exp(-origin_length / (3.0 * _mesh_spacing)) / _mesh_spacing / lambda3D(_poissons_ratio)) * _bond_mechanic_strain[_qp] * node_volume;
        _bond_force_dif_disp[_qp] = (_youngs_modulus * std::exp(-origin_length / (3.0 * _mesh_spacing)) / _mesh_spacing / lambda3D(_poissons_ratio)) / origin_length * node_volume; 
        _bond_force_dif_temp[_qp] = - (_youngs_modulus * std::exp(-origin_length / (3.0 * _mesh_spacing)) / _mesh_spacing / lambda3D(_poissons_ratio)) * _thermal_expansion / 2.0 * node_volume;  
      }
      else
      {
        bulk_modulus = _youngs_modulus / 3.0 / (1.0 - 2.0 * _poissons_ratio);
        _bond_force[_qp] = 18.0 * bulk_modulus / (3.14159265358 * std::pow(3.0 * _mesh_spacing, 4)) * _bond_mechanic_strain[_qp] * node_volume * node_volume;
        _bond_force_dif_disp[_qp] = 18.0 * bulk_modulus / (3.14159265358 * std::pow(3.0 * _mesh_spacing, 4)) / origin_length * node_volume * node_volume;
        _bond_force_dif_temp[_qp] = - 18.0 * bulk_modulus / (3.14159265358 * std::pow(3.0 * _mesh_spacing, 4)) * _thermal_expansion / 2.0 * node_volume * node_volume;
      }
    }
  }
}

