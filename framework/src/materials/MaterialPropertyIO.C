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
#include <cstring>


const unsigned int MaterialPropertyIO::file_version = 1;

struct MSMPHeader
{
  char _id[4];                  // 4 letter ID
  unsigned int _file_version;   // file version
};


MaterialPropertyIO::MaterialPropertyIO(MaterialPropertyStorage & material_props, MaterialPropertyStorage & bnd_material_props) :
    _material_props(material_props),
    _bnd_material_props(bnd_material_props)
{
}

MaterialPropertyIO::~MaterialPropertyIO()
{
}

void
MaterialPropertyIO::write(const std::string & file_name)
{
  std::ofstream out(file_name.c_str(), std::ios::out | std::ios::binary);

  // header
  MSMPHeader head;
  std::memcpy(head._id, "MSMP", 4);
  head._file_version = file_version;
  out.write((const char *) &head, sizeof(head));

  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & props = _material_props.props();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & propsOld = _material_props.propsOld();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & propsOlder = _material_props.propsOlder();

  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_props = _bnd_material_props.props();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_propsOld = _bnd_material_props.propsOld();
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & bnd_propsOlder = _bnd_material_props.propsOlder();

  // number of blocks
  // TODO: go over elements and figure out the groups of elements we are going to write in a file
  unsigned int n_blocks = 1;             // just one for right now
  out.write((const char *) &n_blocks, sizeof(n_blocks));

  // number of quadrature points
  // we go grab element 0, side 0 (it's volumetric mat. properties) and we will use the first stateful property to get the number of QPs (all should be sized the same)
  unsigned int n_qps = 0;
  for (MaterialProperties::iterator it = props[0][0].begin(); it != props[0][0].end(); ++it)        // we expect to have element 0 always (lame)
    if (*it != NULL)
      n_qps = (*it)->size();
  out.write((const char *) &n_qps, sizeof(n_qps));

  // save the number of elements in this block (since we do only 1 block right now, we store everything)
  unsigned int n_elems = props.size();
  out.write((const char *) &n_elems, sizeof(n_elems));

  // properties
  std::set<unsigned int> & stateful_props = _material_props.statefulProps();
  std::vector<unsigned int> prop_ids;
  prop_ids.insert(prop_ids.end(), stateful_props.begin(), stateful_props.end());
  std::sort(prop_ids.begin(), prop_ids.end());

  unsigned int n_props = prop_ids.size();        // number of properties in this block
  out.write((const char *) &n_props, sizeof(n_props));
  // property names
  for (unsigned int i = 0; i < n_props; i++)
  {
    unsigned int pid = prop_ids[i];
    std::string prop_name = _material_props.statefulPropNames()[pid];
    out.write(prop_name.c_str(), prop_name.length() + 1);                 // do not forget the trailing zero ;-)
  }

  std::cout<<"props size: "<<props.size()<<std::endl;

  // save current material properties
  for (HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> >::iterator props_it=props.begin(); props_it != props.end(); ++props_it)
  {
    const Elem * elem = props_it->first;

    if(elem)
    {
      unsigned int elem_id = elem->id();
      out.write((const char *) &elem_id, sizeof(elem_id));

      // write out the properties themselves
      for (unsigned int i = 0; i < n_props; i++)
      {
        unsigned int pid = prop_ids[i];
        props[elem][0][pid]->store(out);
        propsOld[elem][0][pid]->store(out);
        if (_material_props.hasOlderProperties())
          propsOlder[elem][0][pid]->store(out);
      }
    }
  }

  // save the material props on sides
  unsigned int n_sides = bnd_props[0].size();
  out.write((const char *) &n_sides, sizeof(n_sides));

  // save current material properties
  for (HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> >::iterator props_it=props.begin(); props_it != props.end(); ++props_it)
  {
    const Elem * elem = props_it->first;

    if(elem)
    {

      unsigned int elem_id = elem->id();
      out.write((const char *) &elem_id, sizeof(elem_id));

      for (unsigned int s = 0; s < n_sides; s++)
      {
        // write out the properties themselves
        for (unsigned int i = 0; i < n_props; i++)
        {
          unsigned int pid = prop_ids[i];
          bnd_props[elem][s][pid]->store(out);
          bnd_propsOld[elem][s][pid]->store(out);
          if (_material_props.hasOlderProperties())
            bnd_propsOlder[elem][s][pid]->store(out);
        }
      }
    }
  }

  // TODO: end of the loop over blocks

  out.close();
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

  // number of blocks
  unsigned int n_blocks = 0;
  in.read((char *) &n_blocks, sizeof(n_blocks));

  // loop over block
  for (unsigned int blk_id = 0; blk_id < n_blocks; blk_id++)
  {
    // number of quadrature points
    unsigned int n_qps = 0;
    in.read((char *) &n_qps, sizeof(n_qps));
    // number of elements
    unsigned int n_elems = props.size();
    in.read((char *) &n_elems, sizeof(n_elems));
    // number of properties in this block
    unsigned int n_props = 0;
    in.read((char *) &n_props, sizeof(n_props));
    // property names
    std::vector<std::string> prop_names;

    for (unsigned int i = 0; i < n_props; i++)
    {
      std::string prop_name;
      char ch = 0;
      do {
        in.read(&ch, 1);
        if (ch != '\0')
          prop_name += ch;
      } while (ch != '\0');
      prop_names.push_back(prop_name);
    }

    for (HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> >::iterator props_it=props.begin(); props_it != props.end(); ++props_it)
    {
      const Elem * elem = props_it->first;

      if(elem)
      {
        unsigned int elem_id = elem->id();
        in.read((char *) &elem_id, sizeof(elem_id));

        // read in the properties themselves
        for (unsigned int i = 0; i < n_props; i++)
        {
          unsigned int pid = stateful_prop_ids[prop_names[i]];

          props[elem][0][pid]->load(in);
          propsOld[elem][0][pid]->load(in);
          if (_material_props.hasOlderProperties())               // this should actually check if the value is stored in the file (we do not store it right now)
            propsOlder[elem][0][pid]->load(in);
        }
      }
    }

    // load in the material props on sides
    unsigned int n_sides = 0;
    in.read((char *) &n_sides, sizeof(n_sides));

    for (HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> >::iterator props_it=props.begin(); props_it != props.end(); ++props_it)
    {
      const Elem * elem = props_it->first;

      if(elem)
      {
        unsigned int elem_id = elem->id();
        in.read((char *) &elem_id, sizeof(elem_id));

        for (unsigned int s = 0; s < n_sides; s++)
        {
          // read in the properties themselves
          for (unsigned int i = 0; i < n_props; i++)
          {
            unsigned int pid = stateful_prop_ids[prop_names[i]];

            bnd_props[elem][s][pid]->load(in);
            bnd_propsOld[elem][s][pid]->load(in);
            if (_material_props.hasOlderProperties())               // this should actually check if the value is stored in the file (we do not store it right now)
              bnd_propsOlder[elem][s][pid]->load(in);
          }
        }
      }
    }
  }

  in.close();
}
