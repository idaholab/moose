/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "StressDivergenceTruss.h"

#include "Material.h"
#include "SymmElasticityTensor.h"

template<>
InputParameters validParams<StressDivergenceTruss>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature");
  params.addCoupledVar("area", "Cross-sectional area of truss element");
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");

  params.set<bool>("use_displaced_mesh") = true;

  return params;
}


StressDivergenceTruss::StressDivergenceTruss(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _axial_stress(getMaterialProperty<Real>("axial_stress" + getParam<std::string>("appended_property_name"))),
   _E_over_L(getMaterialProperty<Real>("e_over_l" + getParam<std::string>("appended_property_name"))),
   _component(getParam<unsigned int>("component")),
   _xdisp_coupled(isCoupled("disp_x")),
   _ydisp_coupled(isCoupled("disp_y")),
   _zdisp_coupled(isCoupled("disp_z")),
   _temp_coupled(isCoupled("temp")),
   _xdisp_var(_xdisp_coupled ? coupled("disp_x") : 0),
   _ydisp_var(_ydisp_coupled ? coupled("disp_y") : 0),
   _zdisp_var(_zdisp_coupled ? coupled("disp_z") : 0),
   _temp_var(_temp_coupled ? coupled("temp") : 0),
   _area(coupledValue("area")),
   _orientation(NULL)
{}

void
StressDivergenceTruss::initialSetup()
{
  // Assume that all trusses are one dimensional in the call below
  _orientation = &_subproblem.assembly(_tid).getFE(FEType(), 1)->get_dxyzdxi();
}


void
StressDivergenceTruss::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  mooseAssert(re.size() == 2, "Truss elements must have two nodes");
  _local_re.resize(re.size());
  _local_re.zero();

  RealGradient orientation( (*_orientation)[0] );
  orientation /= orientation.size();
  VectorValue<Real> force_local = _axial_stress[0] * _area[0] * orientation;

  int sign(-_test[0][0]/std::abs(_test[0][0]));
  _local_re(0) = sign * force_local(_component);
  _local_re(1) = -_local_re(0);

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i=0; i<_save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
StressDivergenceTruss::computeStiffness(ColumnMajorMatrix & stiff_global)
{
  RealGradient orientation( (*_orientation)[0] );
  orientation /= orientation.size();

  // Now get a rotation matrix
  // The orientation is the first row of the matrix.
  // Need two other directions.
  VectorValue<Real> & row1( orientation );
  VectorValue<Real> row3( row1 );
  unsigned zero_index(0);
  if (row3(1) != 0)
  {
    zero_index = 1;
  }
  if (row3(2) != 0)
  {
    zero_index = 2;
  }
  row3(zero_index) += 1;
  VectorValue<Real> row2 = orientation.cross( row3 );
  row3 = orientation.cross( row2 );

  Real k = _E_over_L[0] * _area[0];

  stiff_global(0,0) = row1(0)*row1(0)*k;
  stiff_global(0,1) = row1(0)*row2(0)*k;
  stiff_global(0,2) = row1(0)*row3(0)*k;
  stiff_global(1,0) = row2(0)*row1(0)*k;
  stiff_global(1,1) = row2(0)*row2(0)*k;
  stiff_global(1,2) = row2(0)*row3(0)*k;
  stiff_global(2,0) = row3(0)*row1(0)*k;
  stiff_global(2,1) = row3(0)*row2(0)*k;
  stiff_global(2,2) = row3(0)*row3(0)*k;
}

void
StressDivergenceTruss::computeJacobian()
{

  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  ColumnMajorMatrix stiff_global(3,3);
  computeStiffness( stiff_global );

  for (_i = 0; _i < _test.size(); _i++)
  {
    for (_j = 0; _j < _phi.size(); _j++)
    {
      int sign( _i == _j ? 1 : -1 );
      _local_ke(_i, _j) += sign * stiff_global(_component, _component);
    }
  }

  ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i=0; i<rows; i++)
      diag(i) = _local_ke(i,i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i=0; i<_diag_save_in.size(); i++)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

void
StressDivergenceTruss::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
  {
    computeJacobian();
  }
  else
  {
    unsigned int coupled_component = 0;

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

    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);

    if (active)
    {
      ColumnMajorMatrix stiff_global(3,3);
      computeStiffness( stiff_global );

      for (_i=0; _i<_test.size(); _i++)
      {
        for (_j=0; _j<_phi.size(); _j++)
        {
          int sign( _i == _j ? 1 : -1 );
          ke(_i,_j) += sign * stiff_global(_component, coupled_component);
        }
      }
    }
    else if ( false ) // Need some code here for coupling with temperature
    {
    }
  }
}
