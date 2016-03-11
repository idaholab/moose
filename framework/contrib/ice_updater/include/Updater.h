/*******************************************************************************
 * Copyright (c) 2015, UT-Battelle, LLC.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*******************************************************************************/
#ifndef UPDATER_H
#define UPDATER_H

#include <string>
#include <queue>
#include <map>

#include "Post.h"
#include "PropertyType.h"
#include "ErrorLogger.h"
#include "MooseTypes.h"
#include "MooseError.h"
#include "INetworkingTool.h"

#ifdef LIBMESH_HAVE_CXX11

#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>

class UpdaterThread;

/**
 * A PostPtr is a shared pointer object referring to a Post instance.
 */
typedef MooseSharedPointer<Post> PostPtr;

/**
 * An ErrorLoggerPtr is a shared pointer object referring to an ErrorLogger instance.
 */
typedef MooseSharedPointer<ErrorLogger> ErrorLoggerPtr;

/**
 * A PropertyMap is a map template associating with PropertyType keys and string values.
 */
typedef std::map<PropertyType, std::string> PropertyMap;

/**
 * The Updater allows ICE users to transmit information concerning the ongoing simulation in near real-time.
 * The information will be transmitted to a URL with the POST method using the HTTP or HTTPS protocol and cURL.
 * After instantiation of Updater, the user program can call start() and stop() to begin and end the transmission
 * thread. After starting Updater, the user program can call postMessage(), postFileCreated(), postFileModified(),
 * postFileDeleted(), updateProgress() and updateConvergence() to append Post objects to a queue. At each cycle of the
 * thread, all posts in the queue are transmitted to the provided URL in the JSON format. These messages are intended
 * to be consumed by a running instance of ICECore and broadcasted to any number of ICEClient objects or other
 * client program such as simple web page.
 */
class Updater
{
private:

  /**
   * A smart pointer to an ErrorLogger instance.
   */
  ErrorLoggerPtr errorLoggerPtr;

  /**
   * A flag to indicate whether the propertyMap submitted
   * through the Constructor has been successfully validated.
   */
  bool goodPropertyMap;

  /**
   * A flag indicating whether or not cURL calls will skip peer
   * certificate verification for HTTPS urls. This flag should
   * only be set to true for testing purposes.
   */
  bool ignoreSslPeerVerification;

  /**
   * A standard template library map container keyed on
   * PropertyType containing values required by Updater.
   */
  PropertyMap propertyMap;

  /**
   * A flag indicating whether start() has been called and thread exists.
   */
  bool threadCreated;

  /**
   * True by default.  When true, set (CURLOPT_NOPROXY, "*") when
   * calling libcurl APIs.  Call the setNoProxyFlag() setter to change
   * the value.
   */
  bool noProxyFlag;

  /**
   * Reference to the UpdaterThread that serves as the TBB Task to launch
   */
  UpdaterThread * updaterThread;

  /**
   * Adds a Post object to the posts queue.
   *
   * @param type A PostType literal.
   * @param message The string message assigned to the Post.
   */
  void addPostToQueue(PostType type, std::string message);

  /**
   * Returns the contents of the updater.properties file as a string.
   *
   * @return The string contents of the updater.properties file if it exists in the current directory.
   */
  std::string getPropertyFileContents();

  /**
   * Creates and returns a PropertyMap type from a string formatted as a Java properties file.
   *
   * @param propertyString A updater.properties formatted string.
   * @return A PropertyMap comprised of the name/value pairs in the propertyString.
   */
  PropertyMap getPropertyMap(std::string propertyString);

  /**
   * Validates the propertyMap object and writes any errors to the error logger.
   *
   * @return True if the property map contains good values.
   */
  bool validatePropertyMap();

  /**
   * Initializes ICEupdater by creating the property map and setting threadCreated to false.
   * Called only from the constructors.
   *
   * @param propertyString A string formatted as a Java properties file containing name/value pairs.
   */
  void initialize(std::string propertyString);

public:

  /**
   * This Constructor will search for properties file in the current directory.
   * This file must contain the url, item id and client key used for transmission.
   */
  Updater();

  /**
   * This Constructor requires an input stream in the form of the
   * updater.properties file format.
   *
   * @param stream An input stream in the form of the updater.properties file format.
   */
  Updater(std::istream &stream);

  /**
   * The Destructor.  Empty.
   */
  ~Updater();

  /**
   * Adds a Post object to the posts queue indicating the creation of the file located at path.
   *
   * @param path The path of the file.
   */
  void postFileCreated(std::string path);

  /**
   * Adds a Post object to the posts queue indicating the deletion of the file located at path.
   *
   * @param path The path of the file.
   */
  void postFileDeleted(std::string path);

  /**
   * Adds a Post to the posts queue indicating the modification of the file located at path.
   *
   * @param path The path of the file.
   */
  void postFileModified(std::string path);

  /**
   * Adds a plain text message Post to the posts queue.
   *
   * @param message A plain text message.
   */
  void postMessage(std::string message);

  /**
   * Sets the ignoreSslPeerVerification flag. If ignoreSslPeerVerification flag is
   * set to true then cURL will skip peer certificate verification for HTTPS urls.
   * This flag should only be set to true for testing purposes.
   *
   * @param ignoreSslPeerVerification The value for the ignoreSslPeerVerification flag.
   */
  void setIgnoreSslPeerVerification(bool ignoreSslPeerVerification);

  /**
   * Creates the thread object initialized with a reference to the threadProcess() operation.
   * Returns whether the thread was created successfully.
   *
   * @return true if the thread was started successfully.
   */
  bool start();

  /**
   * Stops the thread by calling the interrupt operation on thread. Returns whether
   * the thread was interrupted successfully.
   *
   * @return true if the thread was stopped successfully.
   */
  bool stop();

  /**
   * Adds a Post object to posts queue containing the convergence status of the user simulation.
   * Status must be >=1 and <=100. If the value of status is less than 0 then the value will be set to 0.
   * If the value of status is greater than 100 then the value will be set to 100.
   *
   * @param status The convergence value to post.
   */
  void updateConvergence(int status);

  /**
   * Adds a Post object to posts queue containing the progress of the user simulation.
   * Status must be >=1 and <=100. If the value of status is less than 0 then the value will be set to 0.
   * If the value of status is greater than 100 then the value will be set to 100.
   *
   * @param status The progress value to post.
   */
  void updateProgress(int status);

  /**
   * Sets the noProxyFlag's value to 'val'.
   *
   * @param val The new value for the noProxyFlag.
   */
  void setNoProxyFlag(bool val) { noProxyFlag = val; }
};

/**
 * This queue class serves as a thread-safe wrapper around
 * the std queue.
 */
template <typename T>
class Queue
{
 public:

  T pop()
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    auto item = queue_.front();
    queue_.pop();
    return item;
  }

  bool pop(T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    item = queue_.front();
    queue_.pop();
    return item != nullptr;
  }

  void push(const T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(item);
    mlock.unlock();
    cond_.notify_one();
  }

  void push(T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(std::move(item));
    mlock.unlock();
    cond_.notify_one();
  }

  bool empty()
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    bool empty = queue_.empty();
    mlock.unlock();
    cond_.notify_one();
    return empty;
  }

 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

/**
 * A PostPtrQueue is a queue template typed for PostPtr objects.
 */
typedef Queue<PostPtr> PostPtrQueue;

/**
 * UpdaterThread is a subclass of TBB Task and TBB concurrent queue
 * that executes the event loop for posting updates to the ICE Core.
 */
class UpdaterThread: public PostPtrQueue
{
private:

  /**
   * A standard template library map container keyed on
   * PropertyType containing values required by Updater.
   */
  PropertyMap propertyMap;

  /**
   * A smart pointer to an ErrorLogger instance.
   */
  ErrorLoggerPtr errorLoggerPtr;

  /**
   * A flag indicating whether or not cURL calls will skip peer
   * certificate verification for HTTPS urls. This flag should
   * only be set to true for testing purposes.
   */
  bool ignoreSslPeerVerification;

  /**
   * Flag to be passed down to the libcurlUtils object to control
   * whether proxies are ignored while making libcurl API calls.
   */
  bool noProxyFlag;

  /**
   * Stop flag.
   */
//  tbb::atomic<bool> stop;
   std::atomic<bool> stop;

  /**
   *
   */
  MooseSharedPointer<INetworkingTool> networkingTool;

  MooseSharedPointer<std::thread> processThread;
public:

  /**
   * The constructor
   */
  UpdaterThread(PropertyMap map,
                ErrorLoggerPtr error,
                bool ssl,
                bool noproxy) :
      propertyMap(map),
      errorLoggerPtr(error),
      ignoreSslPeerVerification(ssl),
      noProxyFlag(noproxy)
  {
    stop.store(false);
  }

  /**
   * The method passed to the thread constructor which transmits all Posts in the
   * posts queue to url with cURL in JSON format.
   * The thread is then put to sleep for 1000 milliseconds.
   */
  void execute()
  {
    processThread = MooseSharedPointer<std::thread>(new std::thread(&UpdaterThread::runThreaded, this));
  }

  /**
   * This method is passed to the std thread to start the
   * post loop.
   */
  void runThreaded();

  /**
   * Stop the thread
   */
  void stopThread() { stop.store(true); }
};



#else // !LIBMESH_HAVE_CXX11



/**
 * If we aren't using cxx11, build a stub Updater class that does nothing
 * but throw errors if used.
 */
class Updater
{
public:
  /**
   * The constructors all throw errors.
   */
  Updater() { mooseError("Updater requires --enable-cxx11 parameter to update_and_build_libmesh.sh."); }
  Updater(std::istream &) { mooseError("Updater requires --enable-cxx11 parameter to update_and_build_libmesh.sh."); }

  /**
   * The following functions do nothing, and will never be called.
   */
  void postFileCreated(std::string) {}
  void postFileDeleted(std::string) {}
  void postFileModified(std::string) {}
  void postMessage(std::string) {}
  void setIgnoreSslPeerVerification(bool) {}
  bool start() { return false; }
  bool stop() { return false; }
  void updateConvergence(int) {}
  void updateProgress(int) {}
};

#endif // LIBMESH_HAVE_CXX11

#endif
