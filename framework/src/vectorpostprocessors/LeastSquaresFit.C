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

#include "LeastSquaresFit.h"
#include "VectorPostprocessorInterface.h"
#include "PolynomialFit.h"

template<>
InputParameters validParams<LeastSquaresFit>()
{
  InputParameters params = validParams<VectorPostprocessor>();

  params.addRequiredParam<VectorPostprocessorName>("vectorpostprocessor", "The vectorpostprocessor on whose values we perform a least squares fit");
  params.addRequiredParam<std::string>("x_name", "The name of the independent variable");
  params.addRequiredParam<std::string>("y_name", "The name of the dependent variable");
  params.addRequiredParam<unsigned int>("order", "The order of the polynomial fit");
  params.addParam<unsigned int>("num_samples", "The number of samples to be output");
  MooseEnum output_type("Coefficients Samples","Coefficients");
  params.addParam<MooseEnum>("output", output_type, "The quantity to output.  Options are: " + output_type.getRawNames());
  params.addClassDescription("Performs a polynomial least squares fit on the data contained in another VectorPostprocessor");

  return params;
}

LeastSquaresFit::LeastSquaresFit(const std::string & name, InputParameters parameters) :
    GeneralVectorPostprocessor(name, parameters),
    VectorPostprocessorInterface(parameters),
    _vpp_name(getParam<VectorPostprocessorName>("vectorpostprocessor")),
    _order(parameters.get<unsigned int>("order")),
    _x_name(getParam<std::string>("x_name")),
    _y_name(getParam<std::string>("y_name")),
    _x_values(NULL),
    _y_values(NULL),
    _output_type(getParam<MooseEnum>("output")),
    _num_samples(0),
    _sample_x(NULL),
    _sample_y(NULL),
    _coeffs(NULL)
{
  if (_output_type == "Samples")
  {
    if (isParamValid("num_samples"))
      _num_samples = getParam<unsigned int>("num_samples");
    else
      mooseError("In LeastSquaresFit num_samples parameter must be provided with output=Samples");
    _sample_x = &declareVector(_x_name);
    _sample_y = &declareVector(_y_name);
  }
  else
  {
    if (isParamValid("num_samples"))
      mooseWarning("In LeastSquaresFit num_samples parameter is unused with output=Coefficients");
    _coeffs = &declareVector("coefficients");
  }
}

void
LeastSquaresFit::initialize()
{
  if (!_x_values)
  {
    if (!hasVectorPostprocessorByName(_vpp_name))
      mooseError("In LeastSquaresFit, VectorPostprocessor with name: "<<_vpp_name<<" does not exist");
    _x_values = &getVectorPostprocessorValueByName(_vpp_name, _x_name);
    _y_values = &getVectorPostprocessorValueByName(_vpp_name, _y_name);
    if (_output_type == "Samples")
    {
      _sample_x->resize(_num_samples);
      _sample_y->resize(_num_samples);
    }
    else
      _coeffs->resize(_order+1);
  }

  if (_output_type == "Samples")
  {
    _sample_x->clear();
    _sample_y->clear();
  }
  else
    _coeffs->clear();
}

void
LeastSquaresFit::execute()
{
  if (_x_values->size() != _y_values->size())
    mooseError("In LeastSquresFit size of data in x_values and y_values must be equal");
  if (_x_values->size() == 0)
    mooseError("In LeastSquresFit size of data in x_values and y_values must be > 0");
  PolynomialFit pf(*_x_values, *_y_values, _order, true);
  pf.generate();


  if (_output_type == "Samples")
  {
    Real x_min = *(std::min_element(_x_values->begin(), _x_values->end()));
    Real x_max = *(std::max_element(_x_values->begin(), _x_values->end()));
    Real x_span = x_max - x_min;

    for (unsigned int i=0; i<_num_samples; ++i)
    {
      Real x = x_min + (Real)i / _num_samples * x_span;
      _sample_x->push_back(x);
      _sample_y->push_back(pf.sample(x));
    }
  }
  else
    *_coeffs = pf.getCoefficients();
}
