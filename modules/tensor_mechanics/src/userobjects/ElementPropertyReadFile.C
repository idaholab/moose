/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ElementPropertyReadFile.h"
#include "MooseRandom.h"

template<>
InputParameters validParams<ElementPropertyReadFile>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addParam<std::string>("prop_file_name", "", "Name of the property file name");
  params.addRequiredParam<unsigned int>("nprop", "Number of tabulated property values");
  params.addParam<unsigned int>("ngrain",0,"Number of grains");
  MooseEnum read_options("element grain none","none");
  params.addParam<MooseEnum>("read_type", read_options, "Type of property distribution: element:element by element property variation; grain:voronoi grain structure");
  params.addParam<unsigned int>("rand_seed", 2000, "random seed");
  MooseEnum rve_options("periodic none","none");
  params.addParam<MooseEnum>("rve_type",rve_options , "Periodic or non-periodic grain distribution: Default is non-periodic");
  params.addClassDescription("User Object to read property data from an external file and assign to elements: Works only for Rectangular geometry (2D-3D)");

  return params;
}

ElementPropertyReadFile::ElementPropertyReadFile(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name,parameters),
    _prop_file_name(getParam<std::string>("prop_file_name")),
    _nprop(getParam<unsigned int>("nprop")),
    _ngrain(getParam<unsigned int>("ngrain")),
    _read_type(getParam<MooseEnum>("read_type")),
    _rand_seed(getParam<unsigned int>("rand_seed")),
    _rve_type(getParam<MooseEnum>("rve_type")),
    _mesh(_fe_problem.mesh())
{
  _nelem = _mesh.nElem();

  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
    _range(i) = _top_right(i) - _bottom_left(i);
  }

  for (unsigned int i = 1; i < LIBMESH_DIM; i++)
    if ( _range(i) > _max_range )
      _max_range = _range(i);


  switch (_read_type)
  {

  case 0:
    readElementData();
    break;
  case 1:
    readGrainData();
    break;
  default:
    mooseError("Error ElementPropertyReadFile: Provide valid read type");


  }



}


void
ElementPropertyReadFile::readElementData()
{

  _data.resize(_nprop * _nelem);

  MooseUtils::checkFileReadable(_prop_file_name);

  std::ifstream file_prop;
  file_prop.open(_prop_file_name.c_str());

  for ( unsigned int i=0; i < _nelem; i++)
    for ( unsigned int j = 0; j < _nprop; j++ )
      if (!(file_prop >> _data[ i * _nprop + j]))
        mooseError("Error ElementPropertyReadFile: Premature end of file");

  file_prop.close();

}


void
ElementPropertyReadFile::readGrainData()
{
  mooseAssert( _ngrain > 0, "Error ElementPropertyReadFile: Provide non-zero number of grains" );

  _data.resize(_nprop * _ngrain);

  MooseUtils::checkFileReadable(_prop_file_name);

  std::ifstream file_prop;
  file_prop.open(_prop_file_name.c_str());

  for ( unsigned int i=0; i < _ngrain; i++)
    for ( unsigned int j = 0; j < _nprop; j++ )
      if (!(file_prop >> _data[ i * _nprop + j]))
        mooseError("Error ElementPropertyReadFile: Premature end of file");

  file_prop.close();


  initGrainCenterPoints();


}


void
ElementPropertyReadFile::initGrainCenterPoints()
{

  _center.resize(_ngrain);

  MooseRandom::seed(_rand_seed);

    for ( unsigned int i = 0; i < _ngrain; i++ )
      for ( unsigned int j = 0; j < LIBMESH_DIM; j++ )
        _center[i](j) = _bottom_left(j) + MooseRandom::rand()*_range(j);

}


Real
ElementPropertyReadFile::getData( const Elem * elem , unsigned int prop_num ) const
{

  Real val = 0.0;

  switch (_read_type)
  {

  case 0:
    val =  getElementData(elem , prop_num);
    break;
  case 1:
    val =  getGrainData(elem, prop_num);
    break;
  default:
    mooseError("Error ElementPropertyReadFile: Provide valid read type");


  }

  return val;

}


Real
ElementPropertyReadFile::getElementData(const Elem * elem , unsigned int prop_num) const
{

  unsigned int jelem = elem->id();

  mooseAssert( jelem < _nelem , "Error ElementPropertyReadFile: Element " << jelem << " greater than than total number of element in mesh " << _nelem );
  mooseAssert( prop_num < _nprop , "Error ElementPropertyReadFile: Property number " << prop_num << " greater than than total number of properties " << _nprop );

  return _data[ jelem * _nprop + prop_num ];

}


Real
ElementPropertyReadFile::getGrainData(const Elem * elem , unsigned int prop_num) const
{

  mooseAssert( prop_num < _nprop , "Error ElementPropertyReadFile: Property number " << prop_num << " greater than than total number of properties " << _nprop << "\n");

  Point centroid = elem->centroid();

  Real min_dist = _max_range;
  unsigned int igrain = 0;

  for ( unsigned int i = 0; i < _ngrain; i++ )
  {

    Point center = _center[i];
    Real dist = 0.0;

    switch ( _rve_type )
    {
    case 0:
      //Calculates minimum periodic distance when "periodic" is specified
      //for rve_type
      dist = minPeriodicDistance( _center[i], centroid);
      break;
    default:
      //Calculates minimum distance when nothing is specified
      //for rve_type
      Point dist_vec = _center[i] - centroid;
      dist = dist_vec.size();
    }

    if ( dist < min_dist )
    {
      min_dist = dist;
      igrain = i;

    }



  }


  return _data[ igrain * _nprop + prop_num ];

}

Real
ElementPropertyReadFile::minPeriodicDistance(Point c, Point p) const
{

  Point dist_vec = c - p;
  Real min_dist = dist_vec.size();

  Real fac[3];
  fac[0] = -1.0;
  fac[1] = 0.0;
  fac[2] = 1.0;

  Point p1;

  for ( unsigned int i = 0; i < 3; i++ )
    for ( unsigned int j = 0; j < 3; j++ )
      for ( unsigned int k = 0; k < 3; k++ )
      {
        Point p1;

        p1(0) = p(0) + fac[i] * _range(0);
        p1(1) = p(1) + fac[j] * _range(1);
        p1(2) = p(2) + fac[k] * _range(2);

        dist_vec = c - p1;
        Real dist = dist_vec.size();

        if ( dist < min_dist ) min_dist = dist;

      }

  return min_dist;

}
