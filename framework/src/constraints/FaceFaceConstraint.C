/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "FaceFaceConstraint.h"
#include "FEProblem.h"
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"

template<>
InputParameters validParams<FaceFaceConstraint>()
{
  InputParameters params = validParams<Constraint>();
  params.addRequiredParam<std::string>("interface", "The name of the interface.");
  params.addRequiredParam<VariableName>("master_variable", "Variable on master surface");
  params.addParam<VariableName>("slave_variable", "Variable on master surface");
  return params;
}

FaceFaceConstraint::FaceFaceConstraint(const std::string & name, InputParameters parameters) :
    Constraint(name, parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(parameters, true),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _dim(_mesh.dimension()),

    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation()),
    _current_elem(_assembly.elem()),

    _master_var(_subproblem.getVariable(_tid, getParam<VariableName>("master_variable"))),
    _slave_var(isParamValid("slave_variable") ? _subproblem.getVariable(_tid, getParam<VariableName>("slave_variable")) : _subproblem.getVariable(_tid, getParam<VariableName>("master_variable"))),
    _lambda(_var.sln()),

    _iface(*_mesh.getMortarInterfaceByName(getParam<std::string>("interface"))),
    _master_penetration_locator(getMortarPenetrationLocator(_iface._master, _iface._slave, Moose::Master, Order(_master_var.order()))),
    _slave_penetration_locator(getMortarPenetrationLocator(_iface._master, _iface._slave, Moose::Slave, Order(_slave_var.order()))),

    _test_master(_master_var.phi()),
    _phi_master(_master_var.phi()),

    _test_slave(_slave_var.phi()),
    _phi_slave(_slave_var.phi())
{
}

FaceFaceConstraint::~FaceFaceConstraint()
{
}

void
FaceFaceConstraint::reinit()
{
  unsigned int nqp = _qrule->n_points();

  _u_master.resize(nqp);
  _phys_points_master.resize(nqp);
  _u_slave.resize(nqp);
  _phys_points_slave.resize(nqp);
  _test = _assembly.getFE(_var.feType(), _dim-1)->get_phi();                     // yes we need to do a copy here
  _JxW_lm = _assembly.getFE(_var.feType(), _dim-1)->get_JxW();                   // another copy here to preserve the right JxW

  for (_qp = 0; _qp < nqp; _qp++)
  {
    const Node * current_node = _mesh.getQuadratureNode(_current_elem, 0, _qp);

    PenetrationInfo * master_pinfo = _master_penetration_locator._penetration_info[current_node->id()];
    PenetrationInfo * slave_pinfo = _slave_penetration_locator._penetration_info[current_node->id()];

    if (master_pinfo && slave_pinfo)
    {
      Elem * master_side = master_pinfo->_elem->build_side(master_pinfo->_side_num, true).release();
      std::vector<std::vector<Real> > & master_side_phi = master_pinfo->_side_phi;
      _u_master[_qp] = _master_var.getValue(master_side, master_side_phi);
      _phys_points_master[_qp] = master_pinfo->_closest_point;
      _elem_master = master_pinfo->_elem;
      delete master_side;

      Elem * slave_side = slave_pinfo->_elem->build_side(slave_pinfo->_side_num, true).release();
      std::vector<std::vector<Real> > & slave_side_phi = slave_pinfo->_side_phi;
      _u_slave[_qp] = _slave_var.getValue(slave_side, slave_side_phi);
      _phys_points_slave[_qp] = slave_pinfo->_closest_point;
      _elem_slave = slave_pinfo->_elem;
      delete slave_side;
    }
  }
}

void
FaceFaceConstraint::reinitSide(Moose::ConstraintType res_type)
{
  switch (res_type)
  {
  case Moose::Master:
    _assembly.reinit(_elem_master);
    _master_var.prepare();
    _assembly.prepare();
    _assembly.reinitAtPhysical(_elem_master, _phys_points_master);
    break;

  case Moose::Slave:
    _assembly.reinit(_elem_slave);
    _slave_var.prepare();
    _assembly.prepare();
    _assembly.reinitAtPhysical(_elem_slave, _phys_points_slave);
    break;
  }
}

void
FaceFaceConstraint::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      re(_i) += _JxW_lm[_qp] * _coord[_qp] * computeQpResidual();
}

void
FaceFaceConstraint::computeResidualSide(Moose::ConstraintType side)
{
  switch (side)
  {
  case Moose::Master:
    {
      DenseVector<Number> & re_master = _assembly.residualBlock(_master_var.number());
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        for (_i = 0; _i < _test_master.size(); _i++)
          re_master(_i) += _JxW_lm[_qp] * computeQpResidualSide(Moose::Master);
      }
    }
    break;

  case Moose::Slave:
    {
      DenseVector<Number> & re_slave = _assembly.residualBlock(_slave_var.number());
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        for (_i = 0; _i < _test_slave.size(); _i++)
          re_slave(_i) += _JxW_lm[_qp] * _coord[_qp] * computeQpResidualSide(Moose::Slave);
      }
    }
    break;
  }
}

void
FaceFaceConstraint::computeJacobian()
{
  _phi = _assembly.getFE(_var.feType(), _dim-1)->get_phi();                // yes we need to do a copy here
  std::vector<std::vector<Real> > phi_master;
  std::vector<std::vector<Real> > phi_slave;

  DenseMatrix<Number> & Kee = _assembly.jacobianBlock(_var.number(), _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        Kee(_i, _j) += _JxW_lm[_qp] * _coord[_qp] * computeQpJacobian();
}

void
FaceFaceConstraint::computeJacobianSide(Moose::ConstraintType side)
{
  switch (side)
  {
  case Moose::Master:
    {
      DenseMatrix<Number> & Ken_master = _assembly.jacobianBlock(_var.number(), _master_var.number());
      DenseMatrix<Number> & Kne_master = _assembly.jacobianBlock(_master_var.number(), _var.number());

      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        for (_i = 0; _i < _test_master.size(); _i++)
        {
          for (_j = 0; _j < _phi.size(); _j++)
          {
            Ken_master(_j, _i) += _JxW_lm[_qp] * _coord[_qp] * computeQpJacobianSide(Moose::MasterMaster);
            Kne_master(_i, _j) += _JxW_lm[_qp] * _coord[_qp] * computeQpJacobianSide(Moose::SlaveMaster);
          }
        }
    }
    break;

  case Moose::Slave:
    {
      DenseMatrix<Number> & Ken_slave = _assembly.jacobianBlock(_var.number(), _slave_var.number());
      DenseMatrix<Number> & Kne_slave = _assembly.jacobianBlock(_slave_var.number(), _var.number());
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        for (_i = 0; _i < _test_slave.size(); _i++)
        {
          for (_j = 0; _j < _phi.size(); _j++)
          {
            Ken_slave(_j, _i) += _JxW_lm[_qp] * _coord[_qp] * computeQpJacobianSide(Moose::MasterSlave);
            Kne_slave(_i, _j) += _JxW_lm[_qp] * _coord[_qp] * computeQpJacobianSide(Moose::SlaveSlave);
          }
        }
    }
    break;
  }
}

Real
FaceFaceConstraint::computeQpJacobian()
{
  return 0.;
}

Real
FaceFaceConstraint::computeQpJacobianSide(Moose::ConstraintJacobianType /*side_type*/)
{
  return 0.;
}
