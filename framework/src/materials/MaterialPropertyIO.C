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

#include "MaterialPropertyIO.h"
#include "MaterialPropertyStorage.h"
#include "MooseMesh.h"
#include "FEProblem.h"
#include <cstring>


const unsigned int MaterialPropertyIO::file_version = 2;

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
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & props = _material_props.props();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & propsOld = _material_props.propsOld();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & propsOlder = _material_props.propsOlder();

  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_props = _bnd_material_props.props();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_propsOld = _bnd_material_props.propsOld();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_propsOlder = _bnd_material_props.propsOlder();

  std::vector<unsigned int> & prop_ids = _material_props.statefulProps();
  unsigned int n_props = prop_ids.size();        // number of properties in this block

  std::ofstream out;
  // head node writes the header
  if (libMesh::processor_id() == 0)
  {
    out.open(file_name.c_str(), std::ios::out | std::ios::binary);

    // header
    MSMPHeader head;
    std::memcpy(head._id, "MSMP", 4);
    head._file_version = file_version;
    out.write((const char *) &head, sizeof(head));

    // save the number of elements in this block (since we do only 1 block right now, we store everything)
    unsigned int n_elems = _mesh.nElem();
    out.write((const char *) &n_elems, sizeof(n_elems));

    // properties
    out.write((const char *) &n_props, sizeof(n_props));
    // property names
    for (unsigned int i = 0; i < n_props; i++)
    {
      unsigned int pid = prop_ids[i];
      std::string prop_name = _material_props.statefulPropNames()[pid];
      out.write(prop_name.c_str(), prop_name.length() + 1);                 // do not forget the trailing zero ;-)
    }

    out.close();
  }

  libMesh::Parallel::barrier(libMesh::CommWorld);

  // now each process dump its part, appending into the file
  for (unsigned int proc = 0; proc < libMesh::n_processors(); proc++)
  {
    if (libMesh::processor_id() == proc)
    {
      out.open(file_name.c_str(), std::ios::app | std::ios::binary);

      // save current material properties
      for (HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> >::iterator props_it=props.begin(); props_it != props.end(); ++props_it)
      {
        const Elem * elem = props_it->first;

        if (elem && elem->processor_id() == proc)
        {
          unsigned int elem_id = elem->id();
          out.write((const char *) &elem_id, sizeof(elem_id));

          // write out the properties into mem buffer
          std::ostringstream prop_blk;
          for (unsigned int i = 0; i < n_props; i++)
          {
            props[elem][0][i]->store(prop_blk);
            propsOld[elem][0][i]->store(prop_blk);
            if (_material_props.hasOlderProperties())
              propsOlder[elem][0][i]->store(prop_blk);
          }

          unsigned int prop_blk_size = prop_blk.tellp();
          out.write((const char *) &prop_blk_size, sizeof(prop_blk_size));

          out << prop_blk.str();
        }
      }

      out.close();
    }

    libMesh::Parallel::barrier(libMesh::CommWorld);
  }

  // again, each process dumps its part, appending into the file
  for (unsigned int proc = 0; proc < libMesh::n_processors(); proc++)
  {
    if (libMesh::processor_id() == proc)
    {
      out.open(file_name.c_str(), std::ios::app | std::ios::binary);

      // save current material properties
      for (HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> >::iterator props_it=props.begin(); props_it != props.end(); ++props_it)
      {
        const Elem * elem = props_it->first;

        if (elem && elem->processor_id() == proc)
        {
          unsigned int elem_id = elem->id();
          out.write((const char *) &elem_id, sizeof(elem_id));

          // save the material props on sides
          unsigned int n_sides = bnd_props[elem].size();
          out.write((const char *) &n_sides, sizeof(n_sides));

          for (HashMap<unsigned int, MaterialProperties>::iterator side_it = bnd_props[elem].begin(); side_it != bnd_props[elem].end(); ++side_it)
          {
            unsigned int s = side_it->first;
            out.write((const char *) &s, sizeof(s));

            // write out the properties into mem buffer
            std::ostringstream prop_blk;
            for (unsigned int i = 0; i < n_props; i++)
            {
              bnd_props[elem][s][i]->store(prop_blk);
              bnd_propsOld[elem][s][i]->store(prop_blk);
              if (_material_props.hasOlderProperties())
                bnd_propsOlder[elem][s][i]->store(prop_blk);
            }

            unsigned int prop_blk_size = prop_blk.tellp();
            out.write((const char *) &prop_blk_size, sizeof(prop_blk_size));

            out << prop_blk.str();
          }
        }
      }

      out.close();
    }

    libMesh::Parallel::barrier(libMesh::CommWorld);
  }
}

void
MaterialPropertyIO::read(const std::string & file_name)
{
  std::ifstream in(file_name.c_str(), std::ios::in | std::ios::binary);

  // header
  MSMPHeader head;
  in.read((char *) &head, sizeof(head));

  // check the header
  if (!(head._id[0] == 'M' && head._id[1] == 'S' && head._id[2] == 'M' && head._id[3] == 'P'))
    mooseError("Corrupted material properties file");
  // check the file version
  if (head._file_version > file_version)
    mooseError("Trying to restart from a newer file version - you need to update MOOSE");

  // grab some references we will need to later
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & props = _material_props.props();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & propsOld = _material_props.propsOld();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & propsOlder = _material_props.propsOlder();

  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_props = _bnd_material_props.props();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_propsOld = _bnd_material_props.propsOld();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_propsOlder = _bnd_material_props.propsOlder();

  std::map<unsigned int, std::string> stateful_prop_names = _material_props.statefulPropNames();
  std::map<std::string, unsigned int> stateful_prop_ids;                                // inverse map of stateful_prop_names
  for (std::map<unsigned int, std::string>::iterator it = stateful_prop_names.begin(); it != stateful_prop_names.end(); ++it)
    stateful_prop_ids[it->second] = it->first;

  // number of elements
  unsigned int n_elems = props.size();
  in.read((char *) &n_elems, sizeof(n_elems));
  // number of properties in this block
  unsigned int n_props = 0;
  in.read((char *) &n_props, sizeof(n_props));
  // property names
  std::vector<std::string> prop_names(n_props);
  std::vector<unsigned int> prop_id(n_props);

  for (unsigned int i = 0; i < n_props; i++)
  {
    std::string prop_name;
    char ch = 0;
    do {
      in.read(&ch, 1);
      if (ch != '\0')
        prop_name += ch;
    } while (ch != '\0');
    prop_names[i] = prop_name;
    prop_id[i] = stateful_prop_ids[prop_name];
  }

  for (unsigned int i = 0; i < n_elems; i++)
  {
    unsigned int elem_id = 0;
    in.read((char *) &elem_id, sizeof(elem_id));

    unsigned int blk_size = 0;
    in.read((char *) &blk_size, sizeof(blk_size));

    const Elem * elem = _mesh.elem(elem_id);

    if (elem && (elem->processor_id() == libMesh::processor_id()))
    {
      // read in the properties themselves
      for (unsigned int i = 0; i < n_props; i++)
      {
        unsigned int pid = prop_id[i];
        if (props[elem][0][pid] != NULL) props[elem][0][pid]->load(in);
        if (propsOld[elem][0][pid] != NULL) propsOld[elem][0][pid]->load(in);
        if (_material_props.hasOlderProperties())               // this should actually check if the value is stored in the file (we do not store it right now)
          if (propsOlder[elem][0][pid] != NULL) propsOlder[elem][0][pid]->load(in);
      }
    }
    else
      in.seekg(blk_size, std::ios_base::cur);
  }

  // load in the material props on sides
  for (unsigned int i = 0; i < n_elems; i++)
  {
    unsigned int elem_id = 0;
    in.read((char *) &elem_id, sizeof(elem_id));

    unsigned int n_sides = 0;
    in.read((char *) &n_sides, sizeof(n_sides));

    for (unsigned int s = 0; s < n_sides; s++)
    {
      unsigned int side = 0;
      in.read((char *) &side, sizeof(side));

      unsigned int blk_size = 0;
      in.read((char *) &blk_size, sizeof(blk_size));

      const Elem * elem = _mesh.elem(elem_id);
      if (elem && (elem->processor_id() == libMesh::processor_id()))
      {
        // read in the properties themselves
        for (unsigned int i = 0; i < n_props; i++)
        {
          unsigned int pid = prop_id[i];

          if (bnd_props[elem][side][pid] != NULL) bnd_props[elem][side][pid]->load(in);
          if (bnd_propsOld[elem][side][pid] != NULL) bnd_propsOld[elem][side][pid]->load(in);
          if (_material_props.hasOlderProperties())               // this should actually check if the value is stored in the file (we do not store it right now)
            if (bnd_propsOlder[elem][side][pid] != NULL) bnd_propsOlder[elem][side][pid]->load(in);
        }
      }
      else
        in.seekg(blk_size, std::ios_base::cur);
    }
  }

  in.close();
}
