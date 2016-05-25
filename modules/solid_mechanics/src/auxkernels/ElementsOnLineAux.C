/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ElementsOnLineAux.h"

template<>
InputParameters validParams<ElementsOnLineAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredParam<RealVectorValue>("line1", "First point of line along which to pick elements");
  params.addRequiredParam<RealVectorValue>("line2", "Second point of line along which to pick elements");
  params.addRequiredParam<Real>("dist_tol", "Tolerance for distance between element and line");
  params.addParam<int>("line_id", 1, "ID of the line along which to pick elements");

  params.set<MultiMooseEnum>("execute_on") = "initial";

  return params;
}

ElementsOnLineAux::ElementsOnLineAux(const InputParameters & parameters) : AuxKernel(parameters),

  _line1( getParam<RealVectorValue>("line1") ),
  _line2( getParam<RealVectorValue>("line2") ),
  _dist_tol( getParam<Real>("dist_tol") ),
  _line_id( getParam<int>("line_id"))

{
  if ( _nodal ) mooseError("From ElementsOnLineAux: element on line id must be an element property (use CONSTANT MONOMIAL)");
}

void
ElementsOnLineAux::compute()
{

  const Point elem_pos(_current_elem->centroid());

  const Point line_vec(_line2 - _line1);
  const Real length(line_vec.norm());
  const Point line_unit_vec(line_vec / length);

  const Point line1_to_elem_vec(elem_pos - _line1);
  const Real proj(line1_to_elem_vec * line_unit_vec);
  const Point proj_vec(proj * line_unit_vec);
  const Point dist_vec(line1_to_elem_vec - proj_vec);

  const Real distance(dist_vec.norm());

  if (distance < _dist_tol)
  {
    /**
     * Update the variable data refernced by other kernels.
     * Note that this will update the values at the quadrature points too
     * (because this is an Elemental variable)
     */
    _var.setNodalValue(_line_id);
  }
}

Real
ElementsOnLineAux::computeValue()
{
  mooseError("From ElementsOnLineAux: computeValue() is not defined");
  return 0;
}

