#include "Kernel.h"
#include "Moose.h"
#include "MaterialFactory.h"

// libMesh includes
#include "dof_map.h"
#include "dense_vector.h"
#include "numeric_vector.h"
#include "dense_subvector.h"

Kernel::Kernel(std::string name,
               Parameters parameters,
               std::string var_name,
               bool integrated,
               std::vector<std::string> coupled_to,
               std::vector<std::string> coupled_as)
  :_name(name),
   _parameters(parameters),
   _var_name(var_name),
   _var_num(_system->variable_number(_var_name)),
   _integrated(integrated),
   _u(_var_vals[_var_num]),
   _grad_u(_var_grads[_var_num]),
   _u_old(_var_vals_old[_var_num]),
   _u_older(_var_vals_older[_var_num]),
   _grad_u_old(_var_grads_old[_var_num]),
   _grad_u_older(_var_grads_older[_var_num]),
   _JxW(*_static_JxW),
   _phi(*_static_phi),
   _dphi(*_static_dphi),
   _q_point(*_static_q_point),
   _coupled_to(coupled_to),
   _coupled_as(coupled_as)
{
  // If this variable isn't known yet... make it so
  if(std::find(_var_nums.begin(),_var_nums.end(),_var_num) == _var_nums.end())
    _var_nums.push_back(_var_num);

  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    unsigned int coupled_var_num = _system->variable_number(coupled_var_name);

    _coupled_as_to_var_num[coupled_as[i]] = coupled_var_num;

    if(std::find(_var_nums.begin(),_var_nums.end(),coupled_var_num) == _var_nums.end())
      _var_nums.push_back(coupled_var_num);

    if(std::find(_coupled_var_nums.begin(),_coupled_var_nums.end(),coupled_var_num) == _coupled_var_nums.end())
      _coupled_var_nums.push_back(coupled_var_num);
  }
}

void
Kernel::init(EquationSystems * es)
{
  _es = es;
  _mesh = &_es->get_mesh();
  _dim = _mesh->mesh_dimension();
  _system = &_es->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
  _dof_map = &_system->get_dof_map();
  _fe_type = _dof_map->variable_type(0);

  _fe = FEBase::build(_dim, _fe_type);
  _qrule = new QGauss(_dim,_fe_type.default_quadrature_order());
  _fe->attach_quadrature_rule(_qrule);

  _static_JxW = &_fe->get_JxW();
  _static_phi = &_fe->get_phi();
  _static_dphi = &_fe->get_dphi();
  _static_q_point = &_fe->get_xyz();
             
  _t = 0;
  _dt = 0;
  _is_transient = false;
  _n_of_rk_stages = 1;
  _t_scheme = 0;  

  if(_es->parameters.have_parameter<Real>("time") && _es->parameters.have_parameter<Real>("dt"))
  {
    _is_transient = true;
    _t            = _es->parameters.get<Real>("time");
    _dt           = _es->parameters.get<Real>("dt");
    _t_step       = 0;    
    _dt_old       = _dt;
    _bdf2_wei[0]  = 1.;
    _bdf2_wei[1]  =-1.;
    _bdf2_wei[2]  = 0.;    
  }
  if(_es->parameters.have_parameter<Real>("keff"))
  {
    _is_transient = true;
  }
  
}

void
Kernel::reinitDT()
{
  if(_is_transient)
  {
    _t = _es->parameters.get<Real>("time");
    _t_step = _es->parameters.get<int>("t_step");
    _dt_old = _dt;
    _dt = _es->parameters.get<Real>("dt");
    Real sum = _dt+_dt_old;
    _bdf2_wei[2] = 1.+_dt/sum;
    _bdf2_wei[1] =-sum/_dt_old;
    _bdf2_wei[0] =_dt*_dt/_dt_old/sum;
  }   
}

std::string
Kernel::name()
{
  return _name;
}

void
Kernel::reinit(const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke)
{
  Moose::perf_log.push("reinit()","Kernel");

  _current_elem = elem;

  _dof_map->dof_indices(elem, _dof_indices);

  _fe->reinit(elem);

  if(Re)
    Re->resize(_dof_indices.size());

  if(Ke)
    Ke->resize(_dof_indices.size(),_dof_indices.size());

  std::vector<unsigned int>::iterator var_num_it = _var_nums.begin();
  std::vector<unsigned int>::iterator var_num_end = _var_nums.end();

  for(;var_num_it != var_num_end; ++var_num_it)
  {
    unsigned int var_num = *var_num_it;

    _dof_map->dof_indices(elem, _var_dof_indices[var_num], var_num);

    unsigned int num_dofs = _var_dof_indices[var_num].size();

    if(Re)
    {
      if(_var_Res[var_num])
        delete _var_Res[var_num];
    
      _var_Res[var_num] = new DenseSubVector<Number>(*Re,var_num*num_dofs,num_dofs);
    }

    if(Ke)
    {
      if(_var_Kes[var_num])
        delete _var_Kes[var_num];
    
      _var_Kes[var_num] = new DenseSubMatrix<Number>(*Ke,var_num*num_dofs,var_num*num_dofs,num_dofs,num_dofs);
    }
    
    _var_vals[var_num].resize(_qrule->n_points());
    _var_grads[var_num].resize(_qrule->n_points());

    if(_is_transient)
    {
      _var_vals_old[var_num].resize(_qrule->n_points());
      _var_grads_old[var_num].resize(_qrule->n_points());

      _var_vals_older[var_num].resize(_qrule->n_points());
      _var_grads_older[var_num].resize(_qrule->n_points());
    }

    for (unsigned int qp=0; qp<_qrule->n_points(); qp++)
    {
      computeQpSolution(_var_vals[var_num][qp], soln, _var_dof_indices[var_num], qp, *_static_phi);
      computeQpGradSolution(_var_grads[var_num][qp], soln, _var_dof_indices[var_num], qp, *_static_dphi);

      if(_is_transient)
      {
        computeQpSolution(_var_vals_old[var_num][qp], *_system->old_local_solution, _var_dof_indices[var_num], qp, *_static_phi);
        computeQpGradSolution(_var_grads_old[var_num][qp], *_system->old_local_solution, _var_dof_indices[var_num], qp, *_static_dphi);
        
        computeQpSolution(_var_vals_older[var_num][qp], *_system->older_local_solution, _var_dof_indices[var_num], qp, *_static_phi);
        computeQpGradSolution(_var_grads_older[var_num][qp], *_system->older_local_solution, _var_dof_indices[var_num], qp, *_static_dphi);
      }
    }
  }

  _material = MaterialFactory::instance()->getMaterial(elem->subdomain_id());
  _material->materialReinit();
  
  Moose::perf_log.pop("reinit()","Kernel");
}

void
Kernel::computeResidual()
{
  Moose::perf_log.push("computeResidual()","Kernel");

  DenseSubVector<Number> & var_Re = *_var_Res[_var_num];

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    for (_i=0; _i<_phi.size(); _i++)
      var_Re(_i) += _JxW[_qp]*computeQpResidual();
  
  Moose::perf_log.pop("computeResidual()","Kernel");
}

void
Kernel::computeJacobian()
{
  Moose::perf_log.push("computeJacobian()","Kernel");

  DenseSubMatrix<Number> & var_Ke = *_var_Kes[_var_num];

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    for (_i=0; _i<_phi.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
        var_Ke(_i,_j) += _JxW[_qp]*computeQpJacobian();
  
  Moose::perf_log.pop("computeJacobian()","Kernel");
}


Real
Kernel::computeIntegral()
{
  Moose::perf_log.push("computeIntegral()","Kernel");

  Real sum = 0;
  
  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    for (_i=0; _i<_phi.size(); _i++)
      sum += _JxW[_qp]*computeQpIntegral();
  
  Moose::perf_log.pop("computeIntegral()","Kernel");
  return sum;
}

void
Kernel::computeQpSolution(Real & u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<Real> > & phi)
{
  u=0;

  for (unsigned int i=0; i<phi.size(); i++) 
  {
    u +=  phi[i][qp]*soln(dof_indices[i]);
  }
}

void
Kernel::computeQpGradSolution(RealGradient & grad_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealGradient> > & dphi)
{
  grad_u=0;

  for (unsigned int i=0; i<dphi.size(); i++) 
  {
    grad_u += dphi[i][qp]*soln(dof_indices[i]);
  }
}

bool
Kernel::isCoupled(std::string name)
{
  return std::find(_coupled_as.begin(),_coupled_as.end(),name) != _coupled_as.end();
}

std::vector<Real> &
Kernel::coupledVal(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }
  
  return _var_vals[_coupled_as_to_var_num[name]];
}

std::vector<RealGradient> &
Kernel::coupledGrad(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }
  
  return _var_grads[_coupled_as_to_var_num[name]];
}

const Elem * Kernel::_current_elem;
DofMap * Kernel::_dof_map;
std::vector<unsigned int> Kernel::_dof_indices;
EquationSystems * Kernel::_es;
TransientNonlinearImplicitSystem * Kernel::_system;
MeshBase * Kernel::_mesh;
unsigned int Kernel::_dim;
FEType Kernel::_fe_type;
AutoPtr<FEBase> Kernel::_fe;
QGauss * Kernel::_qrule;
const std::vector<Real> * Kernel::_static_JxW;
const std::vector<std::vector<Real> > * Kernel::_static_phi;
const std::vector<std::vector<RealGradient> > * Kernel::_static_dphi;
const std::vector<Point> * Kernel::_static_q_point;
std::vector<unsigned int> Kernel::_var_nums;
std::map<unsigned int, std::vector<unsigned int> > Kernel::_var_dof_indices;
std::map<unsigned int, DenseSubVector<Number> * > Kernel::_var_Res;
std::map<unsigned int, DenseSubMatrix<Number> * > Kernel::_var_Kes;
std::map<unsigned int, std::vector<Real> > Kernel::_var_vals;
std::map<unsigned int, std::vector<RealGradient> > Kernel::_var_grads;
std::map<unsigned int, std::vector<Real> > Kernel::_var_vals_old;
std::map<unsigned int, std::vector<Real> > Kernel::_var_vals_older;
std::map<unsigned int, std::vector<RealGradient> > Kernel::_var_grads_old;
std::map<unsigned int, std::vector<RealGradient> > Kernel::_var_grads_older;
Real Kernel::_t;
Real Kernel::_dt;
Real Kernel::_dt_old;
int Kernel::_t_step;
short Kernel::_t_scheme;
short Kernel::_n_of_rk_stages;
Real Kernel::_bdf2_wei[3];
bool Kernel::_is_transient;
Material * Kernel::_material;
