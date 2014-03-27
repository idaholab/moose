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
{
  _marked_elems.clear();
}

void
XFEMMarkerUserObject::execute()
{
  RealVectorValue direction;
  bool isCTE = _xfem->is_elem_at_crack_tip(_current_elem);
  if (isCTE && doesElementCrack(direction))
  {
    unsigned int _current_eid = _current_elem->id();
    std::map<unsigned int, RealVectorValue>::iterator mit;
    mit = _marked_elems.find(_current_eid);
    if (mit != _marked_elems.end())
    {
      mooseError("ERROR: element "<<_current_eid<<" already marked for crack growth.");
    }
    _marked_elems[_current_eid] = direction;
  }
}

void
XFEMMarkerUserObject::threadJoin(const UserObject &y)
{
  const XFEMMarkerUserObject &xmuo = dynamic_cast<const XFEMMarkerUserObject &>(y);

  for ( std::map<unsigned int, RealVectorValue>::const_iterator mit = xmuo._marked_elems.begin();
        mit != xmuo._marked_elems.end();
        ++mit )
  {
    _marked_elems[mit->first] = mit->second; //TODO do error checking for duplicates here too
  }
}

void
XFEMMarkerUserObject::finalize()
{
  //TODO: This doesn't compile.  My guess is that it's because of the RealVectorValue.
  //Parallel::set_union(_marked_elems); //TODO do error checking for duplicates here too

  std::map<unsigned int, RealVectorValue>::iterator mit;
  for (mit = _marked_elems.begin(); mit != _marked_elems.end(); ++mit)
  {
    _xfem->addStateMarkedElem(mit->first, mit->second);
  }
  _marked_elems.clear();
}

bool
XFEMMarkerUserObject::doesElementCrack(RealVectorValue &direction)
{
  direction(1) = 1.0;
  return true;
}
