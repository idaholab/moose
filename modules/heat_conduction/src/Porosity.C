#include "Porosity.h"

template<>
InputParameters validParams<Porosity>()
{
  InputParameters params = validParams<Material>();

  params.addCoupledVar("temp", "Coupled Temperature");

  params.addParam<Real>("porosity", "The porosity value");
  params.addParam<FunctionName>("porosity_temperature_function", "", "porosity as a function of temperature.");
  params.addParam<Real>("anneal_temp", "The temperature when porosity stops changing");
  

  return params;
}

Porosity::Porosity(const std::string & name, InputParameters parameters) :
    Material(name, parameters),

    _has_temp(isCoupled("temp")),
    _temperature(_has_temp ? coupledValue("temp") : _zero),
    _my_porosity(isParamValid("porosity") ? getParam<Real>("porosity") : 0),
    _porosity(declareProperty<Real>("porosity")),
    _porosity_temperature_function( getParam<FunctionName>("porosity_temperature_function") != "" ? &getFunction("porosity_temperature_function") : NULL),
    _anneal_temp(isParamValid("anneal_temp") ? getParam<Real>("anneal_temp") : 0),
    _annealed(declareProperty<int>("annealed")),
    _annealed_old(declarePropertyOld<int>("annealed"))
{
  if (_porosity_temperature_function && !_has_temp)
  {
    mooseError("Must couple with temperature if using porosity function");
  }
  if (isParamValid("porosity") && _porosity_temperature_function)
  {
    mooseError("Cannot define both porosity and porosity temperature function");
  }
  if (isParamValid("anneal_temp") && _my_porosity)
  {
    mooseError("Cannot define annealing temperature when porosity is a constant");
  }
  if (_has_temp && _my_porosity)
  {
    mooseError("Remove temp = temp when porosity is a constant");
  }
  if (_porosity_temperature_function && !isParamValid("anneal_temp"))
  {
    mooseError("When defining porosity as a function of temperature, anneal_temp must be defined");
  }
}

void
Porosity::initQpStatefulProperties()
{
  _porosity[_qp] = 0;
  _annealed[_qp] = 0;
  _annealed_old[_qp] = 0;
}

void
Porosity::computeProperties()
{
  for(unsigned int qp(0); qp < _qrule->n_points(); ++qp)
  {
    if(_anneal_temp && _porosity_temperature_function && _has_temp)
    {
      if(_annealed_old[qp] == 1)
      {
        Point p;
        _porosity[qp] = _porosity_temperature_function->value(_anneal_temp, p);
        _annealed[qp] = 1;
      }
      else
        if(_temperature[qp] >= _anneal_temp)
        {
          Point p;
          _porosity[qp] = _porosity_temperature_function->value(_anneal_temp, p);
          _annealed[qp] = 1;
        }
        else
        {
          Point p;
          _porosity[qp] = _porosity_temperature_function->value(_temperature[qp], p);
        }
    }
    else
      if(_my_porosity)
      {
        _porosity[qp] = _my_porosity;
      }    
//      std::cout << "qp = " << qp << std::endl;
//      std::cout << "annealed = " << _annealed[qp] << std::endl;
//      std::cout << "annealed_old = " << _annealed_old[qp] << std::endl;
//      std::cout << "porosity[qp] = " << _porosity[qp] << std::endl;
//      std::cout << "temp = " << _temperature[qp] << std::endl;
//      std::cout << "time = " << _t << std::endl;
//      std::cout << "  " << std::endl;
  }
}
