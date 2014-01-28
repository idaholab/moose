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

#ifndef BOUNDARYUSEROBJECT_H
#define BOUNDARYUSEROBJECT_H

#include "SideUserObject.h"

class BoundaryUserObject;

template<>
InputParameters validParams<BoundaryUserObject>();

/**
 *
 */
class BoundaryUserObject : public SideUserObject
{
public:
	BoundaryUserObject(const std::string & name, InputParameters parameters);
	virtual ~BoundaryUserObject();

	virtual void initialize();
	virtual void execute();
	virtual void finalize();
	virtual void threadJoin(const UserObject & uo);

	Real getValue() const { return _value; }

protected:
	VariableValue & _u;

	Real _value;

	std::map<BoundaryID, Real> _factors;
};

#endif /* BOUNDARYUSEROBJECT_H */
