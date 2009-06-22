#include "Kernel.h"
#include "Moose.h"
#include "MaterialFactory.h"
#include "ParallelUniqueId.h"

// libMesh includes
#include "dof_map.h"
#include "dense_vector.h"
#include "numeric_vector.h"
#include "dense_subvector.h"
#include "libmesh_common.h"

Kernel::Kernel(std::string name,
               Parameters parameters,
               std::string var_name,
               bool integrated,
               std::vector<std::string> coupled_to,
               std::vector<std::string> coupled_as)
  :_name(name),
   _tid(Moose::current_thread_id),
   _parameters(parameters),
   _var_name(var_name),
   _is_aux(_aux_system->has_variable(_var_name)),
   _var_num(_is_aux ? _aux_system->variable_number(_var_name) : _system->variable_number(_var_name)),
   _integrated(integrated),
   _u(_var_vals[_tid][_var_num]),
   _grad_u(_var_grads[_tid][_var_num]),
   _second_u(_var_seconds[_tid][_var_num]),
   _u_old(_var_vals_old[_tid][_var_num]),
   _u_older(_var_vals_older[_tid][_var_num]),
   _grad_u_old(_var_grads_old[_tid][_var_num]),
   _grad_u_older(_var_grads_older[_tid][_var_num]),
   _fe_type(_is_aux ? _aux_dof_map->variable_type(_var_num) : _dof_map->variable_type(_var_num)),
   _has_second_derivatives(_fe_type.family == CLOUGH || _fe_type.family == HERMITE),
   _current_elem(_static_current_elem[_tid]),
   _material(_static_material[_tid]),
   _JxW(*(_static_JxW[_tid])[_fe_type]),
   _phi(*(_static_phi[_tid])[_fe_type]),
   _dphi(*(_static_dphi[_tid])[_fe_type]),
   _d2phi(*(_static_d2phi[_tid])[_fe_type]),
   _qrule(_static_qrule[_tid]),
   _q_point(*(_static_q_point[_tid])[_fe_type]),
   _coupled_to(coupled_to),
   _coupled_as(coupled_as),
   _real_zero(_static_real_zero[_tid]),
   _zero(_static_zero[_tid]),
   _grad_zero(_static_grad_zero[_tid]),
   _second_zero(_static_second_zero[_tid])
{
  // If this variable isn't known yet... make it so
  if(!_is_aux)
  {
    if(std::find(_var_nums.begin(),_var_nums.end(),_var_num) == _var_nums.end())
      _var_nums.push_back(_var_num);
  }
  else
  {
    if(std::find(_aux_var_nums.begin(),_aux_var_nums.end(),_var_num) == _aux_var_nums.end())
      _aux_var_nums.push_back(_var_num);
  }

  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if(_system->has_variable(coupled_var_name))
    {
      unsigned int coupled_var_num = _system->variable_number(coupled_var_name);

      _coupled_as_to_var_num[coupled_as[i]] = coupled_var_num;

      if(std::find(_var_nums.begin(),_var_nums.end(),coupled_var_num) == _var_nums.end())
        _var_nums.push_back(coupled_var_num);

      if(std::find(_coupled_var_nums.begin(),_coupled_var_nums.end(),coupled_var_num) == _coupled_var_nums.end())
        _coupled_var_nums.push_back(coupled_var_num);
    }
    else //Look for it in the Aux system
    {
      unsigned int coupled_var_num = _aux_system->variable_number(coupled_var_name);

      _aux_coupled_as_to_var_num[coupled_as[i]] = coupled_var_num;

      if(std::find(_aux_var_nums.begin(),_aux_var_nums.end(),coupled_var_num) == _aux_var_nums.end())
        _aux_var_nums.push_back(coupled_var_num);

      if(std::find(_aux_coupled_var_nums.begin(),_aux_coupled_var_nums.end(),coupled_var_num) == _aux_coupled_var_nums.end())
        _aux_coupled_var_nums.push_back(coupled_var_num);
    }  
  }
}

void
Kernel::sizeEverything()
{
  int n_threads = libMesh::n_threads();
  
  _static_current_elem.resize(n_threads);
  _dof_indices.resize(n_threads);
  _aux_dof_indices.resize(n_threads);
  _fe.resize(n_threads);
  _static_qrule.resize(n_threads);

  _static_JxW.resize(n_threads);
  _static_phi.resize(n_threads);
  _static_dphi.resize(n_threads);
  _static_d2phi.resize(n_threads);
  _static_q_point.resize(n_threads);
  _var_dof_indices.resize(n_threads);
  _aux_var_dof_indices.resize(n_threads);
  _var_Res.resize(n_threads);
  _var_Kes.resize(n_threads);
  _var_vals.resize(n_threads);
  _var_grads.resize(n_threads);
  _var_seconds.resize(n_threads);
  _var_vals_old.resize(n_threads);
  _var_vals_older.resize(n_threads);
  _var_grads_old.resize(n_threads);
  _var_grads_older.resize(n_threads);
  _aux_var_vals.resize(n_threads);
  _aux_var_grads.resize(n_threads);
  _aux_var_vals_old.resize(n_threads);
  _aux_var_vals_older.resize(n_threads);
  _aux_var_grads_old.resize(n_threads);
  _aux_var_grads_older.resize(n_threads);

  _static_material.resize(n_threads);
  _static_real_zero.resize(n_threads);
  _static_zero.resize(n_threads);
  _static_grad_zero.resize(n_threads);
  _static_second_zero.resize(n_threads);  
}

void
Kernel::init(EquationSystems * es)
{
  _es = es;
  _mesh = &_es->get_mesh();
  _dim = _mesh->mesh_dimension();
  _system = &_es->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
  _dof_map = &_system->get_dof_map();

  _aux_system = &_es->get_system<TransientExplicitSystem>("AuxiliarySystem");
  _aux_dof_map = &_aux_system->get_dof_map();

  _max_quadrature_order = CONSTANT;
  
  //Find the largest quadrature order necessary... all variables _must_ use the same rule!
  for(unsigned int var=0; var < _system->n_vars(); var++)
  {
    FEType fe_type = _dof_map->variable_type(var);
    if(fe_type.default_quadrature_order() > _max_quadrature_order)
      _max_quadrature_order = fe_type.default_quadrature_order();
  }

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    _static_qrule[tid] = new QGauss(_dim, _max_quadrature_order);

  //This allows for different basis functions / orders for each variable
  for(unsigned int var=0; var < _system->n_vars(); var++)
  {
    FEType fe_type = _dof_map->variable_type(var);

    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    { 
      if(!_fe[tid][fe_type])
      {
        _fe[tid][fe_type] = FEBase::build(_dim, fe_type).release();
        _fe[tid][fe_type]->attach_quadrature_rule(_static_qrule[tid]);

        _static_JxW[tid][fe_type] = &_fe[tid][fe_type]->get_JxW();
        _static_phi[tid][fe_type] = &_fe[tid][fe_type]->get_phi();
        _static_dphi[tid][fe_type] = &_fe[tid][fe_type]->get_dphi();
        _static_q_point[tid][fe_type] = &_fe[tid][fe_type]->get_xyz();

        FEFamily family = fe_type.family;

        if(family == CLOUGH || family == HERMITE)
          _static_d2phi[tid][fe_type] = &_fe[tid][fe_type]->get_d2phi();
      }
    }
  }

  //This allows for different basis functions / orders for each Aux variable
  for(unsigned int var=0; var < _aux_system->n_vars(); var++)
  {
    FEType fe_type = _aux_dof_map->variable_type(var);

    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    { 
      if(!_fe[tid][fe_type])
      {
        _fe[tid][fe_type] = FEBase::build(_dim, fe_type).release();
        _fe[tid][fe_type]->attach_quadrature_rule(_static_qrule[tid]);
        
        _static_JxW[tid][fe_type] = &_fe[tid][fe_type]->get_JxW();
        _static_phi[tid][fe_type] = &_fe[tid][fe_type]->get_phi();
        _static_dphi[tid][fe_type] = &_fe[tid][fe_type]->get_dphi();
        _static_q_point[tid][fe_type] = &_fe[tid][fe_type]->get_xyz();
      }
    }
  }

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
Kernel::reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke)
{
//  Moose::perf_log.push("reinit()","Kernel");
//  Moose::perf_log.push("reinit() - dof_indices","Kernel");
  
  _static_current_elem[tid] = elem;

  _dof_map->dof_indices(elem, _dof_indices[tid]);

  std::map<FEType, FEBase*>::iterator fe_it = _fe[tid].begin();
  std::map<FEType, FEBase*>::iterator fe_end = _fe[tid].end();

//  Moose::perf_log.pop("reinit() - dof_indices","Kernel");
//  Moose::perf_log.push("reinit() - fereinit","Kernel");
  
  static bool first = true;

  if(!Moose::no_fe_reinit || first)
    for(;fe_it != fe_end; ++fe_it)
      fe_it->second->reinit(elem);

  first = false;

//  Moose::perf_log.pop("reinit() - fereinit","Kernel");

//  Moose::perf_log.push("reinit() - resizing","Kernel");
  if(Re)
    Re->resize(_dof_indices[tid].size());

  if(Ke)
    Ke->resize(_dof_indices[tid].size(),_dof_indices[tid].size());

  unsigned int position = 0;

  for(unsigned int i=0; i<_var_nums.size();i++)
  {
    _dof_map->dof_indices(elem, _var_dof_indices[tid][i], i);

    unsigned int num_dofs = _var_dof_indices[tid][i].size();
    
    if(Re)
    {
      if(_var_Res[tid][i])
        delete _var_Res[tid][i];

      _var_Res[tid][i] = new DenseSubVector<Number>(*Re,position, num_dofs);
    }

    if(Ke)
    {
      if(_var_Kes[tid][i])
        delete _var_Kes[tid][i];
    
      _var_Kes[tid][i] = new DenseSubMatrix<Number>(*Ke,position,position,num_dofs,num_dofs);
    }
    position+=num_dofs;
  }
  
  unsigned int num_q_points = _static_qrule[tid]->n_points();

  _static_real_zero[tid] = 0;
  _static_zero[tid].resize(num_q_points,0);
  _static_grad_zero[tid].resize(num_q_points,0);
  _static_second_zero[tid].resize(num_q_points,0);

  std::vector<unsigned int>::iterator var_num_it = _var_nums.begin();
  std::vector<unsigned int>::iterator var_num_end = _var_nums.end();
  
//  Moose::perf_log.pop("reinit() - resizing","Kernel");
//  Moose::perf_log.push("reinit() - compute vals","Kernel");
  
  for(;var_num_it != var_num_end; ++var_num_it)
  {
    unsigned int var_num = *var_num_it;
    
    FEType fe_type = _dof_map->variable_type(var_num);

    FEFamily family = fe_type.family;

    bool has_second_derivatives = (family == CLOUGH || family == HERMITE);

    unsigned int num_dofs = _var_dof_indices[tid][var_num].size();

    _var_vals[tid][var_num].resize(num_q_points);
    _var_grads[tid][var_num].resize(num_q_points);

    if(has_second_derivatives)
      _var_seconds[tid][var_num].resize(num_q_points);

    if(_is_transient)
    {
      _var_vals_old[tid][var_num].resize(num_q_points);
      _var_grads_old[tid][var_num].resize(num_q_points);

      _var_vals_older[tid][var_num].resize(num_q_points);
      _var_grads_older[tid][var_num].resize(num_q_points);
    }
    
    const std::vector<std::vector<Real> > & static_phi = *_static_phi[tid][fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi= *_static_dphi[tid][fe_type];
    const std::vector<std::vector<RealTensor> > & static_d2phi= *_static_d2phi[tid][fe_type];


    if (_is_transient)
    {
      if( has_second_derivatives )
        computeQpSolutionAll(_var_vals[tid][var_num], _var_vals_old[tid][var_num], _var_vals_older[tid][var_num],
                             _var_grads[tid][var_num], _var_grads_old[tid][var_num], _var_grads_older[tid][var_num],
                             _var_seconds[tid][var_num],
                             soln, *_system->old_local_solution, *_system->older_local_solution,
                             _var_dof_indices[tid][var_num], _static_qrule[tid]->n_points(),
                             static_phi, static_dphi, static_d2phi);
      else
        computeQpSolutionAll(_var_vals[tid][var_num], _var_vals_old[tid][var_num], _var_vals_older[tid][var_num],
                             _var_grads[tid][var_num], _var_grads_old[tid][var_num], _var_grads_older[tid][var_num],
                             soln, *_system->old_local_solution, *_system->older_local_solution,
                             _var_dof_indices[tid][var_num], _static_qrule[tid]->n_points(),
                             static_phi, static_dphi);
    }
    else
    {
      if( has_second_derivatives )
        computeQpSolutionAll(_var_vals[tid][var_num], _var_grads[tid][var_num],  _var_seconds[tid][var_num],
                             soln, _var_dof_indices[tid][var_num], _static_qrule[tid]->n_points(), static_phi, static_dphi, static_d2phi);
      else
        computeQpSolutionAll(_var_vals[tid][var_num], _var_grads[tid][var_num],
                             soln, _var_dof_indices[tid][var_num], _static_qrule[tid]->n_points(),static_phi, static_dphi);
    }
  }
//  Moose::perf_log.pop("reinit() - compute vals","Kernel");
  
//  Moose::perf_log.push("reinit() - compute aux vals","Kernel");
  const NumericVector<Number>& aux_soln = (*_aux_system->current_local_solution);

  std::vector<unsigned int>::iterator aux_var_num_it = _aux_var_nums.begin();
  std::vector<unsigned int>::iterator aux_var_num_end = _aux_var_nums.end();

  for(;aux_var_num_it != aux_var_num_end; ++aux_var_num_it)
  {
    unsigned int var_num = *aux_var_num_it;
    
    FEType fe_type = _aux_dof_map->variable_type(var_num);

    _aux_dof_map->dof_indices(elem, _aux_var_dof_indices[tid][var_num], var_num);

    unsigned int num_dofs = _aux_var_dof_indices[tid][var_num].size();

    _aux_var_vals[tid][var_num].resize(num_q_points);
    _aux_var_grads[tid][var_num].resize(num_q_points);

    if(_is_transient)
    {
      _aux_var_vals_old[tid][var_num].resize(num_q_points);
      _aux_var_grads_old[tid][var_num].resize(num_q_points);

      _aux_var_vals_older[tid][var_num].resize(num_q_points);
      _aux_var_grads_older[tid][var_num].resize(num_q_points);
    }
    
    const std::vector<std::vector<Real> > & static_phi = *_static_phi[tid][fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi= *_static_dphi[tid][fe_type];


    if (_is_transient)
    {
      computeQpSolutionAll(_aux_var_vals[tid][var_num], _aux_var_vals_old[tid][var_num], _aux_var_vals_older[tid][var_num],
                           _aux_var_grads[tid][var_num], _aux_var_grads_old[tid][var_num], _aux_var_grads_older[tid][var_num],
                           aux_soln, *_aux_system->old_local_solution, *_aux_system->older_local_solution,
                           _aux_var_dof_indices[tid][var_num], _static_qrule[tid]->n_points(),static_phi, static_dphi);
    }
    else
    {
      computeQpSolutionAll(_aux_var_vals[tid][var_num], _aux_var_grads[tid][var_num],
                           aux_soln, _aux_var_dof_indices[tid][var_num], _static_qrule[tid]->n_points(),static_phi, static_dphi);
    }
  }
  
//  Moose::perf_log.pop("reinit() - compute aux vals","Kernel");
//  Moose::perf_log.push("reinit() - material","Kernel");

  _static_material[tid] = MaterialFactory::instance()->getMaterial(tid,elem->subdomain_id());
  _static_material[tid]->materialReinit();

//  Moose::perf_log.pop("reinit() - material","Kernel");
//  Moose::perf_log.pop("reinit()","Kernel");
}

void
Kernel::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","Kernel");
  
  DenseSubVector<Number> & var_Re = *_var_Res[_tid][_var_num];

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    for (_i=0; _i<_phi.size(); _i++)
      var_Re(_i) += _JxW[_qp]*computeQpResidual();
  
//  Moose::perf_log.pop("computeResidual()","Kernel");
}

void
Kernel::computeJacobian()
{
//  Moose::perf_log.push("computeJacobian()",_name);

  DenseSubMatrix<Number> & var_Ke = *_var_Kes[_tid][_var_num];

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    for (_i=0; _i<_phi.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
        var_Ke(_i,_j) += _JxW[_qp]*computeQpJacobian();
  
//  Moose::perf_log.pop("computeJacobian()",_name);
}

void
Kernel::computeOffDiagJacobian(DenseMatrix<Number> & Ke, unsigned int jvar)
{
//  Moose::perf_log.push("computeOffDiagJacobian()",_name);

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    for (_i=0; _i<_phi.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
      {
        if(jvar == _var_num)
          Ke(_i,_j) += _JxW[_qp]*computeQpJacobian();
        else
          Ke(_i,_j) += _JxW[_qp]*computeQpOffDiagJacobian(jvar);
      }
  
//  Moose::perf_log.pop("computeOffDiagJacobian()",_name);
}


Real
Kernel::computeIntegral()
{
//  Moose::perf_log.push("computeIntegral()",_name);

  Real sum = 0;
  
  for (_qp=0; _qp<_qrule->n_points(); _qp++)
      sum += _JxW[_qp]*computeQpIntegral();
  
//  Moose::perf_log.pop("computeIntegral()",_name);
  return sum;
}

void
Kernel::computeQpSolution(Real & u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<Real> > & phi)
{
  u=0;

  unsigned int phi_size = phi.size();

  //All of this stuff so that the loop will vectorize
  std::vector<Real> sol_vals(phi_size);
  std::vector<Real> phi_vals(phi_size);

  for (unsigned int i=0; i<phi_size; i++) 
  {
    sol_vals[i] = soln(dof_indices[i]);
    phi_vals[i] = phi[i][qp];
  }
  
  for (unsigned int i=0; i<phi_size; i++) 
  {
    u +=  phi_vals[i]*sol_vals[i];
  }
}

void
Kernel::computeQpSolutionAll(std::vector<Real> & u, std::vector<Real> & u_old, std::vector<Real> & u_older,
                             std::vector<RealGradient> &grad_u,  std::vector<RealGradient> &grad_u_old, std::vector<RealGradient> &grad_u_older,
                             std::vector<RealTensor> &second_u,
                             const NumericVector<Number> & soln, const NumericVector<Number> & soln_old,  const NumericVector<Number> & soln_older,
                             const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                             const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi, const std::vector<std::vector<RealTensor> > & d2phi)
{
  for (unsigned int qp =0;qp<n_qp;qp++)
  {
    u[qp] = 0;
    u_old[qp] = 0;
    u_older[qp] = 0;
    
    grad_u[qp] = 0;
    grad_u_old[qp] = 0;
    grad_u_older[qp] = 0;

    second_u[qp] = 0;
  }
  
  unsigned int phi_size = phi.size();

  for (unsigned int i=0; i<phi_size; i++)
  {
    int indx = dof_indices[i];
    Real soln_local       = soln(indx);
    Real soln_old_local   = soln_old(indx);
    Real soln_older_local = soln_older(indx);
    
    for (unsigned int qp =0; qp<n_qp; qp++)
    {
      Real phi_local = phi[i][qp];
      RealGradient dphi_local = dphi[i][qp];
      
      u[qp]        += phi_local*soln_local;
      u_old[qp]    += phi_local*soln_old_local;
      u_older[qp]  += phi_local*soln_older_local;

      grad_u[qp]       += dphi_local*soln_local;
      grad_u_old[qp]   += dphi_local*soln_old_local;
      grad_u_older[qp] += dphi_local*soln_older_local;

      second_u[qp] +=d2phi[i][qp]*soln_local;
    }
    
  }
}

void
Kernel::computeQpSolutionAll(std::vector<Real> & u, std::vector<Real> & u_old, std::vector<Real> & u_older,
                             std::vector<RealGradient> &grad_u,  std::vector<RealGradient> &grad_u_old, std::vector<RealGradient> &grad_u_older,
                             const NumericVector<Number> & soln, const NumericVector<Number> & soln_old,  const NumericVector<Number> & soln_older,
                             const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                             const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi)
{

  for (unsigned int qp =0;qp<n_qp;qp++)
  {
    u[qp]=0;
    u_old[qp]=0;
    u_older[qp] = 0;
    
    grad_u[qp] = 0;
    grad_u_old[qp] = 0;
    grad_u_older[qp] = 0;
  }
  
  unsigned int phi_size = phi.size();

  for (unsigned int i=0; i<phi_size; i++)
  {
    int indx = dof_indices[i];
    Real soln_local       = soln(indx);
    Real soln_old_local   = soln_old(indx);
    Real soln_older_local = soln_older(indx);
    
    for (unsigned int qp =0; qp<n_qp; qp++)
    {
      Real phi_local = phi[i][qp];
      RealGradient dphi_local = dphi[i][qp];
      
      u[qp]        += phi_local*soln_local;
      u_old[qp]    += phi_local*soln_old_local;
      u_older[qp]  += phi_local*soln_older_local;

      grad_u[qp]       += dphi_local*soln_local;
      grad_u_old[qp]   += dphi_local*soln_old_local;
      grad_u_older[qp] += dphi_local*soln_older_local;
    }
    
  }
}

void
Kernel::computeQpSolutionAll(std::vector<Real> & u,
                             std::vector<RealGradient> &grad_u,
                             std::vector<RealTensor> &second_u,
                             const NumericVector<Number> & soln,
                             const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                             const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi, const std::vector<std::vector<RealTensor> > & d2phi)
{
  for (unsigned int qp =0;qp<n_qp;qp++)
  {
    u[qp]=0;
    grad_u[qp] = 0;
    second_u[qp] = 0;
  }
  
  unsigned int phi_size = phi.size();

  for (unsigned int i=0; i<phi_size; i++)
  {
    int indx = dof_indices[i];
    Real soln_local       = soln(indx);
    
    for (unsigned int qp =0; qp<n_qp; qp++)
    {
      Real phi_local = phi[i][qp];
      RealGradient dphi_local = dphi[i][qp];
      
      u[qp]        += phi_local*soln_local;
      grad_u[qp]   += dphi_local*soln_local;
      second_u[qp] += d2phi[i][qp]*soln_local;
    }
    
  }
}


void
Kernel::computeQpSolutionAll(std::vector<Real> & u,
                             std::vector<RealGradient> &grad_u,
                             const NumericVector<Number> & soln,
                             const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                             const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi)
{
  for (unsigned int qp =0;qp<n_qp;qp++)
  {
    u[qp]=0;
    grad_u[qp] = 0;
  }
  
  unsigned int phi_size = phi.size();

  for (unsigned int i=0; i<phi_size; i++)
  {
    int indx = dof_indices[i];
    Real soln_local = soln(indx);

    for (unsigned int qp =0; qp<n_qp; qp++)
    {
      Real phi_local = phi[i][qp];
      RealGradient dphi_local = dphi[i][qp];
      
      u[qp]        += phi_local*soln_local;
      grad_u[qp]   += dphi_local*soln_local;
    }
    
  }
}

void
Kernel::computeQpGradSolution(RealGradient & grad_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealGradient> > & dphi)
{
  grad_u=0;

  unsigned int dphi_size = dphi.size();

  for (unsigned int i=0; i<dphi_size; i++) 
  {
    grad_u += dphi[i][qp]*soln(dof_indices[i]);
  }
}

void
Kernel::computeQpSecondSolution(RealTensor & second_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealTensor> > & d2phi)
{
  second_u=0;

  unsigned int d2phi_size = d2phi.size();

  for (unsigned int i=0; i<d2phi_size; i++) 
  {
    second_u += d2phi[i][qp]*soln(dof_indices[i]);
  }
}

void
Kernel::subdomainSetup()
{
}

unsigned int
Kernel::variable()
{
  return _var_num;
}

bool
Kernel::modifiedAuxVarNum(unsigned int var_num)
{
  return MAX_VARS + var_num;
}

Real
Kernel::computeQpJacobian()
  {
    return 0;
  }

Real
Kernel::computeQpOffDiagJacobian(unsigned int jvar)
  {
    return 0;
  }

Real
Kernel::computeQpIntegral()
  {
    return 0;
  }

bool
Kernel::isAux(std::string name)
{
  return _aux_coupled_as_to_var_num.find(name) != _aux_coupled_as_to_var_num.end();
}

bool
Kernel::isCoupled(std::string name)
{
  bool found = std::find(_coupled_as.begin(),_coupled_as.end(),name) != _coupled_as.end();

  //See if it's an Aux variable
  if(!found)
    found = isAux(name);

  return found;
}

unsigned int
Kernel::coupled(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }

  if(!isAux(name))
    return _coupled_as_to_var_num[name];
  else
    return modifiedAuxVarNum(_aux_coupled_as_to_var_num[name]);
}

std::vector<Real> &
Kernel::coupledVal(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }

  if(!isAux(name))
    return _var_vals[_tid][_coupled_as_to_var_num[name]];
  else
    return _aux_var_vals[_tid][_aux_coupled_as_to_var_num[name]];
}

std::vector<RealGradient> &
Kernel::coupledGrad(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }

  if(!isAux(name))
    return _var_grads[_tid][_coupled_as_to_var_num[name]];
  else
    return _aux_var_grads[_tid][_aux_coupled_as_to_var_num[name]];
}

std::vector<RealTensor> &
Kernel::coupledSecond(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }

  //Aux vars can't have second derivatives!
  return _var_seconds[_tid][_coupled_as_to_var_num[name]];
}

std::vector<Real> &
Kernel::coupledValOld(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }

  if(!isAux(name))
    return _var_vals_old[_tid][_coupled_as_to_var_num[name]];
  else
    return _aux_var_vals_old[_tid][_aux_coupled_as_to_var_num[name]];
}
std::vector<RealGradient> &
Kernel::coupledGradValOld(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }
  
  return _var_grads_old[_tid][_coupled_as_to_var_num[name]];
}

std::vector<const Elem *> Kernel::_static_current_elem;
DofMap * Kernel::_dof_map;
DofMap * Kernel::_aux_dof_map;
std::vector<std::vector<unsigned int> > Kernel::_dof_indices;
std::vector<std::vector<unsigned int> > Kernel::_aux_dof_indices;
EquationSystems * Kernel::_es;
TransientNonlinearImplicitSystem * Kernel::_system;
TransientExplicitSystem * Kernel::_aux_system;
MeshBase * Kernel::_mesh;
unsigned int Kernel::_dim;
std::vector<std::map<FEType, FEBase *> > Kernel::_fe;
Order Kernel::_max_quadrature_order;
std::vector<QGauss *> Kernel::_static_qrule;
std::vector<std::map<FEType, const std::vector<Real> *> > Kernel::_static_JxW;
std::vector<std::map<FEType, const std::vector<std::vector<Real> > *> > Kernel::_static_phi;
std::vector<std::map<FEType, const std::vector<std::vector<RealGradient> > *> > Kernel::_static_dphi;
std::vector<std::map<FEType, const std::vector<std::vector<RealTensor> > *> > Kernel::_static_d2phi;
std::vector<std::map<FEType, const std::vector<Point> *> > Kernel::_static_q_point;
std::vector<unsigned int> Kernel::_var_nums;
std::vector<unsigned int> Kernel::_aux_var_nums;
std::vector<std::map<unsigned int, std::vector<unsigned int> > > Kernel::_var_dof_indices;
std::vector<std::map<unsigned int, std::vector<unsigned int> > > Kernel::_aux_var_dof_indices;
std::vector<std::map<unsigned int, DenseSubVector<Number> * > > Kernel::_var_Res;
std::vector<std::map<unsigned int, DenseSubMatrix<Number> * > > Kernel::_var_Kes;
std::vector<std::map<unsigned int, std::vector<Real> > > Kernel::_var_vals;
std::vector<std::map<unsigned int, std::vector<RealGradient> > > Kernel::_var_grads;
std::vector<std::map<unsigned int, std::vector<RealTensor> > > Kernel::_var_seconds;
std::vector<std::map<unsigned int, std::vector<Real> > > Kernel::_var_vals_old;
std::vector<std::map<unsigned int, std::vector<Real> > > Kernel::_var_vals_older;
std::vector<std::map<unsigned int, std::vector<RealGradient> > > Kernel::_var_grads_old;
std::vector<std::map<unsigned int, std::vector<RealGradient> > > Kernel::_var_grads_older;
std::vector<std::map<unsigned int, std::vector<Real> > > Kernel::_aux_var_vals;
std::vector<std::map<unsigned int, std::vector<RealGradient> > > Kernel::_aux_var_grads;
std::vector<std::map<unsigned int, std::vector<Real> > > Kernel::_aux_var_vals_old;
std::vector<std::map<unsigned int, std::vector<Real> > > Kernel::_aux_var_vals_older;
std::vector<std::map<unsigned int, std::vector<RealGradient> > > Kernel::_aux_var_grads_old;
std::vector<std::map<unsigned int, std::vector<RealGradient> > > Kernel::_aux_var_grads_older;
Real Kernel::_t;
Real Kernel::_dt;
Real Kernel::_dt_old;
int Kernel::_t_step;
short Kernel::_t_scheme;
short Kernel::_n_of_rk_stages;
Real Kernel::_bdf2_wei[3];
bool Kernel::_is_transient;
std::vector<Material *> Kernel::_static_material;
std::vector<Real> Kernel::_static_real_zero;
std::vector<std::vector<Real> > Kernel::_static_zero;
std::vector<std::vector<RealGradient> > Kernel::_static_grad_zero;
std::vector<std::vector<RealTensor> > Kernel::_static_second_zero;
