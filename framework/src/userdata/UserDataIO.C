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

#include "UserDataIO.h"
#include "UserObject.h"
#include <vector>

const unsigned int UserDataIO::file_version = 1;

struct MUDSHeader               // MOOSE User Data Storage
{
  char _id[4];                  // 4 letter ID
  unsigned int _file_version;   // file version
};


UserDataIO::UserDataIO(UserObjectWarehouse & ud_wh) :
    _userobject_wh(ud_wh)
{
}

UserDataIO::~UserDataIO()
{
}

void
UserDataIO::write(const std::string & file_name)
{
  std::ofstream out(file_name.c_str(), std::ios::out | std::ios::binary);

  // header
  MUDSHeader head;
  memcpy(head._id, "MUDS", 4);
  head._file_version = file_version;
  out.write((const char *) & head, sizeof(head));

  // save the number of user objects
  unsigned int n_objs = _userobject_wh.size();
  out.write((const char *) & n_objs, sizeof(n_objs));

  // loop over objects and save them
  std::vector<UserObject *> & uds = _userobject_wh.getObjects();
  for (std::vector<UserObject *>::iterator it = uds.begin(); it != uds.end(); ++it)
  {
    UserObject * uo = *it;

    // write out the name
    std::string name = uo->name();
    out.write(name.c_str(), name.length() + 1);                 // do not forget the trailing zero ;-)

    long pos = out.tellp();
    unsigned int size = 0;
    // write out the size
    out.write((const char *) & size, sizeof(size));
    // write out the data
    uo->store(out);
    long end = out.tellp();
    // go back and overwrite the size
    out.seekp(pos);
    size = end - pos - sizeof(size);
    out.write((const char *) & size, sizeof(size));
    // go at the and of the stream and continue
    out.seekp(end);
  }
}

void
UserDataIO::read(const std::string & file_name)
{
  std::ifstream in(file_name.c_str(), std::ios::in | std::ios::binary);

  // header
  MUDSHeader head;
  in.read((char *) & head, sizeof(head));

  // check the header
  if (!(head._id[0] == 'M' && head._id[1] == 'U' && head._id[2] == 'D' && head._id[3] == 'S'))
    mooseError("Corrupted user data restart file");
  // check the file version
  if (head._file_version > file_version)
    mooseError("Trying to restart from a newer file version - you need to update MOOSE");

  // load the number of user objects
  unsigned int n_objs = 0;
  in.read((char *) & n_objs, sizeof(n_objs));

  // loop over objects and save them
  for (unsigned int i = 0; i < n_objs; i++)
  {
    // FIXME: turn this into a function
    std::string name;
    char ch = 0;
    do {
      in.read(&ch, 1);
      if (ch != '\0')
        name += ch;
    } while (ch != '\0');

    // read the size
    unsigned int size = 0;
    // write out the size
    in.read((char *) & size, sizeof(size));

    if (_userobject_wh.hasUserObject(name))
    {
      // read the object
      UserObject * uo = _userobject_wh.getUserObjectByName(name);
      uo->load(in);
    }
    else
    {
      // we do not know the object, so we skip it
      in.seekg(size, std::ios::cur);
    }
  }
}
