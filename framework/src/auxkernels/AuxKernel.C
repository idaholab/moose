#include "AuxKernel.h"

//local includes
#include "Moose.h"

//libmesh includes
#include "numeric_vector.h"
#include "dof_map.h"

AuxKernel::AuxKernel(std::string name,
                     Parameters parameters,
                     std::string var_name,
                     std::vector<std::string> coupled_to,
                     std::vector<std::string> coupled_as)
  :Kernel(name, parameters, var_name, false, coupled_to, coupled_as),
   _nodal(_fe_type.family == LAGRANGE),
   _u_aux(_nodal ? _aux_var_vals_nodal[_tid][_var_num] : _aux_var_vals_element[_tid][_var_num]),
   _u_old_aux(_nodal ? _aux_var_vals_old_nodal[_tid][_var_num] : _aux_var_vals_old_element[_tid][_var_num]),
   _u_older_aux(_nodal ? _aux_var_vals_older_nodal[_tid][_var_num] : _aux_var_vals_older_element[_tid][_var_num]),
   _current_node(_static_current_node[_tid])
  {
    if(_nodal)
      _nodal_var_nums.push_back(_var_num);
    else
      _element_var_nums.push_back(_var_num);
  }

void
AuxKernel::sizeEverything()
{
  int n_threads = libMesh::n_threads();

  _static_current_node.resize(n_threads);
  
  _var_vals_nodal.resize(n_threads);
  _var_vals_old_nodal.resize(n_threads);
  _var_vals_older_nodal.resize(n_threads);

  _aux_var_dofs.resize(n_threads);
  _aux_var_vals_nodal.resize(n_threads);
  _aux_var_vals_old_nodal.resize(n_threads);
  _aux_var_vals_older_nodal.resize(n_threads);

  _var_vals_element.resize(n_threads);
  _var_vals_old_element.resize(n_threads);
  _var_vals_older_element.resize(n_threads);
  _var_grads_element.resize(n_threads);
  _var_grads_old_element.resize(n_threads);
  _var_grads_older_element.resize(n_threads);
  _aux_var_vals_element.resize(n_threads);
  _aux_var_vals_old_element.resize(n_threads);
  _aux_var_vals_older_element.resize(n_threads);
  _aux_var_grads_element.resize(n_threads);
  _aux_var_grads_old_element.resize(n_threads);
  _aux_var_grads_older_element.resize(n_threads);
}

void
AuxKernel::init()
{
  _nonlinear_old_soln = _system->old_local_solution.get();
  _nonlinear_older_soln = _system->older_local_solution.get();
  
  _aux_soln = _aux_system->solution.get();
  _aux_old_soln = _aux_system->old_local_solution.get();
  _aux_older_soln = _aux_system->older_local_solution.get();
}

void
AuxKernel::reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node)
{
  Moose::perf_log.push("reinit(node)","AuxKernel");

  _static_current_node[tid] = &node;

  unsigned int nonlinear_system_number = _system->number();
  unsigned int aux_system_number = _aux_system->number();

  //Non Aux vars first
  for(unsigned int i=0; i<_var_nums.size(); i++)
  {
    unsigned int var_num = _var_nums[i];
    
    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(nonlinear_system_number, var_num, 0);

    _var_vals_nodal[tid][var_num] = soln(dof_number);

    if(_is_transient)
    {
      _var_vals_old_nodal[tid][var_num] = (*_nonlinear_old_soln)(dof_number);
      _var_vals_older_nodal[tid][var_num] = (*_nonlinear_older_soln)(dof_number);
    }
  }

  const NumericVector<Number>& aux_soln = *_aux_system->solution;
  const NumericVector<Number>& aux_old_soln = *_aux_system->old_local_solution;
  const NumericVector<Number>& aux_older_soln = *_aux_system->older_local_solution;

  //Now Nodal Aux vars
  for(unsigned int i=0; i<_nodal_var_nums.size(); i++)
  {
    unsigned int var_num = _nodal_var_nums[i];

    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(aux_system_number, var_num, 0);

    _aux_var_dofs[tid][var_num] = dof_number;
    _aux_var_vals_nodal[tid][var_num] = (*_aux_soln)(dof_number);

    if(_is_transient)
    {
      _aux_var_vals_old_nodal[tid][var_num] = (*_aux_old_soln)(dof_number);
      _aux_var_vals_older_nodal[tid][var_num] = (*_aux_older_soln)(dof_number);
    }
  }

  Moose::perf_log.pop("reinit(node)","AuxKernel");
}

void
AuxKernel::reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Elem & elem)
{
  Moose::perf_log.push("reinit(elem)","AuxKernel");

  unsigned int nonlinear_system_number = _system->number();
  unsigned int aux_system_number = _aux_system->number();

  //Compute the area of the element
  Real area = 0;
  //Just use any old JxW... they are all actually the same
  const std::vector<Real> & jxw = *(_static_JxW[tid].begin()->second);

  if( Moose::geom_type == Moose::XYZ)
  {
    for (unsigned int qp=0; qp<_static_qrule[tid]->n_points(); qp++)
      area += jxw[qp];
  }
  else if (Moose::geom_type == Moose::CYLINDRICAL)
  {  
    const std::vector<Point> & q_point = *(_static_q_point[tid].begin()->second);
    for (unsigned int qp=0; qp<_static_qrule[tid]->n_points(); qp++)
      area += q_point[qp](0)*jxw[qp];
  }
  else
  {
    std::cerr << "geom_type must either XYZ or CYLINDRICAL" << std::endl;
    mooseError("");
  }
  
  //Compute the average value of each variable on the element
  
  //Non Aux vars first
  for(unsigned int i=0; i<_var_nums.size(); i++)
  {
    unsigned int var_num = _var_nums[i];

    FEType fe_type = _dof_map->variable_type(var_num);

    const std::vector<Real> & JxW = *_static_JxW[tid][fe_type];
    const std::vector<Point> & q_point = *_static_q_point[tid][fe_type];
    
    _var_vals_element[tid][var_num] = integrateValue(_var_vals[tid][var_num], JxW, q_point) / area;

    if(_is_transient)
    {
      _var_vals_old_element[tid][var_num] = integrateValue(_var_vals_old[tid][var_num], JxW, q_point) / area;
      _var_vals_older_element[tid][var_num] = integrateValue(_var_vals_older[tid][var_num], JxW, q_point) / area;
    }

    _var_grads_element[tid][var_num] = integrateGradient(_var_grads[tid][var_num], JxW, q_point) / area;

    if(_is_transient)
    {
      _var_grads_old_element[tid][var_num] = integrateGradient(_var_grads_old[tid][var_num], JxW, q_point) / area;
      _var_grads_older_element[tid][var_num] = integrateGradient(_var_grads_older[tid][var_num], JxW, q_point) / area;
    }
  }

  //Now Aux vars
  for(unsigned int i=0; i<_aux_var_nums.size(); i++)
  {
    unsigned int var_num = _aux_var_nums[i];
    
    FEType fe_type = _aux_dof_map->variable_type(var_num);

    const std::vector<Real> & JxW = *_static_JxW[tid][fe_type];
    const std::vector<Point> & q_point = *_static_q_point[tid][fe_type];
    
    _aux_var_vals_element[tid][var_num] = integrateValue(_aux_var_vals[tid][var_num], JxW, q_point) / area;

    if(_is_transient)
    {
      _aux_var_vals_old_element[tid][var_num] = integrateValue(_aux_var_vals_old[tid][var_num], JxW, q_point) / area;
      _aux_var_vals_older_element[tid][var_num] = integrateValue(_aux_var_vals_older[tid][var_num], JxW, q_point) / area;
    }
    
    _aux_var_grads_element[tid][var_num] = integrateGradient(_aux_var_grads[tid][var_num], JxW, q_point) / area;

    if(_is_transient)
    {
      _aux_var_grads_old_element[tid][var_num] = integrateGradient(_aux_var_grads_old[tid][var_num], JxW, q_point) / area;
      _aux_var_grads_older_element[tid][var_num] = integrateGradient(_aux_var_grads_older[tid][var_num], JxW, q_point) / area;
    }
  }

  //Grab the dof numbers for the element variables
  for(unsigned int i=0; i<_element_var_nums.size(); i++)
  {
    unsigned int var_num = _element_var_nums[i];

    //The zero is the component... that works fine for FIRST order monomials
    unsigned int dof_number = elem.dof_number(aux_system_number, var_num, 0);

    _aux_var_dofs[tid][var_num] = dof_number;
  }

  Moose::perf_log.pop("reinit(elem)","AuxKernel");
}

void
AuxKernel::computeAndStore(THREAD_ID tid)
{
  _aux_soln->set(_aux_var_dofs[tid][_var_num], computeValue());
}

bool
AuxKernel::isNodal()
  {
    return _nodal;
  }

Real
AuxKernel::computeQpResidual()
  {
    return 0;
  }

Real &
AuxKernel::coupledValAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_nodal)
  {
    if(!isAux(name))
      return _var_vals_nodal[_tid][_coupled_as_to_var_num[name]];
    else
      return _aux_var_vals_nodal[_tid][_aux_coupled_as_to_var_num[name]];
  }
  else
  {
    if(!isAux(name))
      return _var_vals_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _aux_var_vals_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}


Real &
AuxKernel::coupledValOldAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_nodal)
  {
    if(!isAux(name))
      return _var_vals_old_nodal[_tid][_coupled_as_to_var_num[name]];
    else
      return _aux_var_vals_old_nodal[_tid][_aux_coupled_as_to_var_num[name]];
  }
  else
  {
    if(!isAux(name))
      return _var_vals_old_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _aux_var_vals_old_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}


Real &
AuxKernel::coupledValOlderAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_nodal)
  {
    if(!isAux(name))
      return _var_vals_older_nodal[_tid][_coupled_as_to_var_num[name]];
    else
      return _aux_var_vals_older_nodal[_tid][_aux_coupled_as_to_var_num[name]];
  }
  else
  {
    if(!isAux(name))
      return _var_vals_older_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _aux_var_vals_older_element[_tid][_aux_coupled_as_to_var_num[name]];
  }  
}


RealGradient &
AuxKernel::coupledGradAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_nodal)
  {
    std::cerr<<std::endl<<"Gradient can not be recovered with nodal AuxKernel "<<_name<<" with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }
  else
  {
    if(!isAux(name))
      return _var_grads_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _aux_var_grads_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}

RealGradient &
AuxKernel::coupledGradOldAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_nodal)
  {
    std::cerr<<std::endl<<"Old Gradient can not be recovered with nodal AuxKernel "<<_name<<" with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }
  else
  {
    if(!isAux(name))
      return _var_grads_old_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _aux_var_grads_old_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}

RealGradient &
AuxKernel::coupledGradOlderAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_nodal)
  {
    std::cerr<<std::endl<<"Older Gradient can not be recovered with nodal AuxKernel "<<_name<<" with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }
  else
  {
    if(!isAux(name))
      return _var_grads_older_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _aux_var_grads_older_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}


Real
AuxKernel::integrateValue(const std::vector<Real> & vals, const std::vector<Real> & JxW, const std::vector<Point> & q_point)
{
  Real value = 0;

  if( Moose::geom_type == Moose::XYZ)
  {    
    for (unsigned int qp=0; qp<_static_qrule[0]->n_points(); qp++)
      value += vals[qp]*JxW[qp];
  }
  else if( Moose::geom_type == Moose::CYLINDRICAL )
  {        
    for (unsigned int qp=0; qp<_static_qrule[0]->n_points(); qp++)
      value += q_point[qp](0)*vals[qp]*JxW[qp];
  }
  else
  {
    std::cerr << "geom_type must either XYZ or CYLINDRICAL" << std::endl;
    mooseError("");
  }
  
  return value;
}

RealGradient
AuxKernel::integrateGradient(const std::vector<RealGradient> & grads, const std::vector<Real> & JxW, const std::vector<Point> & q_point)
{
  RealGradient value = 0;

  if( Moose::geom_type == Moose::XYZ )
  {  
    for (unsigned int qp=0; qp<_static_qrule[0]->n_points(); qp++)
      value += grads[qp]*JxW[qp];
  }
  else if( Moose::geom_type == Moose::CYLINDRICAL )
  {
    for (unsigned int qp=0; qp<_static_qrule[0]->n_points(); qp++)
      value += q_point[qp](0)*grads[qp]*JxW[qp];
  }
  else
  {
    std::cerr << "geom_type must either XYZ or CYLINDRICAL" << std::endl;
    mooseError("");
  }
  
  
  return value;
}



const NumericVector<Number> * AuxKernel::_nonlinear_old_soln;
const NumericVector<Number> * AuxKernel::_nonlinear_older_soln;
NumericVector<Number> * AuxKernel::_aux_soln;
const NumericVector<Number> * AuxKernel::_aux_old_soln;
const NumericVector<Number> * AuxKernel::_aux_older_soln;

std::vector<const Node *> AuxKernel::_static_current_node;

std::vector<unsigned int> AuxKernel::_nodal_var_nums;

std::vector<std::map<unsigned int, Real > > AuxKernel::_var_vals_nodal;
std::vector<std::map<unsigned int, Real > > AuxKernel::_var_vals_old_nodal;
std::vector<std::map<unsigned int, Real > > AuxKernel::_var_vals_older_nodal;

std::vector<std::map<unsigned int, unsigned int> > AuxKernel::_aux_var_dofs;
std::vector<std::map<unsigned int, Real > > AuxKernel::_aux_var_vals_nodal;
std::vector<std::map<unsigned int, Real > > AuxKernel::_aux_var_vals_old_nodal;
std::vector<std::map<unsigned int, Real > > AuxKernel::_aux_var_vals_older_nodal;

std::vector<unsigned int> AuxKernel::_element_var_nums;
std::vector<std::map<unsigned int, Real > > AuxKernel::_var_vals_element;
std::vector<std::map<unsigned int, Real > > AuxKernel::_var_vals_old_element;
std::vector<std::map<unsigned int, Real > > AuxKernel::_var_vals_older_element;
std::vector<std::map<unsigned int, RealGradient > > AuxKernel::_var_grads_element;
std::vector<std::map<unsigned int, RealGradient > > AuxKernel::_var_grads_old_element;
std::vector<std::map<unsigned int, RealGradient > > AuxKernel::_var_grads_older_element;
std::vector<std::map<unsigned int, Real > > AuxKernel::_aux_var_vals_element;
std::vector<std::map<unsigned int, Real > > AuxKernel::_aux_var_vals_old_element;
std::vector<std::map<unsigned int, Real > > AuxKernel::_aux_var_vals_older_element;
std::vector<std::map<unsigned int, RealGradient > > AuxKernel::_aux_var_grads_element;
std::vector<std::map<unsigned int, RealGradient > > AuxKernel::_aux_var_grads_old_element;
std::vector<std::map<unsigned int, RealGradient > > AuxKernel::_aux_var_grads_older_element;
