#include "MooseInit.h"
#include "Moose.h"
#include "MooseTypes.h"
#include "libmesh/parallel.h"
#include "libmesh/null_output_iterator.h"

#include <vector>
#include <list>
#include <iostream>
#include <iterator>
#include <algorithm>

// Create a performance log
PerfLog Moose::perf_log("Moose Test");


template <typename T>
struct string_list_inserter
  : std::iterator<std::output_iterator_tag, T>
{
  explicit string_list_inserter(std::list<T *> & list) : _list(list) 
    {
    }
  
  template <typename T2>
  void operator=(const T2 & value)
    {
      _list.push_back(value);
    }

  string_list_inserter& operator++() {
    return *this;
  }

  string_list_inserter operator++(int) {
    return string_list_inserter(*this);
  }

  // We don't return a reference-to-T here because we don't want to
  // construct one or have any of its methods called.
  string_list_inserter& operator*() { return *this; }

private:
  std::list<T *> & _list;
};


namespace libMesh {
namespace Parallel {

// BufferType<> specializations to return a buffer datatype
// to handle communication of Elems
template <>
struct BufferType<const std::string *> {
  typedef largest_id_type type;
};

template<>
unsigned int packed_size(const std::string *, std::vector<largest_id_type>::const_iterator in)
{
  // String is encoded as a length followed by the content (as long long ints)
  return in[0] + 1;;
}

template<>
unsigned int packed_size(const std::string * s, std::vector<largest_id_type>::iterator in)
{
  return packed_size(s, std::vector<largest_id_type>::const_iterator(in));
}


template<>
unsigned int packable_size(const std::string * s, const void *)
{
  // String is encoded as a length followed by the content (as long long ints)
  return s->size() + 1;
}
 
 
template <>
void pack (const std::string * b, std::vector<largest_id_type> & data, const void *)
{
  data.push_back(b->size());
  std::copy(b->begin(), b->end(), std::back_inserter(data));
}
 
template <>
void unpack(std::vector<largest_id_type>::const_iterator in, std::string ** out, void *)
{
  largest_id_type size = in[0];

  std::ostringstream oss;
  for (unsigned int i = 0; i < size; ++i)
    oss << static_cast<char>(in[i+1]);

  in += size + 1;
  
  (*out) = new std::string(oss.str());
}

}
}
  


int main(int argc, char *argv[])
{ 
  // Initialize MPI, solvers and MOOSE
  MooseInit init(argc, argv);

  MooseSharedPointer<Parallel::Communicator> comm(new Parallel::Communicator(MPI_COMM_WORLD));

  std::string foo;
  switch (comm->rank())
  {
  case 0:
    foo.assign("zero"); break;
  case 1:
    foo.assign("one"); break;
  case 2:
    foo.assign("two"); break;
  case 3:
    foo.assign("three"); break;
  case 4:
    foo.assign("four"); break;
  case 5:
    foo.assign("five"); break;
  case 6:
    foo.assign("six"); break;
  default:
    foo.assign("some bigger number"); break;
  }
  
  std::vector<std::string *> send(1);
  send[0] = &foo;

  std::list<std::string *> recv;
  
  comm->allgather_packed_range((void *)(NULL), send.begin(), send.end(),
                               string_list_inserter<std::string>(recv));

  for (std::list<std::string *>::iterator it = recv.begin(); it != recv.end(); ++it)
    std::cout << **it << '\n';
 
  // Vitally important!
  for (std::list<std::string *>::iterator it = recv.begin(); it != recv.end(); ++it)
    delete *it;
  
  return 0;
}
