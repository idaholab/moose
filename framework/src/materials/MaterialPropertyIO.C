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

#include "MaterialProperty.h"
#include "MaterialPropertyIO.h"
#include "MaterialPropertyStorage.h"
#include "MooseMesh.h"
#include "FEProblem.h"
#include <cstring>


const unsigned int MaterialPropertyIO::file_version = 4;

struct MSMPHeader
{
  char _id[4];                  // 4 letter ID
  unsigned int _file_version;   // file version
};


MaterialPropertyIO::MaterialPropertyIO(FEProblem & fe_problem) :
    _fe_problem(fe_problem),
    _mesh(_fe_problem.mesh()),
    _material_props(_fe_problem._material_props),
    _bnd_material_props(_fe_problem._bnd_material_props)
{
}

MaterialPropertyIO::~MaterialPropertyIO()
{
}

void
MaterialPropertyIO::write(const std::string & file_name)
{
  processor_id_type proc_id = libMesh::processor_id();

  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & props = _material_props.props();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & propsOld = _material_props.propsOld();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & propsOlder = _material_props.propsOlder();

  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_props = _bnd_material_props.props();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_propsOld = _bnd_material_props.propsOld();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_propsOlder = _bnd_material_props.propsOlder();

  std::ostringstream file_name_stream;
  file_name_stream << file_name;
  file_name_stream << "-" << proc_id;

  std::ofstream out;

  out.open(file_name_stream.str().c_str(), std::ios::out | std::ios::binary);

  // version
  storeHelper(out, file_version, NULL);

  storeHelper(out, props, &_mesh);
  storeHelper(out, propsOld, &_mesh);

  if(_material_props.hasOlderProperties())
    storeHelper(out, propsOlder, &_mesh);

  storeHelper(out, bnd_props, &_mesh);
  storeHelper(out, bnd_propsOld, &_mesh);

  if(_bnd_material_props.hasOlderProperties())
    storeHelper(out, bnd_propsOlder, &_mesh);

  out.close();
}

void
MaterialPropertyIO::read(const std::string & file_name)
{
  processor_id_type proc_id = libMesh::processor_id();

  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & props = _material_props.props();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & propsOld = _material_props.propsOld();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & propsOlder = _material_props.propsOlder();

  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_props = _bnd_material_props.props();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_propsOld = _bnd_material_props.propsOld();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_propsOlder = _bnd_material_props.propsOlder();

  std::ostringstream file_name_stream;
  file_name_stream << file_name;
  file_name_stream << "-" << proc_id;

  std::ifstream in;

  in.open(file_name_stream.str().c_str(), std::ios::in | std::ios::binary);

  unsigned int read_file_version;

  // version
  loadHelper(in, read_file_version, NULL);

  if(read_file_version != file_version)
    mooseError("The stateful MaterialProperty checkpoint file you are attempting to read is incompatible with this version of MOOSE!");

  loadHelper(in, props, &_mesh);
  loadHelper(in, propsOld, &_mesh);

  if(_material_props.hasOlderProperties())
    loadHelper(in, propsOlder, &_mesh);

  loadHelper(in, bnd_props, &_mesh);
  loadHelper(in, bnd_propsOld, &_mesh);

  if(_bnd_material_props.hasOlderProperties())
    loadHelper(in, bnd_propsOlder, &_mesh);

  in.close();
}
