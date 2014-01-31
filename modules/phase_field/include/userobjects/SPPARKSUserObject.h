#ifndef SPPARKSUSEROBJECT_H
#define SPPARKSUSEROBJECT_H

#include "GeneralUserObject.h"

#include "SPPARKS/src/library.h"

class SPPARKSUserObject : public GeneralUserObject
{
public:

  SPPARKSUserObject(const std::string & name, InputParameters parameters);

  virtual ~SPPARKSUserObject();

  virtual void initialSetup();

  virtual void residualSetup() {}

  virtual void timestepSetup() {}

  virtual void initialize();
  virtual void execute();
  virtual void finalize() {}

  int  getIntValue(unsigned elk_node_id, unsigned index) const;
  Real getDoubleValue(unsigned elk_node_id, unsigned index) const;

protected:

  char * runSPPARKSCommand( const std::string & cmd )
  {
    std::vector<char> strng(cmd.c_str(), cmd.c_str()+cmd.size()+1u);
    return spparks_command(_spparks, &strng[0]);
  }

  Real getSPPARKSTime( Real dt )
  {
    return dt;
  }

  template
  <typename T>
  Real getValue(const std::map<unsigned, std::map<unsigned, T> > & data, unsigned elk_node_id, unsigned index) const
  {
    // Extract the data
    const typename std::map<unsigned, std::map<unsigned, T> >::const_iterator it = data.find(index);
    if (it == data.end())
    {
      std::stringstream err;
      err << "SPPARKSUserObject error: unknown index ";
      err << index << std::endl;
      mooseError(err.str());
    }
    const typename std::map<unsigned, T>::const_iterator it2 = it->second.find(elk_node_id);
    if (it2 == it->second.end())
    {
      std::stringstream err;
      err << "SPPARKSUserObject error: unknown elk node id ";
      err << elk_node_id << std::endl;
      mooseWarning(err.str());
      return 0;
    }
    return it2->second;
  }


  template
  <typename T>
  void getSPPARKSDataPointer( T *& ptr, char * string, unsigned i )
  {
    std::stringstream name;
    name << string;
    name << i;
    std::string s = name.str();
    std::vector<char> strng(s.c_str(), s.c_str()+s.size()+1u);
    getSPPARKSPointer(ptr, &strng[0]);
  }

  template
  <typename T>
  void getSPPARKSPointer( T *& ptr, char * string ) const
  {
    void * p = spparks_extract(_spparks, string);
    if (!p)
    {
      mooseError("SPPARKS returned NULL pointer for " + std::string(string));
    }
    ptr = static_cast<T*>(p);
  }

  void * _spparks;

  const std::string & _file;
  const bool _spparks_only;
  const std::vector<unsigned> _ivar;
  const std::vector<unsigned> _dvar;
  const Real _xmin;
  const Real _ymin;
  const Real _zmin;
  const Real _xmax;
  const Real _ymax;
  const Real _zmax;

  int _dim;

  class ELKID;
  struct SPPARKSID
  {
    SPPARKSID( unsigned indx, const Point & p ) :
      index(indx),
      coor(p)
    {}
    SPPARKSID( ELKID eid ) :
      index(eid.id),
      coor(eid.coor)
    {}
    unsigned index;
    Point coor;
    bool operator<(const SPPARKSID & rhs) const
    {
      return coor < rhs.coor;
    }
  };
  struct ELKID
  {
    ELKID( libMesh::dof_id_type ident, const Point & p ) :
      id(ident),
      coor(p)
    {}
    ELKID( SPPARKSID sid ):
      id(sid.index),
      coor(sid.coor)
    {}
    libMesh::dof_id_type id;
    Point coor;
    bool operator<(const ELKID & rhs) const
    {
      return coor < rhs.coor;
    }
  };

  void getSPPARKSData();

  template
  <typename T>
  void getSPPARKSData( std::map<unsigned, T> & storage, char * string, unsigned index )
  {
    T * data;
    getSPPARKSDataPointer( data, string, index );

    for (std::multimap<ELKID, SPPARKSID>::const_iterator i = _elk_to_spparks.begin(); i != _elk_to_spparks.end(); ++i)
    {
      // Index into storage is ELK node id.
      storage[i->first.id] = data[i->second.index];
    }

    if ( libMesh::n_processors() == 1 )
    {
      return;
    }

    sendRecvSPPARKSData( data, storage );
  }

  template
  <typename T>
  void sendRecvSPPARKSData( const T * const data, std::map<unsigned, T> & storage )
  {

    Parallel::MessageTag comm_tag(101);

    const unsigned num_recvs = _sending_proc_to_elk_id.size();
    std::vector<Parallel::Request> recv_request(num_recvs);

    std::map<unsigned, std::vector<T> > data_to_me;
    unsigned offset = 0;
    for (std::map<unsigned, std::vector<libMesh::dof_id_type> >::const_iterator i = _sending_proc_to_elk_id.begin();
         i != _sending_proc_to_elk_id.end(); ++i)
    {
      data_to_me[i->first].resize( i->second.size() );
      Parallel::receive(i->first, data_to_me[i->first], recv_request[offset], comm_tag);
      ++offset;
    }


    std::map<unsigned, std::vector<T> > data_from_me;
    for (std::map<SPPARKSID, std::vector<unsigned> >::const_iterator i = _spparks_to_proc.begin(); i != _spparks_to_proc.end(); ++i)
    {
      for (unsigned j = 0; j < i->second.size(); ++j)
      {
        data_from_me[i->second[j]].push_back( data[i->first.index] );
      }
    }

    offset = 0;
    for (typename std::map<unsigned, std::vector<T> >::const_iterator i = data_from_me.begin(); i != data_from_me.end(); ++i)
    {
      // Parallel::send(i->first, i->second, send_request[offset], comm_tag);
      Parallel::send(i->first, data_from_me[i->first], comm_tag);
      ++offset;
    }

    Parallel::wait(recv_request);

    // Move data into storage
    for (std::map<unsigned, std::vector<libMesh::dof_id_type> >::const_iterator i = _sending_proc_to_elk_id.begin();
         i != _sending_proc_to_elk_id.end(); ++i)
    {
      const std::vector<libMesh::dof_id_type> & id = i->second;
      const std::vector<T> & v = data_to_me[i->first];
      for (unsigned j = 0; j < v.size(); ++j)
      {
        storage[id[j]] = v[j];
      }
    }
  }


  // Communication maps
  std::map<SPPARKSID, std::vector<unsigned> > _spparks_to_proc;
  std::map<unsigned, std::vector<libMesh::dof_id_type> > _sending_proc_to_elk_id;

  unsigned _num_local_elk_nodes;

  std::map<SPPARKSID, ELKID> _spparks_to_elk;
  std::multimap<ELKID, SPPARKSID> _elk_to_spparks;

  // Maps from ELK node id to data
  std::map<unsigned, std::map<unsigned, int> > _int_data_for_elk;
  std::map<unsigned, std::map<unsigned, double> > _double_data_for_elk;


  Real _last_time;

};

template<>
InputParameters validParams<SPPARKSUserObject>();

#endif
