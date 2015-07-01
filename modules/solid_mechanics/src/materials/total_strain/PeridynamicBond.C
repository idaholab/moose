/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PeridynamicBond.h"

#include "Material.h"
#include "ColumnMajorMatrix.h"
#include "SymmIsotropicElasticityTensor.h"
#include "VolumetricModel.h"

//using namespace std;

double lamda2D(double poissons_ratio)
{
	int m,n;
	double e1 = 0.0001,e2,s;
	double ftemp1,cn,dx,dy;
	double tol = 0.0001;
	double lamda = 0.0;
	e2 = -poissons_ratio*e1;
	for(m = -3;m<4;m++)
	{	
		for(n = 1;n<4;n++)
		{
			dx = m;
			dy = n;
			ftemp1 = sqrt(pow(dx,2) + pow(dy,2));
			if(ftemp1 <= 3.0 + tol)
			{
				cn = dy / ftemp1;
				s = (sqrt(pow(dy*(1.0+e1),2.0)+pow(dx*(1.0+e2),2.0))-sqrt(dx*dx+dy*dy))/sqrt(dx*dx+dy*dy);
				lamda += n*(exp(-ftemp1/3.0)*cn*s);
			}
		}
	}
	lamda /= e1;
	return lamda;
}

template<>
InputParameters validParams<PeridynamicBond>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<NonlinearVariableName>("disp_x","Variable containing the x displacement");
  params.addParam<NonlinearVariableName>        ("disp_y","Variable containing the y displacement");
  params.addParam<NonlinearVariableName>        ("disp_z","Variable containing the z displacement");
  params.addParam<Real>("youngs_modulus", "Young's Modulus");
  params.addParam<Real>("poissons_ratio", "Poisson's Ratio");
  params.addParam<Real>("MeshSpacing", "MeshSpacing");
  params.addParam<Real>("ThicknessPerLayer", 1.0, "ThicknessPerLayer");
  params.addCoupledVar("youngs_modulus_var","Variable containing Young's modulus");
  params.addParam<Real>("t_ref", 0.0, "The reference temperature at which this material has zero strain.");
  params.addParam<Real>("thermal_expansion", 0.0, "The thermal expansion coefficient.");
  params.addCoupledVar("temp", "The temperature if you want thermal expansion.");
  return params;
}

PeridynamicBond::PeridynamicBond(const std::string  & name,
                             InputParameters parameters)
  :Material(name, parameters),
   _axial_force(declareProperty<Real>("axial_force")),
   _stiff_elem(declareProperty<Real>("stiff_elem")),
   _youngs_modulus(isParamValid("youngs_modulus") ? getParam<Real>("youngs_modulus") : 0),
   _poissons_ratio(isParamValid("poissons_ratio") ? getParam<Real>("poissons_ratio") : 0),
   _MeshSpacing(isParamValid("MeshSpacing") ? getParam<Real>("MeshSpacing") : 0),
   _ThicknessPerLayer(isParamValid("ThicknessPerLayer") ? getParam<Real>("ThicknessPerLayer") : 0),
   _youngs_modulus_coupled(isCoupled("youngs_modulus_var")),
   _youngs_modulus_var(_youngs_modulus_coupled ? coupledValue("youngs_modulus_var"): _zero),
   _has_temp(isCoupled("temp")),
   _temp(_has_temp ? coupledValue("temp") : _zero),
   _t_ref(getParam<Real>("t_ref")),
   _alpha(getParam<Real>("thermal_expansion")),
   _dim(1)
{
// This doesn't work: if there are just line elements, this
// returns 1 even if the mesh has higher dimensionality
//  unsigned int dim = _subproblem.mesh().dimension();


// This won't work for multiple threads because computeProperties will get called for
// all of the threads.
  NonlinearVariableName disp_x = parameters.get<NonlinearVariableName>("disp_x");
  _disp_x_var = &_fe_problem.getVariable(_tid,disp_x);
  _disp_y_var = NULL;
  _disp_z_var = NULL;

  if (parameters.isParamValid("disp_y"))
  {
    NonlinearVariableName disp_y = parameters.get<NonlinearVariableName>("disp_y");
    _disp_y_var = &_fe_problem.getVariable(_tid,disp_y);
    _dim = 2;

    if (parameters.isParamValid("disp_z"))
    {
      NonlinearVariableName disp_z = parameters.get<NonlinearVariableName>("disp_z");
      _disp_z_var = &_fe_problem.getVariable(_tid,disp_z);
      _dim = 3;
    }
  }

  if (parameters.isParamValid("youngs_modulus"))
  {
    if (_youngs_modulus_coupled)
    {
      mooseError("Cannot specify both youngs_modulus and youngs_modulus_var");
    }
  }
  else
  {
    if (!_youngs_modulus_coupled)
    {
      mooseError("Must specify either youngs_modulus or youngs_modulus_var");
    }
  }
  /***********************************************************************************************/
  /* Peridynamic Code */                                                                           
  /***********************************************************************************************/
  _MaterialRegion = 3.0 * _MeshSpacing;
  _VolumePerNode = _MeshSpacing * _MeshSpacing * _ThicknessPerLayer;
  _lamda = lamda2D(_poissons_ratio);
  /***********************************************************************************************/
}

PeridynamicBond::~PeridynamicBond()
{
}

void
PeridynamicBond::computeProperties()
{
  
  const Node* const node0=_current_elem->get_node(0);
  const Node* const node1=_current_elem->get_node(1);

  Real dx=(*node1)(0)-(*node0)(0);
  Real dy=0;
  Real dz=0;
  if (_dim > 1)
  {
    dy=(*node1)(1)-(*node0)(1);
    if (_dim > 2)
    {
      dz=(*node1)(2)-(*node0)(2);
    }
  }
  Real orig_length=std::sqrt( dx*dx + dy*dy + dz*dz );

  NonlinearSystem & nonlinear_sys = _fe_problem.getNonlinearSystem();
  const NumericVector<Number>& ghosted_solution = *nonlinear_sys.currentSolution();

  VectorValue<unsigned int> disp_dofs0(node0->dof_number(nonlinear_sys.number(), _disp_x_var->number(), 0),
                                       (_disp_y_var ? node0->dof_number(nonlinear_sys.number(), _disp_y_var->number(), 0) : 0),
                                       (_disp_z_var ? node0->dof_number(nonlinear_sys.number(), _disp_z_var->number(), 0) : 0));
  VectorValue<unsigned int> disp_dofs1(node1->dof_number(nonlinear_sys.number(), _disp_x_var->number(), 0),
                                       (_disp_y_var ? node1->dof_number(nonlinear_sys.number(), _disp_y_var->number(), 0) : 0),
                                       (_disp_z_var ? node1->dof_number(nonlinear_sys.number(), _disp_z_var->number(), 0) : 0));

  RealVectorValue disp_vec0;
  RealVectorValue disp_vec1;

  for (unsigned int i=0; i<_dim; ++i)
  {
    disp_vec0(i) = ghosted_solution(disp_dofs0(i));
    disp_vec1(i) = ghosted_solution(disp_dofs1(i));
  }

  Real ddx=dx+disp_vec1(0)-disp_vec0(0);
  Real ddy=0;
  Real ddz=0;
  if (_dim > 1)
  {
    ddy=dy+disp_vec1(1)-disp_vec0(1);
    if (_dim > 2)
    {
      ddz=dz+disp_vec1(2)-disp_vec0(2);
    }
  }
  Real new_length = std::sqrt( ddx*ddx + ddy*ddy + ddz*ddz );
  Real strain = (new_length-orig_length)/orig_length;

  Real thermal_strain = 0.0;

  for (_qp=0; _qp < _qrule->n_points(); ++_qp)
  {
    Real youngs_modulus(_youngs_modulus_coupled ? _youngs_modulus_var[_qp] : _youngs_modulus);
    if (_has_temp)
    {
      thermal_strain = _alpha * (_t_ref - _temp[_qp]);
    }
    _axial_force[_qp] = youngs_modulus * (strain+thermal_strain) * exp(-orig_length / _MaterialRegion) * _VolumePerNode / _MeshSpacing / _lamda;
    //_e_over_l[_qp] = youngs_modulus/orig_length;
    _stiff_elem[_qp] = (youngs_modulus * exp(-orig_length / _MaterialRegion) / _MeshSpacing / _VolumePerNode / _lamda) * _VolumePerNode * _VolumePerNode / new_length;
  }
}
