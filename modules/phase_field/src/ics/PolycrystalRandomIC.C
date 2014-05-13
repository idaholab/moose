#include "PolycrystalRandomIC.h"
#include "MooseRandom.h"

template<>
InputParameters validParams<PolycrystalRandomIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<unsigned int>("crys_num", "Number of order parameters");
  params.addRequiredParam<unsigned int>("crys_index", "The index for the current order parameter");

  params.addRequiredParam<unsigned int>("typ", "Type of random grain structure");

  return params;
}

PolycrystalRandomIC::PolycrystalRandomIC(const std::string & name,
                             InputParameters parameters) :
    InitialCondition(name, parameters),
    _crys_num(getParam<unsigned int>("crys_num")),
    _crys_index(getParam<unsigned int>("crys_index")),
    _typ(getParam<unsigned int>("typ"))
{}

Real PolycrystalRandomIC::value(const Point & p)
{
  Point cur_pos = p;
  Real val =  MooseRandom::rand();

  switch (_typ)
  {
    case 0: //Continuously random
      return val;

    case 1: //Discretely random
    {
      unsigned int rndind = _crys_num * val;

      if (rndind == _crys_index)
        return 1.0;
      else
        return 0.0;
    }
  }

  mooseError("Bad case passed in PolycrystalRandomIC");
}
