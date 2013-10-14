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

#include "AuxKernel.h"

//local includes
#include "SubProblem.h"
#include "AuxiliarySystem.h"
#include "MooseTypes.h"

//libmesh includes
#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"

template<>
InputParameters validParams<AuxKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();

  // Add the SetupInterface parameter, 'execute_on', the default is 'residual'
  params += validParams<SetupInterface>();

  params.addRequiredParam<AuxVariableName>("variable", "The name of the variable that this object applies to");

  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.addPrivateParam<std::string>("built_by_action", "add_aux_kernel");
  return params;
}

AuxKernel::AuxKernel(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    BlockRestrictable(name, parameters),
    BoundaryRestrictable(name, parameters),
    SetupInterface(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(parameters, parameters.get<AuxiliarySystem *>("_aux_sys")->getVariable(parameters.get<THREAD_ID>("_tid"), parameters.get<AuxVariableName>("variable")).isNodal()),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    TransientInterface(parameters, name, "aux_kernels"),
    MaterialPropertyInterface(parameters),
    PostprocessorInterface(parameters),
    DependencyResolverInterface(),
    GeometricSearchInterface(parameters),
    Restartable(name, parameters, "AuxKernels"),
    Reportable(name, parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _nl_sys(*parameters.get<SystemBase *>("_nl_sys")),
    _aux_sys(*parameters.get<AuxiliarySystem *>("_aux_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),

    _var(_aux_sys.getVariable(_tid, parameters.get<AuxVariableName>("variable"))),
    _nodal(_var.isNodal()),

    _bnd(parameters.have_parameter<BoundaryID>("_boundary_id")),

    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),

    _q_point(_bnd ? _assembly.qPointsFace() : _assembly.qPoints()),
    _qrule(_bnd ? _assembly.qRuleFace() : _assembly.qRule()),
    _JxW(_bnd ? _assembly.JxWFace() : _assembly.JxW()),
    _coord(_assembly.coordTransformation()),

    _u(_nodal ? _var.nodalSln() : _var.sln()),
    _u_old(_nodal ? _var.nodalSlnOld() : _var.slnOld()),
    _u_older(_nodal ? _var.nodalSlnOlder() : _var.slnOlder()),

    _current_elem(_var.currentElem()),
    _current_side(_var.currentSide()),
    _current_elem_volume(_assembly.elemVolume()),
    _current_side_volume(_assembly.sideElemVolume()),

    _current_node(_var.node()),

    _solution(_aux_sys.solution()),

    _real_zero(_subproblem._real_zero[_tid]),
    _zero(_subproblem._zero[_tid]),
    _grad_zero(_subproblem._grad_zero[_tid]),
    _second_zero(_subproblem._second_zero[_tid])
{
  _supplied_vars.insert(parameters.get<AuxVariableName>("variable"));

  std::map<std::string, std::vector<MooseVariable *> > coupled_vars = getCoupledVars();
  for (std::map<std::string, std::vector<MooseVariable *> >::iterator it = coupled_vars.begin(); it != coupled_vars.end(); ++it)
    for (std::vector<MooseVariable *>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
      _depend_vars.insert((*it2)->name());
}

AuxKernel::~AuxKernel()
{
}

const std::set<std::string> &
AuxKernel::getRequestedItems()
{
  return _depend_vars;
}

const std::set<std::string> &
AuxKernel::getSuppliedItems()
{
  return _supplied_vars;
}

const UserObject &
AuxKernel::getUserObjectBase(const std::string & name)
{
  _depend_uo.insert(_pars.get<UserObjectName>(name));
  return UserObjectInterface::getUserObjectBase(name);
}


void
AuxKernel::coupledCallback(const std::string & var_name, bool is_old)
{
  if (is_old)
  {
    std::vector<VariableName> var_names = getParam<std::vector<VariableName> >(var_name);
    for (std::vector<VariableName>::const_iterator it = var_names.begin(); it != var_names.end(); ++it)
      _depend_vars.erase(*it);
    //std::cout << "Removing Depend: " << name() << ": " << getParam<std::vector<AuxVariableName> >(var_name) << "\n";
  }
}

void
AuxKernel::compute()
{
  Real value = 0;
  if (isNodal())
  {
    if (_var.isNodalDefined())
    {
      _qp = 0;
      value = computeValue();
      _var.setNodalValue(value);                  // update variable data, which is referenced by other kernels, so the value is up-to-date
    }
  }
  else
  {
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      value += _JxW[_qp]*_coord[_qp]*computeValue();
    value /= (_bnd ? _current_side_volume : _current_elem_volume);
    _var.setNodalValue(value); // update the variable data refernced by other kernels.  Note that this will update the values at the quadrature points too (because this is an Elemental variable)
  }
}

bool
AuxKernel::isNodal()
{
  return _nodal;
}
