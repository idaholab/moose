/****************************************************************/
/*							 DO NOT MODIFY THIS HEADER											*/
/* MOOSE - Multiphysics Object Oriented Simulation Environment	*/
/*																															*/
/*					 (c) 2010 Battelle Energy Alliance, LLC							*/
/*									 ALL RIGHTS RESERVED												*/
/*																															*/
/*					Prepared by Battelle Energy Alliance, LLC						*/
/*						Under Contract No. DE-AC07-05ID14517							*/
/*						With the U. S. Department of Energy								*/
/*																															*/
/*						See COPYRIGHT for full restrictions								*/
/****************************************************************/

#include "BoundaryUserObject.h"

template<>
InputParameters validParams<BoundaryUserObject>()
{
	InputParameters params = validParams<SideUserObject>();
	params.addRequiredCoupledVar("variable", "the variable name");
	params.addParam<std::vector<Real> >("factors", "factors for all boundary side sets");
	return params;
}

BoundaryUserObject::BoundaryUserObject(const std::string & name, InputParameters parameters) :
		SideUserObject(name, parameters),
		_u(coupledValue("variable")),
		_value(0.)
{

  std::vector<BoundaryID> ids(boundaryIDs().begin(), boundaryIDs().end());

  if (isParamValid("factors"))
	{
		std::vector<Real> facs = getParam<std::vector<Real> >("factors");
		if (facs.size() != ids.size())
      mooseError("number of factors is wrong");
		for (unsigned int i = 0; i < ids.size(); i++)
			_factors[ids[i]] = facs[i];
	}
	else
	{
    for (unsigned int i = 0; i < numBoundaryIDs(); i++)
			_factors[ids[i]] = 1.0;
	}
}

BoundaryUserObject::~BoundaryUserObject()
{
}

void
BoundaryUserObject::initialize()
{
	_value = 0;
}

void
BoundaryUserObject::execute()
{
	for (unsigned int qp = 0; qp < _q_point.size(); ++qp)
		_value += _u[qp] * _factors[_current_boundary_id];
}

void
BoundaryUserObject::finalize()
{
	gatherSum(_value);
	_value = std::sqrt(_value);
}

void
BoundaryUserObject::threadJoin(const UserObject & uo)
{
	const BoundaryUserObject & u = dynamic_cast<const BoundaryUserObject &>(uo);
	_value += u._value;
}
