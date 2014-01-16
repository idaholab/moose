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
#include "MooseMesh.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "FEProblem.h"
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"


template<>
InputParameters validParams<FaceFaceConstraint>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<NonlinearVariableName>("variable", "The name of the variable that this constraint is applied to.");
  params.addRequiredParam<std::string>("interface", "The name of the interface.");
  params.addRequiredParam<VariableName>("master_variable", "Variable on master surface");
  params.addParam<VariableName>("slave_variable", "Variable on master surface");

  params.addPrivateParam<std::string>("built_by_action", "add_constraint");

  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  return params;
}

FaceFaceConstraint::FaceFaceConstraint(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(parameters, true),
    FunctionInterface(parameters),
    TransientInterface(parameters, name, "face_face_constraints"),
    GeometricSearchInterface(parameters),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),

    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation()),
    _current_elem(_assembly.elem()),
    _phi(_var.phi()),

    _master_var(_subproblem.getVariable(_tid, getParam<VariableName>("master_variable"))),
    _slave_var(isParamValid("slave_variable") ? _subproblem.getVariable(_tid, getParam<VariableName>("slave_variable")) : _subproblem.getVariable(_tid, getParam<VariableName>("master_variable"))),
    _lambda(_var.sln()),

    _iface(*_mesh.getMortarInterfaceByName(getParam<std::string>("interface"))),
    _master_penetration_locator(getMortarPenetrationLocator(_iface._master, _iface._slave, Moose::SIDE_MASTER, Order(_master_var.order()))),
    _slave_penetration_locator(getMortarPenetrationLocator(_iface._master, _iface._slave, Moose::SIDE_SLAVE, Order(_slave_var.order()))),

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
  _test = _var.phi();               // yes, copy here
  _JxW_lm = _JxW;                   // another copy here to preserve the right JxW

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
FaceFaceConstraint::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.index());
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      re(_i) += _JxW_lm[_qp] * _coord[_qp] * computeQpResidual();
}

void
FaceFaceConstraint::computeResidualSide()
{
  // TODO: refactor these reinits into NonlinearSystem
  {
    _assembly.reinit(_elem_master);
    _master_var.prepare();
    _assembly.prepare();
    _assembly.reinitAtPhysical(_elem_master, _phys_points_master);

    DenseVector<Number> & re_master = _assembly.residualBlock(_master_var.index());
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      for (_i = 0; _i < _test_master.size(); _i++)
        re_master(_i) += _JxW_lm[_qp] * computeQpResidualSide(Moose::SIDE_MASTER);
    }
    _assembly.addResidual(_fe_problem.residualVector(Moose::KT_NONTIME), Moose::KT_NONTIME);
  }

  // TODO: refactor these reinits into NonlinearSystem
  {
    _assembly.reinit(_elem_slave);
    _slave_var.prepare();
    _assembly.prepare();
    _assembly.reinitAtPhysical(_elem_slave, _phys_points_slave);

    DenseVector<Number> & re_slave = _assembly.residualBlock(_slave_var.index());
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      for (_i = 0; _i < _test_slave.size(); _i++)
        re_slave(_i) += _JxW_lm[_qp] * _coord[_qp] * computeQpResidualSide(Moose::SIDE_SLAVE);
    }
    _assembly.addResidual(_fe_problem.residualVector(Moose::KT_NONTIME), Moose::KT_NONTIME);
  }
}

void
FaceFaceConstraint::computeJacobian(SparseMatrix<Number> & jacobian)
{
  std::vector<std::vector<Real> > phi_master;
  std::vector<std::vector<Real> > phi_slave;

  unsigned int nqp = _qrule->n_points();

  _phys_points_master.resize(nqp);
  _phys_points_slave.resize(nqp);
  _test = _var.phi();               // yes we need to do a copy here
  _phi = _var.phi();                // yes we need to do a copy here
  _JxW_lm = _JxW;                   // another copy here to preserve the right JxW

  for (_qp = 0; _qp < nqp; _qp++)
  {
    const Node * current_node = _mesh.getQuadratureNode(_current_elem, 0, _qp);

    PenetrationInfo * master_pinfo = _master_penetration_locator._penetration_info[current_node->id()];
    PenetrationInfo * slave_pinfo = _slave_penetration_locator._penetration_info[current_node->id()];

    if (master_pinfo && slave_pinfo)
    {
      _phys_points_master[_qp] = master_pinfo->_closest_point;
      _elem_master = master_pinfo->_elem;

      _phys_points_slave[_qp] = slave_pinfo->_closest_point;
      _elem_slave = slave_pinfo->_elem;
    }
  }

  // TODO: some constraints might need to fill in the diagonal block

  {
    _assembly.reinit(_elem_master);
    _master_var.prepare();
    _assembly.prepare();
    _assembly.reinitAtPhysical(_elem_master, _phys_points_master);

    DenseMatrix<Number> & Ken_master = _assembly.jacobianBlock(_var.index(), _master_var.index());
    DenseMatrix<Number> & Kne_master = _assembly.jacobianBlock(_master_var.index(), _var.index());

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < _test_master.size(); _i++)
      {
        for (_j = 0; _j < _phi.size(); _j++)
        {
          Real value = _JxW_lm[_qp] * _coord[_qp] * computeQpJacobianSide(Moose::SIDE_MASTER);
          Ken_master(_j, _i) += value;
          Kne_master(_i, _j) += value;
        }
      }
    _assembly.addJacobian(jacobian);
  }

  {
    _assembly.reinit(_elem_slave);
    _slave_var.prepare();
    _assembly.prepare();
    _assembly.reinitAtPhysical(_elem_slave, _phys_points_slave);

    DenseMatrix<Number> & Ken_slave = _assembly.jacobianBlock(_var.index(), _slave_var.index());
    DenseMatrix<Number> & Kne_slave = _assembly.jacobianBlock(_slave_var.index(), _var.index());
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < _test_slave.size(); _i++)
      {
        for (_j = 0; _j < _phi.size(); _j++)
        {
          Real value = _JxW_lm[_qp] * _coord[_qp] * computeQpJacobianSide(Moose::SIDE_SLAVE);
          Ken_slave(_j, _i) += value;
          Kne_slave(_i, _j) += value;
        }
      }
    _assembly.addJacobian(jacobian);
  }
}

Real
FaceFaceConstraint::computeQpJacobianSide(Moose::ConstraintSideType side_type)
{
  return 0.;
}

MooseVariable &
FaceFaceConstraint::variable()
{
  return _var;
}

SubProblem &
FaceFaceConstraint::subProblem()
{
  return _subproblem;
}
