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
  
  params.addParam<Real>("min", 0.0, "Lower bound of the randomly generated values");
  params.addParam<Real>("max", 1.0, "Upper bound of the randomly generated values");
  params.addParam<Real>("inside", 1.0, "Value inside sinusoids");
  params.addParam<Real>("outside", 0.0, "Value outside sinusoids");
  params.addParam<unsigned int>("power",2,"Power of sinusoid");

  params.addRequiredParam<Real>("lc", "The lattice constant off the crystal structure");
  
  MooseEnum crystal_structures("FCC, BCC");
  params.addParam<MooseEnum>("crystal_structure", crystal_structures,"The type of crystal structure");
  
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
   _crystal_structure(getParam<MooseEnum>("crystal_structure")),
   _bottom_left(_x1,_y1,_z1),
   _top_right(_x2,_y2,_z2),
   _range(_top_right - _bottom_left),
   _min(getParam<Real>("min")),
   _max(getParam<Real>("max")),
   _val_range(_max - _min),
   _inside(getParam<Real>("inside")),
   _outside(getParam<Real>("outside")),
   _power(getParam<unsigned int>("power"))
{
  std::cout << "MooseEnum? " << _crystal_structure << std::endl;
  
  for(unsigned int i=0; i<LIBMESH_DIM; i++)
    mooseAssert(_range(i) >= 0.0, "x1, y1 or z1 is not less than x2, y2 or z2");
  
  MooseRandom::seed(getParam<unsigned int>("seed"));
  
  if (_range(1) == 0.0)
    _icdim = 1;
  else if (_range(2) < 1.0e-10*_range(0))
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
      return _min + _val_range*MooseRandom::rand();
    
  //If in bounds, set sinusoid IC to make atoms

  Real val = 0.0;
  
  for (unsigned int i=0; i<_icdim; i++) 
    val += std::cos((2.0/_lc*p(i))*libMesh::pi);
  
  if (_range(2) > 0.0)
    val = val/6.0 + 0.5;
  else
    val = val/4.0 + 0.5;

  val = std::pow(val,static_cast<Real>(_power));
  
  if (_crystal_structure == "FCC")
  {
    Real val2 = 0.0;
    
    for (unsigned int i=0; i<_icdim; i++) 
      val2 += std::cos((2.0/_lc*p(i) + 1.0)*libMesh::pi);
    
    if (_range(2) > 0.0)
      val2 = val2/6.0 + 0.5;
    else
      val2 = val2/4.0 + 0.5;
    val2 = std::pow(val2,static_cast<Real>(_power));

    val += val2;

    if (_power == 2)
    val = 2.0*(val - 0.5);
    else if (_power == 3)
      val = (val - 0.25)/0.75;
    else if (_power == 4)
      val = (val - 0.1)/0.9;
  }

  val = _outside + (_inside - _outside)*val;
  
  return val;
  
  
  
}

  


  
