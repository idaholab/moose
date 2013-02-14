#include "PFCFreezingIC.h"
#include "MooseRandom.h"

template<>
InputParameters validParams<PFCFreezingIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<Real>("x1", "The x coordinate of the lower left-hand corner of the frozen box");
  params.addRequiredParam<Real>("y1", "The y coordinate of the lower left-hand corner of the frozen box");
  params.addParam<Real>("z1", 0.0, "The z coordinate of the lower left-hand corner of the box");

  params.addRequiredParam<Real>("x2", "The x coordinate of the upper right-hand corner of the box");
  params.addRequiredParam<Real>("y2", "The y coordinate of the upper right-hand corner of the box");
  params.addParam<Real>("z2", 0.0, "The z coordinate of the upper right-hand corner of the box");

  params.addRequiredParam<Real>("lc", "The lattice constant off the crystal structure");
  params.addRequiredParam<std::string>("crystal_structure", "The type of crystal structure, can be FCC, BCC or HCP");
  
  params.addParam<unsigned int>("seed", 0, "Seed value for the random number generator");
  
  return params;
}

PFCFreezingIC::PFCFreezingIC(const std::string & name,
                             InputParameters parameters)
  :InitialCondition(name, parameters),
   _x1(getParam<Real>("x1")),
   _y1(getParam<Real>("y1")),
   _z1(getParam<Real>("z1")),
   _x2(getParam<Real>("x2")),
   _y2(getParam<Real>("y2")),
   _z2(getParam<Real>("z2")),
   _lc(getParam<Real>("lc")),
   _crystal_structure(getParam<std::string>("crystal_structure")),
   _bottom_left(_x1,_y1,_z1),
   _top_right(_x2,_y2,_z2),
   _range(_top_right - _bottom_left)
{
  for(unsigned int i=0; i<LIBMESH_DIM; i++)
    mooseAssert(_range(i) > 0.0, "x1, y1 or z1 is not less than x2, y2 or z2");
  
  MooseRandom::seed(getParam<unsigned int>("seed"));

  _range.print();
  
  if (_range(1) == 0.0)
    _icdim = 1;
  else if (_range(2) == 0.0)
    _icdim = 2;
  else
    _icdim = 3;
  
}

Real
PFCFreezingIC::value(const Point & p)
{
  //If out of bounds, set random value
  for(unsigned int i=0; i<LIBMESH_DIM; i++)
    if(p(i) < _bottom_left(i) || p(i) > _top_right(i))
      return MooseRandom::rand();
  
  //If in bounds, set sinusoid IC to make atoms

  Real val = 0.0;
  for (unsigned int i=0; i<2; i++) //This needs to be changed to icdim
    val += std::sin((4.0/_lc*p(i)-0.5)*libMesh::pi);

  if (_range(2) > 0)
    val = val/6.0 + 0.5;
  else
    val = val/4.0 + 0.5;
  
  return val;
  
  
}

  


  
