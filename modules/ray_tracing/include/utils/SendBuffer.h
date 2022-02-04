//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "Moose.h"
#include "SharedPool.h"

// libMesh Includes
#include "libmesh/parallel_object.h"

// Local includes
#include "ParallelStudyMethod.h"

// System Includes
#include <list>

template <typename Object, typename Context>
class SendBuffer : public ParallelObject
{
public:
  SendBuffer(const libMesh::Parallel::Communicator & comm,
             const Context * const context,
             const processor_id_type pid,
             const ParallelStudyMethod & method,
             const unsigned int min_buffer_size,
             const unsigned int max_buffer_size,
             const Real buffer_growth_multiplier,
             const Real buffer_shrink_multiplier,
             const Parallel::MessageTag object_buffer_tag);

  /**
   * Destructor: ensures that all send requests have completed
   */
  ~SendBuffer() override;

  /**
   * Get the number of objects sent from this buffer
   */
  unsigned long int objectsSent() const { return _objects_sent; }
  /**
   * Get the number of buffers sent from this buffer
   */
  unsigned long int buffersSent() const { return _buffers_sent; }
  /**
   * Get the number of buffers created in the buffer pool
   */
  unsigned long int bufferPoolCreated() const { return _buffer_pool.num_created(); }

  /**
   * Whether or not messages are currently being sent
   */
  bool currentlySending() const { return _requests.size(); }
  /**
   * Whether or not objects are currently waiting to be sent
   */
  bool currentlyBuffered() const { return _buffer.size(); }

  /**
   * Move an object to the buffer.  May cause the buffer to be communicated.
   *
   * This DOES call std::move on the object
   */
  void moveObject(std::shared_ptr<Object> & object);

  /**
   * Forces a Send for all currently buffered objects
   */
  void forceSend(const bool shrink_current_buffer_size = true);

  /**
   * Wait for all requests to finish
   */
  void waitAll();

  /**
   * Clear all existing data
   */
  void clear();

  /**
   * Checks to see if any Requests can be finished
   */
  void cleanupRequests();

private:
  /// The context
  const Context * const _context;
  /// The processor ID this buffer will send to
  const processor_id_type _pid;

  const ParallelStudyMethod & _method;

  /// Minimum size of the buffer (in objects)
  const unsigned int _min_buffer_size;
  /// Maximum size of the buffer (in objects)
  const unsigned int _max_buffer_size;

  /// Multiplier for the buffer size for growing the buffer
  const Real _buffer_growth_multiplier;
  /// Multiplier for the buffer size for shrinking the buffer
  const Real _buffer_shrink_multiplier;

  /// MessageTag for sending objects
  const Parallel::MessageTag _object_buffer_tag;

  /// Current size of the buffer (in objects)
  unsigned int _current_buffer_size;
  /// Running buffer size
  Real _current_buffer_size_real;

  /// The buffer
  std::vector<std::shared_ptr<Object>> _buffer;
  /// The size of the objects in the buffer in bytes
  std::size_t _buffer_size_bytes;

  /// List of Requests
  std::list<std::shared_ptr<Parallel::Request>> _requests;

  /// Shared pool of buffers
  MooseUtils::SharedPool<
      std::vector<typename Parallel::Packing<std::shared_ptr<Object>>::buffer_type>>
      _buffer_pool;

  /// Counter for objects sent
  unsigned long int _objects_sent;
  /// Counter for buffers sent
  unsigned long int _buffers_sent;
};

template <typename Object, typename Context>
SendBuffer<Object, Context>::SendBuffer(const libMesh::Parallel::Communicator & comm,
                                        const Context * const context,
                                        const processor_id_type pid,
                                        const ParallelStudyMethod & method,
                                        const unsigned int min_buffer_size,
                                        const unsigned int max_buffer_size,
                                        const Real buffer_growth_multiplier,
                                        const Real buffer_shrink_multiplier,
                                        const Parallel::MessageTag object_buffer_tag)
  : ParallelObject(comm),
    _context(context),
    _pid(pid),
    _method(method),
    _min_buffer_size(min_buffer_size),
    _max_buffer_size(max_buffer_size),
    _buffer_growth_multiplier(buffer_growth_multiplier),
    _buffer_shrink_multiplier(buffer_shrink_multiplier),
    _object_buffer_tag(object_buffer_tag),
    _current_buffer_size(_min_buffer_size),
    _current_buffer_size_real(_min_buffer_size),
    _buffer_size_bytes(0),
    _objects_sent(0),
    _buffers_sent(0)
{
  _buffer.reserve(max_buffer_size);
}

template <typename Object, typename Context>
SendBuffer<Object, Context>::~SendBuffer()
{
  waitAll();
  cleanupRequests();
}

template <typename Object, typename Context>
void
SendBuffer<Object, Context>::moveObject(std::shared_ptr<Object> & object)
{
  _buffer_size_bytes +=
      Parallel::Packing<std::shared_ptr<Object>>::packable_size(object, _context) *
      sizeof(typename Parallel::Packing<std::shared_ptr<Object>>::buffer_type);
  _buffer.emplace_back(std::move(object));

  // Force a send with SMART if we find it appropriate
  if ((_method == ParallelStudyMethod::SMART &&
       (_buffer.size() >= _current_buffer_size || _buffer.size() == _max_buffer_size)) ||
      (_buffer_size_bytes > 1048576)) // 1 MB
  {
    _current_buffer_size_real =
        std::min(_buffer_growth_multiplier * _current_buffer_size_real, (Real)_max_buffer_size);

    if (_current_buffer_size != (unsigned int)_current_buffer_size_real)
      _current_buffer_size = _current_buffer_size_real;

    forceSend(false);
  }
}

template <typename Object, typename Context>
void
SendBuffer<Object, Context>::forceSend(const bool shrink_current_buffer_size)
{
  if (!_buffer.empty())
  {
    auto req = std::make_shared<Parallel::Request>();

    _requests.push_back(req);

    _buffer_size_bytes = 0;
    _objects_sent += _buffer.size();
    _buffers_sent++;

    std::shared_ptr<std::vector<typename Parallel::Packing<std::shared_ptr<Object>>::buffer_type>>
        buffer = _buffer_pool.acquire();

    comm().nonblocking_send_packed_range(
        _pid, _context, _buffer.begin(), _buffer.end(), *req, buffer, _object_buffer_tag);

    _buffer.clear();
    _buffer.reserve(_max_buffer_size);

    if (_method == ParallelStudyMethod::SMART && shrink_current_buffer_size)
    {
      _current_buffer_size_real =
          std::max((Real)_min_buffer_size, _current_buffer_size_real * _buffer_shrink_multiplier);

      if (_current_buffer_size != (unsigned int)_current_buffer_size_real)
        _current_buffer_size = _current_buffer_size_real;
    }
  }

  cleanupRequests();
}

template <typename Object, typename Context>
void
SendBuffer<Object, Context>::clear()
{
  _buffer.clear();
  _requests.clear();

  _objects_sent = 0;
  _buffers_sent = 0;
}

template <typename Object, typename Context>
void
SendBuffer<Object, Context>::cleanupRequests()
{
  _requests.remove_if(
      [](std::shared_ptr<Parallel::Request> & req)
      {
        if (req->test())
        {
          req->wait(); // MUST call wait() to do post_wait_work
          return true;
        }
        else
          return false;
      });
}

template <typename Object, typename Context>
void
SendBuffer<Object, Context>::waitAll()
{
  std::for_each(_requests.begin(), _requests.end(), [](auto & request) { request->wait(); });
}
