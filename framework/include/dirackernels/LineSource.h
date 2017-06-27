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

#ifndef LINESOURCE_H
#define LINESOURCE_H

// Moose Includes
#include "DiracKernel.h"
#include "FunctionParserUtils.h"

// Forward Declarations
class LineSource;

template <>
InputParameters validParams<LineSource>();

/**
 * TOOD
 */
class LineSource : public DiracKernel, public FunctionParserUtils
{
public:
  LineSource(const InputParameters & parameters);

  virtual void addPoints() override;

protected:
  virtual Real computeQpResidual() override;

  /// helper function setting up parsed functions
  void setupParsedFunctionObject(const std::string & function, ADFunctionPtr & func_F);

  /// minimum t along curve parameterized in t
  const Real _tmin;

  /// maximum t along curve parameterized in t
  const Real _tmax;

  /// precision
  const Real _eps;

  /// function parser object describing the combinatorial geometry as function of t
  std::vector<ADFunctionPtr> _curve;

  /// function parser object describing the length as function of t
  ADFunctionPtr _length;

  /// function parser object describing the strength as function of t
  ADFunctionPtr _strength;

  /// a vector of the points being added
  std::vector<Point> _points;

  /// a vector of point source strengths being added
  std::map<Point, Real> _point_strength;
};

#endif // LineSource_H
