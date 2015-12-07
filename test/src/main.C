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
  typedef unsigned char type;
};

template<>
unsigned int packed_size(const std::string *, std::vector<unsigned char>::const_iterator in)
{
  std::cout << "packed size: " << ((in[0] << 24) | (in[1] << 16) | (in[2] << 8) | (in[3] << 0)) << std::endl;


  // String is encoded as a 32-bit length followed by the content (unsigned char)
  return (in[0] << 24) | (in[1] << 16) | (in[2] << 8) | (in[3]);
}

template<>
unsigned int packed_size(const std::string * s, std::vector<unsigned char>::iterator in)
{
  return packed_size(s, std::vector<unsigned char>::const_iterator(in));
}


template<>
unsigned int packable_size(const std::string * s, const void *)
{
  // String is encoded as a 32-bit length followed by the content (unsigned char)
  return s->size() + 4;
}


template <>
void pack (const std::string * b, std::vector<unsigned char> & data, const void *)
{
  uint32_t size = 4 /* 32-bit int */ + b->size();

//  std::cout << "size: " << size << std::endl;
//
//  const unsigned char* beg = reinterpret_cast<const unsigned char*>(&size);
//  const unsigned char* end = beg + sizeof(size);
//  while(beg != end)
//    std::cout << std::bitset<CHAR_BIT>(*beg++) << ' ';
//  std::cout << '\n';


  data.push_back(size >> 24);
  data.push_back(size >> 16);
  data.push_back(size >> 8);
  data.push_back(size);

//  data.push_back(static_cast<unsigned char>(size & 0x000000FF));
//  data.reinterpret_cast<

  unsigned int packed_size = data.size();
  std::cout << "byte 0: " <<  static_cast<unsigned int>(data[packed_size-4]) << std::endl;
  std::cout << "byte 1: " <<  static_cast<unsigned int>(data[packed_size-3]) << std::endl;
  std::cout << "byte 2: " <<  static_cast<unsigned int>(data[packed_size-2]) << std::endl;
  std::cout << "byte 3: " <<  static_cast<unsigned int>(data[packed_size-1]) << std::endl;


  std::copy(b->begin(), b->end(), std::back_inserter(data));
}

template <>
void unpack(std::vector<unsigned char>::const_iterator in, std::string ** out, void *)
{
  uint32_t size = (in[0] << 24) | (in[1] << 16) | (in[2] << 8) | (in[3] << 0);

  std::cout << "Unpack: " << size << std::endl;

  std::ostringstream oss;
  for (unsigned int i = 4; i < size; ++i)
    oss << in[i];

  in += size;

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
    for (unsigned int i=0; i<260; ++i)
      foo.append("0123456789");
    break;

//    foo.assign("zero"); break;
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
