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

#include "XFEMMarkerUserObject.h"

#include "XFEM.h"

template<>
InputParameters validParams<XFEMMarkerUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  return params;
}

XFEMMarkerUserObject::XFEMMarkerUserObject(const std::string & name, InputParameters parameters)
  :ElementUserObject(name, parameters)
{
  FEProblem * fe_problem = dynamic_cast<FEProblem *>(&_subproblem);
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblem in XFEMMarkerUserObject");
  _xfem = fe_problem->get_xfem();
  if (isNodal())
    mooseError("XFEMMarkerUserObject can only be run on an element variable");
}

void
XFEMMarkerUserObject::initialize()
{}

void
XFEMMarkerUserObject::execute()
{
  bool isCTE = _xfem->is_elem_at_crack_tip(_current_elem);
  if (isCTE)
  {
    RealVectorValue normal(0,1,0);
    _xfem->addStateMarkedElem(_current_elem, normal);
  }
}

void
XFEMMarkerUserObject::threadJoin(const UserObject &y)
{}

void
XFEMMarkerUserObject::finalize()
{}

//Real
//XFEMMarkerUserObject::computeValue()
//{
//  bool isCTE = _xfem->is_elem_at_crack_tip(_current_elem);
//  Real value = 0.0;
//  if (isCTE)
//  {
//    value = 1.0;
//  }
//
//  return value;
//}
