#include "Kernel.h"
#include "Moose.h"
#include "MaterialFactory.h"

// libMesh includes
#include "dof_map.h"
#include "dense_vector.h"
#include "numeric_vector.h"
#include "dense_subvector.h"

Kernel::Kernel(Parameters parameters,
               std::string var_name,
               bool integrated,
               std::vector<std::string> coupled_to,
               std::vector<std::string> coupled_as)
  :_parameters(parameters),
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
   _JxW_face(*_static_JxW_face),
   _phi_face(*_static_phi_face),
   _dphi_face(*_static_dphi_face),
   _normals_face(*_static_normals_face),
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

  _fe_face = FEBase::build(_dim, _fe_type);
  _qface = new QGauss(_dim-1,_fe_type.default_quadrature_order());
  _fe_face->attach_quadrature_rule(_qface);

  _static_JxW = &_fe->get_JxW();
  _static_phi = &_fe->get_phi();
  _static_dphi = &_fe->get_dphi();
  _static_q_point = &_fe->get_xyz();
  _static_JxW_face = &_fe_face->get_JxW();
  _static_phi_face = &_fe_face->get_phi();
  _static_dphi_face = &_fe_face->get_dphi();
  _static_normals_face = &_fe_face->get_normals();
             
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
      computeQpSolution(_var_vals[var_num][qp], soln, _var_dof_indices[var_num], qp);
      computeQpGradSolution(_var_grads[var_num][qp], soln, _var_dof_indices[var_num], qp);

      if(_is_transient)
      {
        computeQpSolution(_var_vals_old[var_num][qp], *_system->old_local_solution, _var_dof_indices[var_num], qp);
        computeQpGradSolution(_var_grads_old[var_num][qp], *_system->old_local_solution, _var_dof_indices[var_num], qp);
        
        computeQpSolution(_var_vals_older[var_num][qp], *_system->older_local_solution, _var_dof_indices[var_num], qp);
        computeQpGradSolution(_var_grads_older[var_num][qp], *_system->older_local_solution, _var_dof_indices[var_num], qp);
      }
    }
  }

  _material = MaterialFactory::instance()->getMaterial(elem->subdomain_id());
  _material->materialReinit();
  
  Moose::perf_log.pop("reinit()","Kernel");
}

void
Kernel::computeElemResidual()
{
  Moose::perf_log.push("computeElemResidual()","Kernel");

  DenseSubVector<Number> & var_Re = *_var_Res[_var_num];

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    for (_i=0; _i<_phi.size(); _i++)
      var_Re(_i) += _JxW[_qp]*computeQpResidual();
  
  Moose::perf_log.pop("computeElemResidual()","Kernel");
}

void
Kernel::computeElemJacobian()
{
  Moose::perf_log.push("computeElemJacobian()","Kernel");

  DenseSubMatrix<Number> & var_Ke = *_var_Kes[_var_num];

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    for (_i=0; _i<_phi.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
        var_Ke(_i,_j) += _JxW[_qp]*computeQpJacobian();
  
  Moose::perf_log.pop("computeElemJacobian()","Kernel");
}

void
Kernel::computeSideResidual(const NumericVector<Number>& soln,
			    const Elem * elem,
			    unsigned int side)
{
  Moose::perf_log.push("computeSideResidual()","Kernel");

  _fe_face->reinit(elem, side);

  std::vector<unsigned int> & var_dof_indices = _var_dof_indices[_var_num];
  DenseSubVector<Number> & var_Re = *_var_Res[_var_num];

  if(_integrated)
    for (_qp=0; _qp<_qface->n_points(); _qp++)
    {
      _u[_qp]=0;
      _grad_u[_qp]=0;
      for (_i=0; _i<_phi_face.size(); _i++)
      {
        _u[_qp]      +=  _phi_face[_i][_qp]*soln(var_dof_indices[_i]);
        _grad_u[_qp] += _dphi_face[_i][_qp]*soln(var_dof_indices[_i]);
      }

      for (_i=0; _i<_phi_face.size(); _i++)
	var_Re(_i) += _JxW_face[_qp]*computeQpResidual();
    } 
  else
  {
    // Do this because U is evaluated at the nodes
    _u.resize(1);
    _qp = 0;
    
    for(_i=0; _i<_phi_face.size(); _i++)
    {
      if(elem->is_node_on_side(_i,side))
      {
	_u[0] = soln(var_dof_indices[_i]);
	var_Re(_i) = computeQpResidual();
      }
    }
  }

  Moose::perf_log.pop("computeSideResidual()","Kernel");
}

void
Kernel::computeSideJacobian(const NumericVector<Number>& soln,
			    const Elem * elem,
			    unsigned int side)
{
  Moose::perf_log.push("computeSideJacobian()","Kernel");

  _fe_face->reinit(elem, side);

  std::vector<unsigned int> & var_dof_indices = _var_dof_indices[_var_num];
  DenseSubMatrix<Number> & var_Ke = *_var_Kes[_var_num];

  if(_integrated)
    for (_qp=0; _qp<_qface->n_points(); _qp++)
    {
      _u[_qp]=0;
      _grad_u[_qp]=0;
      for (_i=0; _i<_phi_face.size(); _i++)
      {
        _u[_qp]      +=  _phi_face[_i][_qp]*soln(var_dof_indices[_i]);
        _grad_u[_qp] += _dphi_face[_i][_qp]*soln(var_dof_indices[_i]);
      }

      for (_i=0; _i<_phi_face.size(); _i++)
        for (_j=0; _j<_phi_face.size(); _j++)
          var_Ke(_i,_j) += _JxW_face[_qp]*computeQpJacobian();
    } 
  else
  {
    // Do this because U is evaluated at the nodes
    _u.resize(1);
    _qp = 0;
    
    for(_i=0; _i<_phi_face.size(); _i++)
    {
      if(elem->is_node_on_side(_i,side))
      {
        for(_j=0; _j<_phi_face.size(); _j++)
          var_Ke(_i,_j) = 0;
        
	_u[0] = soln(var_dof_indices[_i]);
	var_Ke(_i,_i) = 1;
      }
    }
  }

  Moose::perf_log.pop("computeSideJacobian()","Kernel");
}

void
Kernel::computeQpSolution(Real & u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, unsigned int qp)
{
  u=0;

  const std::vector<std::vector<Real> > & phi = *_static_phi;

  for (unsigned int i=0; i<phi.size(); i++) 
  {
    u +=  phi[i][qp]*soln(dof_indices[i]);
  }
}

void
Kernel::computeQpGradSolution(RealGradient & grad_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, unsigned int qp)
{
  grad_u=0;

  const std::vector<std::vector<RealGradient> > & dphi = *_static_dphi;

  for (unsigned int i=0; i<dphi.size(); i++) 
  {
    grad_u += dphi[i][qp]*soln(dof_indices[i]);
  }
}

std::vector<Real> &
Kernel::coupledVal(std::string name)
{
  return _var_vals[_coupled_as_to_var_num[name]];
}

std::vector<RealGradient> &
Kernel::coupledGrad(std::string name)
{
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
AutoPtr<FEBase> Kernel::_fe_face;
QGauss * Kernel::_qface;
const std::vector<Real> * Kernel::_static_JxW;
const std::vector<std::vector<Real> > * Kernel::_static_phi;
const std::vector<std::vector<RealGradient> > * Kernel::_static_dphi;
const std::vector<Point> * Kernel::_static_q_point;
const std::vector<Real> * Kernel::_static_JxW_face;
const std::vector<std::vector<Real> > * Kernel::_static_phi_face;
const std::vector<std::vector<RealGradient> > * Kernel::_static_dphi_face;
const std::vector<Point> * Kernel::_static_normals_face;
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
