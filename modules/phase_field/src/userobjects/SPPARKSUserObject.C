#include "SPPARKSUserObject.h"

// SPPARKS
#include "SPPARKS.h"

template<>
InputParameters validParams<SPPARKSUserObject>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addParam<std::string>("file", "", "SPPARKS input file");

  // Hide from input file dump
  // params.addPrivateParam<std::string>("built_by_action", "" );
  return params;
}

SPPARKSUserObject::SPPARKSUserObject(const std::string & name, InputParameters params)
  :GeneralUserObject(name, params),
   _spparks(NULL),
   _file(getParam<std::string>("file"))
{
  char * file = new char[_file.length()+1];

  spparks_open( 0, NULL, libMesh::COMM_WORLD, &_spparks );
  if (!_spparks)
  {
    mooseError("Error initializing SPPARKS");
  }

  spparks_file(_spparks, file);

  delete[] file;
}

SPPARKSUserObject::~SPPARKSUserObject()
{
  spparks_close( _spparks );
}

Real
SPPARKSUserObject::getValue( const std::string & quantity ) const
{
  Real value = 0;
  return value;
}

void
SPPARKSUserObject::initialize()
{
}

void
SPPARKSUserObject::execute()
{
}
