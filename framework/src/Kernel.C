#include "Kernel.h"

// libMesh includes
#include "dof_map.h"
#include "dense_vector.h"
#include "numeric_vector.h"

Kernel::Kernel(EquationSystems * es, std::string var_name)
  :_es(*es),
   _var_name(var_name),
   _mesh(_es.get_mesh()),
   _dim(_mesh.mesh_dimension()),
   _system(_es.get_system<NonlinearImplicitSystem>("NonlinearSystem")),
   _dof_map(_system.get_dof_map()),
   _fe_type(_dof_map.variable_type(0)),
   _fe(FEBase::build(_dim, _fe_type)),
   _qrule(_fe_type.default_quadrature_order()),
   _fe_face(FEBase::build(_dim, _fe_type)),
   _qface(_fe_type.default_quadrature_order()),
   _JxW(_fe->get_JxW()),
   _phi(_fe->get_phi()),
   _dphi(_fe->get_dphi())
{
  _fe->attach_quadrature_rule(&_qrule);
  _fe_face->attach_quadrature_rule(&_qface);
}

void
Kernel::computeElemResidual(const NumericVector<Number>& soln,
			    DenseVector<Number> & Re,
			    Elem * elem)
{
  _dof_map.dof_indices(elem, _dof_indices);
  _fe->reinit(elem); 
  Re.resize(_dof_indices.size());

  for (_qp=0; _qp<_qrule.n_points(); _qp++)
  {
    for (_i=0; _i<_phi.size(); _i++)
    {
      _u      +=  _phi[_i][_qp]*soln(_dof_indices[_i]);
      _grad_u += _dphi[_i][_qp]*soln(_dof_indices[_i]);
    }

    for (_i=0; _i<_phi.size(); _i++)
      computeQpResidual(Re);
  }
}
