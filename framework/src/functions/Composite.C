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

#include "Composite.h"
#include "Moose.h"

template<>
InputParameters validParams<Composite>()
{
  InputParameters params = validParams<Function>();
  params.addParam<std::vector<std::string> >("functions", "The functions to be multiplied together.");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the ordinate values");
  return params;
}

Composite::Composite(const std::string & name, InputParameters parameters) :
  Function(name, parameters),
  FunctionInterface( parameters ),
  _scale_factor( getParam<Real>("scale_factor") )
{

  const std::vector<std::string> & names( getParam<std::vector<std::string> >("functions") );
  const unsigned len( names.size() );
  if (len == 0)
  {
    mooseError( "A composite function must reference at least one other function" );
  }
  _f.resize(len);
  for (unsigned i(0); i < len; ++i)
  {
    if (_name == names[i])
    {
      mooseError( "A composite function must not reference itself" );
    }
    Function * const f = &getFunctionByName( names[i] );
    if (!f)
    {
      std::string msg("Error in composite function ");
      msg += _name;
      msg += ".  Function ";
      msg += names[i];
      msg += " referenced but not found.";
      mooseError( msg );
    }
    _f[i] = f;
  }

}

Composite::~Composite()
{
}

Real
Composite::value(Real t, const Point & p)
{
  Real val(_scale_factor);
  for (unsigned i(0); i < _f.size(); ++i)
  {
    val *= _f[i]->value( t, p );
  }
  return val;
}
