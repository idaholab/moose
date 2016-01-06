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

using namespace std;

double lamda2D(double poissons_ratio)
{
  int m, n;
  double e1 = 0.0001, e2, s;
  double ftemp1, cn, dx, dy;
  double tol = 0.0001;
  double lamda = 0.0;
  e2 = - poissons_ratio * e1;
  for(m = -3; m < 4; m++)
  {
    for(n = 1; n < 4; n++)
    {
      dx = m;
      dy = n;
      ftemp1 = sqrt(pow(dx,2) + pow(dy,2));
      if(ftemp1 <= 3.0 + tol)
      {
        cn = dy / ftemp1;
        s = (sqrt(pow(dy * (1.0 + e1),2.0) + pow(dx * (1.0 + e2),2.0)) - sqrt(dx * dx + dy * dy))/sqrt(dx * dx + dy * dy);
        lamda += n * (exp(-ftemp1/3.0) * cn * s);
      }
    }
  }
  lamda /= e1;
  return lamda;
}

double lamda3D(double poissons_ratio)
{
  int m, n, p;
  double e1 = 0.0001, e2, e3, s;
  double ftemp1, cn, dx, dy, dz;
  double tol = 0.0001;
  double lamda = 0.0;
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
        ftemp1 = sqrt(pow(dx,2) + pow(dy,2) + pow(dz,2));
        if(ftemp1 <= 3.0 + tol)
        {
          cn = dy / ftemp1;
          s = (sqrt(pow(dy * (1.0 + e1),2.0) + pow(dx * (1.0 + e2),2.0) + pow(dz * (1.0 + e3),2.0)) - sqrt(dx * dx + dy * dy + dz * dz))/sqrt(dx * dx + dy * dy + dz * dz);
          lamda += n * (exp(-ftemp1/3.0) * cn * s);
        }
      }
    }
  }
  lamda /= e1;
  return lamda;
}

double AvgArea2D(double MeshSpacing, double ThicknessPerLayer)
{
  double denominator = 0.0;
  denominator += exp(-3.0/3.0) * 3.0 * 4.0;
  denominator += exp(-2.0/3.0) * 2.0 * 4.0;
  denominator += exp(-1.0/3.0) * 1.0 * 4.0;
  denominator += exp(-sqrt(5.0)/3.0) * sqrt(5.0) * 8.0;
  denominator += exp(-sqrt(8.0)/3.0) * sqrt(8.0) * 4.0;
  denominator += exp(-sqrt(2.0)/3.0) * sqrt(2.0) * 4.0;
  return 2.0 * MeshSpacing * ThicknessPerLayer / denominator;
}

double AvgArea3D(double MeshSpacing)
{
  double denominator = 0.0;
  denominator += exp(-3.0/3.0) * 3.0 * 6.0;
  denominator += exp(-2.0/3.0) * 2.0 * 6.0;
  denominator += exp(-1.0/3.0) * 1.0 * 6.0;
  denominator += exp(-sqrt(5.0)/3.0) * sqrt(5.0) * 24.0;
  denominator += exp(-sqrt(8.0)/3.0) * sqrt(8.0) * 12.0;
  denominator += exp(-sqrt(2.0)/3.0) * sqrt(2.0) * 12.0;
  denominator += exp(-sqrt(3.0)/3.0) * sqrt(3.0) * 8.0;
  denominator += exp(-sqrt(6.0)/3.0) * sqrt(6.0) * 24.0;
  denominator += exp(-sqrt(9.0)/3.0) * sqrt(9.0) * 24.0;
  return 2.0 * MeshSpacing * MeshSpacing / denominator;
}

template<>
InputParameters validParams<PeridynamicBond>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<NonlinearVariableName>("disp_x","Variable containing the x displacement");
  params.addParam<NonlinearVariableName>        ("disp_y","Variable containing the y displacement");
  params.addParam<NonlinearVariableName>        ("disp_z","Variable containing the z displacement");
  params.addRequiredParam<int>("PDdim","Peridynamic dimension is required in Materials Block");
  params.addParam<Real>("youngs_modulus", 1,"Young's Modulus");
  params.addParam<Real>("poissons_ratio", 0.25,"Poisson's Ratio");
  params.addParam<Real>("MeshSpacing", 1.0,"MeshSpacing");
  params.addParam<Real>("ThicknessPerLayer", 1.0, "ThicknessPerLayer");
  params.addParam<Real>("CriticalStretch", 1.0, "CriticalStretch");
  params.addParam<Real>("StandardDeviation", 1.0, "StandardDeviation");
  params.addParam<Real>("reference_temp", 0.0, "The reference temperature at which this material has zero strain.");
  params.addParam<Real>("thermal_expansion", 0.0, "The thermal expansion coefficient.");
  params.addParam<Real>("thermal_conductivity", 0.0, "The thermal conductivity");
  params.addCoupledVar("temp", "The temperature if you want thermal expansion.");
  return params;
}

PeridynamicBond::PeridynamicBond(const InputParameters & parameters)
  :Material(parameters),
  _axial_force(declareProperty<Real>("axial_force")),
  _stiff_elem(declareProperty<Real>("stiff_elem")),
  _bond_status(declareProperty<Real>("bond_status")),
  _bond_status_old(declarePropertyOld<Real>("bond_status")),
  _bond_stretch(declareProperty<Real>("bond_stretch")),
  _critical_stretch(declareProperty<Real>("critical_stretch")),
  _critical_stretch_old(declarePropertyOld<Real>("critical_stretch")),
  _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
  _bond_volume(declareProperty<Real>("bond_volume")),
  _PDdim(isParamValid("PDdim") ? getParam<int>("PDdim") : 0),
  _youngs_modulus(isParamValid("youngs_modulus") ? getParam<Real>("youngs_modulus") : 0),
  _poissons_ratio(isParamValid("poissons_ratio") ? getParam<Real>("poissons_ratio") : 0),
  _MeshSpacing(isParamValid("MeshSpacing") ? getParam<Real>("MeshSpacing") : 0),
  _ThicknessPerLayer(isParamValid("ThicknessPerLayer") ? getParam<Real>("ThicknessPerLayer") : 0),
  _CriticalStretch(isParamValid("CriticalStretch") ? getParam<Real>("CriticalStretch") : 0),
  _StandardDeviation(isParamValid("StandardDeviation") ? getParam<Real>("StandardDeviation") : 0),
  _has_temp(isCoupled("temp")),
  _temp(_has_temp ? coupledValue("temp") : _zero),
  _t_ref(isParamValid("reference_temp") ? getParam<Real>("reference_temp") : 0),
  _alpha(isParamValid("thermal_expansion") ? getParam<Real>("thermal_expansion") : 0),
  _my_thermal_conductivity(isParamValid("thermal_conductivity") ? getParam<Real>("thermal_conductivity") : 0),
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

  /***********************************************************************************************/
  /* Peridynamic Code */
  /***********************************************************************************************/
  _MaterialRegion = 3.0 * _MeshSpacing;
  if (_PDdim == 2)
  {
    _VolumePerNode = _MeshSpacing * _MeshSpacing * _ThicknessPerLayer;
    _lamda = lamda2D(_poissons_ratio);
    _AvgArea = AvgArea2D(_MeshSpacing,_ThicknessPerLayer);
  }
  else if (_PDdim == 3)
  {
    _VolumePerNode = _MeshSpacing * _MeshSpacing * _MeshSpacing;
    _lamda = lamda3D(_poissons_ratio);
    _AvgArea = AvgArea3D(_MeshSpacing);
  }
  setRandomResetFrequency(EXEC_INITIAL);
  //cout << _PDdim << endl;
  //cout << _AvgArea << endl;
  //cout << _lamda << endl;
  /***********************************************************************************************/
}

PeridynamicBond::~PeridynamicBond()
{
}

void
PeridynamicBond::initQpStatefulProperties()
{
  _bond_status[_qp] = 1.0;
  _bond_status_old[_qp] = 1.0;
  _critical_stretch[_qp] = 10.0;
  _critical_stretch_old[_qp] = 10.0;
}
void
PeridynamicBond::computeProperties()
{
  const Node* const node0 = _current_elem->get_node(0);
  const Node* const node1 = _current_elem->get_node(1);

  Real dx = (*node1)(0) - (*node0)(0);
  Real dy = 0;
  Real dz = 0;
  if (_dim > 1)
  {
    dy = (*node1)(1) - (*node0)(1);
    if (_dim > 2)
    {
      dz = (*node1)(2) - (*node0)(2);
    }
  }
  Real orig_length = std::sqrt( dx*dx + dy*dy + dz*dz );
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

  for (unsigned int i = 0; i < _dim; ++i)
  {
    disp_vec0(i) = ghosted_solution(disp_dofs0(i));
    disp_vec1(i) = ghosted_solution(disp_dofs1(i));
  }

  Real ddx = dx + disp_vec1(0) - disp_vec0(0);
  Real ddy = 0;
  Real ddz = 0;
  if (_dim > 1)
  {
    ddy = dy + disp_vec1(1) - disp_vec0(1);
    if (_dim > 2)
    {
      ddz = dz + disp_vec1(2) - disp_vec0(2);
    }
  }
  Real new_length = std::sqrt( ddx*ddx + ddy*ddy + ddz*ddz );
  Real strain = new_length/orig_length - 1.0;
  Real mechanics_strain = 0.0;
  Real thermal_strain = 0.0;
  double Pi = 3.14159265358;
  double Varying_Thermal_Conductivity;
  double t;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    /* Generate randomized critical stretch by Box-Muller method */
    if(_critical_stretch_old[_qp] > 1.0)
    {
      _critical_stretch_old[_qp] = sqrt(-2.0*log(getRandomReal()))*cos(2.0*Pi*getRandomReal());
      _critical_stretch_old[_qp] /= 1.0/_StandardDeviation;
      _critical_stretch_old[_qp] += _CriticalStretch;
    }
      _critical_stretch[_qp] = _critical_stretch_old[_qp];
    if (_has_temp)
    {
      thermal_strain = _alpha * (_temp[_qp] - _t_ref);
      mechanics_strain = strain - thermal_strain;
      //_thermal_conductivity[_qp] = _bond_status_old[_qp] * _my_thermal_conductivity * exp(-orig_length / _MaterialRegion) * _AvgArea;
      t = _temp[_qp] / 1000.0;
      Varying_Thermal_Conductivity = 100.0 / (6.548 + 23.533*t) + 6400.0 * exp(-16.35/t) / pow(t,2.5);
      _thermal_conductivity[_qp] = _bond_status_old[_qp] * Varying_Thermal_Conductivity * exp(-orig_length / _MaterialRegion) * _AvgArea;
      _thermal_conductivity[_qp] *= (double) _PDdim;
    }
    else
    {
      mechanics_strain = strain;
    }
    _axial_force[_qp] = (_youngs_modulus * exp(-orig_length / _MaterialRegion) / _MeshSpacing / _lamda) * mechanics_strain;
   _stiff_elem[_qp] = (_youngs_modulus * exp(-orig_length / _MaterialRegion) / _MeshSpacing / _lamda) / orig_length;
    _bond_stretch[_qp] = mechanics_strain;
    _bond_volume[_qp] = exp(-orig_length / _MaterialRegion) * _AvgArea * orig_length;
    if (_bond_status_old[_qp] == 1.0)
    {
      if (std::abs(mechanics_strain) > _critical_stretch_old[_qp])
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
      _bond_status[_qp] = _bond_status_old[_qp];
    }
  }
}
