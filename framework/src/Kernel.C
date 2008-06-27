#include "Kernel.h"

// libMesh includes
#include "dof_map.h"
#include "dense_vector.h"
#include "numeric_vector.h"
#include "dense_subvector.h"

Kernel::Kernel(Parameters parameters, EquationSystems * es, std::string var_name, bool integrated, std::vector<std::string> coupled_to)
  :_parameters(parameters),
   _integrated(integrated),
   _es(*es),
   _var_name(var_name),
   _mesh(_es.get_mesh()),
   _dim(_mesh.mesh_dimension()),
   _system(_es.get_system<TransientNonlinearImplicitSystem>("NonlinearSystem")),
   _var_num(_system.variable_number(_var_name)),
   _dof_map(_system.get_dof_map()),
   _fe_type(_dof_map.variable_type(0)),
   _fe(FEBase::build(_dim, _fe_type)),
   _qrule(_dim,_fe_type.default_quadrature_order()),
   _fe_face(FEBase::build(_dim, _fe_type)),
   _qface(_dim-1,_fe_type.default_quadrature_order()),
   _JxW(_fe->get_JxW()),
   _phi(_fe->get_phi()),
   _dphi(_fe->get_dphi()),
   _q_point(_fe->get_xyz()),
   _JxW_face(_fe_face->get_JxW()),
   _phi_face(_fe_face->get_phi()),
   _dphi_face(_fe_face->get_dphi()),
   _t(0),
   _dt(0),
   _is_transient(false),
   _coupled_to(coupled_to)
{
  _fe->attach_quadrature_rule(&_qrule);
  _fe_face->attach_quadrature_rule(&_qface);

  if(_es.parameters.have_parameter<Real>("time") && _es.parameters.have_parameter<Real>("dt"))
  {
    _is_transient = true;
    _t = _es.parameters.get<Real>("time");
    _dt = _es.parameters.get<Real>("dt");
  }

  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    _coupled_var_nums[i]=_system.variable_number(var_name);
    _coupled_dof_indices[i];
    _coupled_vals[coupled_var_name]=0;
    _coupled_grads[coupled_var_name]=0;
  }
}

Kernel::Kernel(EquationSystems * es, std::string var_name, bool integrated, std::vector<std::string> coupled_to)
  :_integrated(integrated),
   _es(*es),
   _var_name(var_name),
   _mesh(_es.get_mesh()),
   _dim(_mesh.mesh_dimension()),
   _system(_es.get_system<TransientNonlinearImplicitSystem>("NonlinearSystem")),
   _var_num(_system.variable_number(_var_name)),
   _dof_map(_system.get_dof_map()),
   _fe_type(_dof_map.variable_type(0)),
   _fe(FEBase::build(_dim, _fe_type)),
   _qrule(_dim,_fe_type.default_quadrature_order()),
   _fe_face(FEBase::build(_dim, _fe_type)),
   _qface(_dim-1,_fe_type.default_quadrature_order()),
   _JxW(_fe->get_JxW()),
   _phi(_fe->get_phi()),
   _dphi(_fe->get_dphi()),
   _q_point(_fe->get_xyz()),
   _JxW_face(_fe_face->get_JxW()),
   _phi_face(_fe_face->get_phi()),
   _dphi_face(_fe_face->get_dphi()),
   _t(0),
   _dt(0),
   _is_transient(false),
   _coupled_to(coupled_to)
{
  _fe->attach_quadrature_rule(&_qrule);
  _fe_face->attach_quadrature_rule(&_qface);

  if(_es.parameters.have_parameter<Real>("time") && _es.parameters.have_parameter<Real>("dt"))
  {
    _is_transient = true;
    _t = _es.parameters.get<Real>("time");
    _dt = _es.parameters.get<Real>("dt");
  }

  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    _coupled_var_nums[i]=_system.variable_number(var_name);
    _coupled_dof_indices[i];
    _coupled_vals[coupled_var_name]=0;
    _coupled_grads[coupled_var_name]=0;
  }
}

void
Kernel::computeElemResidual(const NumericVector<Number>& soln,
			    DenseVector<Number> & Re,
			    const Elem * elem)
{
  _dof_map.dof_indices(elem, _dof_indices);
  _dof_map.dof_indices(elem, _var_dof_indices, _var_num);

  for(unsigned int i=0; i<_coupled_to.size(); i++)
    _dof_map.dof_indices(elem, _coupled_dof_indices[i], _coupled_var_nums[i]);
    

  _fe->reinit(elem);

  if(Re.size() != _dof_indices.size())
    Re.resize(_dof_indices.size());

  _var_num_dofs = _var_dof_indices.size();

  DenseSubVector<Number> var_Re(Re,_var_num*_var_num_dofs,_var_num_dofs);

  for (_qp=0; _qp<_qrule.n_points(); _qp++)
  {
    _u=computeQpSolution(soln, _var_dof_indices);
    _grad_u=computeQpGradSolution(soln, _var_dof_indices);

    if(_is_transient)
    {
      _u_old = computeQpSolution(*_system.old_local_solution, _var_dof_indices);
      _grad_u_old = computeQpGradSolution(*_system.old_local_solution, _var_dof_indices);
    }

    for(unsigned int i=0; i<_coupled_to.size(); i++)
    {
      std::string coupled_var_name=_coupled_to[i];

      _coupled_vals[coupled_var_name]=computeQpSolution(soln, _coupled_dof_indices[i]);
      _coupled_grads[coupled_var_name]=computeQpGradSolution(soln, _coupled_dof_indices[i]);
    }

    for(unsigned int i=0; i<_coupled_to.size(); i++)


    for (_i=0; _i<_phi.size(); _i++)
    {
      var_Re(_i) += computeQpResidual();

      if(_is_transient)
	var_Re(_i) += computeQpTransientResidual();
    }
  }
}

void
Kernel::computeSideResidual(const NumericVector<Number>& soln,
			    DenseVector<Number> & Re,
			    const Elem * elem,
			    unsigned int side)
{
  _dof_map.dof_indices(elem, _dof_indices);
  _dof_map.dof_indices(elem, _var_dof_indices, _var_num);
  _fe_face->reinit(elem, side);

  _var_num_dofs = _var_dof_indices.size();

  DenseSubVector<Number> var_Re(Re,_var_num*_var_num_dofs,_var_num_dofs);

  if(_integrated)
    for (_qp=0; _qp<_qface.n_points(); _qp++)
    {
      _u=0;
      _grad_u=0;
      for (_i=0; _i<_phi_face.size(); _i++)
      {
        _u      +=  _phi_face[_i][_qp]*soln(_var_dof_indices[_i]);
        _grad_u += _dphi_face[_i][_qp]*soln(_var_dof_indices[_i]);
      }

      for (_i=0; _i<_phi_face.size(); _i++)
	var_Re(_i) += computeQpResidual();
    } 
  else
    for(_i=0; _i<_phi_face.size(); _i++)
    {
      if(elem->is_node_on_side(_i,side))
      {
	_u = soln(_dof_indices[_i]);
	var_Re(_i) = computeQpResidual();
      }
    }
}

Real
Kernel::computeQpSolution(const NumericVector<Number>& soln, const std::vector<unsigned int>& dof_indices)
{
  Real u=0;

  for (_i=0; _i<_phi.size(); _i++)
  {
    u +=  _phi[_i][_qp]*soln(dof_indices[_i]);
  }

  return u;
}

RealGradient
Kernel::computeQpGradSolution(const NumericVector<Number>& soln, const std::vector<unsigned int>& dof_indices)
{
  RealGradient grad_u=0;

  for (_i=0; _i<_dphi.size(); _i++) 
  {
    grad_u += _dphi[_i][_qp]*soln(dof_indices[_i]);
  }

  return grad_u;
}
