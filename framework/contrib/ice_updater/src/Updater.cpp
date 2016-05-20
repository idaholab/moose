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
#include <fstream>
#include <stdlib.h>
#include <sstream>

#include "Updater.h"

#if defined(LIBMESH_HAVE_CXX11_THREAD) && defined(LIBMESH_HAVE_CXX11_CONDITION_VARIABLE)
#include "NetworkingToolFactory.h"

std::vector<std::string> &split(const std::string &s, char delim,
                                std::vector<std::string> &elems)
{
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim))
    elems.push_back(item);
  return elems;
}

std::vector<std::string> split(const std::string &s, char delim)
{
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}

/**
 * The Constructor. Here, the Constructor will search for properties file in the current directory.
 * This file must contain the url, item id and client key used for transmission.
 */
Updater::Updater() :
    noProxyFlag(true)
{
  // Get the contents of the "updater.properties" file
  std::string contents = getPropertyFileContents();

  // Initialize the updater
  initialize(contents);
}

/**
 * The Constructor. Here, the Constructor requires an input stream in the form of the
 * updater.properties file format.
 *
 * @param stream An input stream in the form of the updater.properties file format.
 */
Updater::Updater(std::istream &stream) :
    noProxyFlag(true)
{
  // Create a string buffer
  std::stringstream buffer;

  // Read contents of the input stream into string buffer.
  buffer << stream.rdbuf();

  // Create a string to hold contents of buffer
  std::string contents;

  // Convert buffer into a string and assign to contents.
  contents = buffer.str();

  // Initialize the updater
  initialize(contents);
}

/**
 * The Destructor.
 */
Updater::~Updater()
{
}

/**
 * Adds a Post object to the posts queue indicating the creation of the file located at path.
 *
 * @param path The path of the file.
 */
void Updater::postFileCreated(std::string path)
{
  // Create a new FILE_CREATED post
  addPostToQueue(FILE_CREATED, path);
}

/**
 * Adds a Post object to the posts queue indicating the deletion of the file located at path.
 *
 * @param path The path of the file.
 */
void Updater::postFileDeleted(std::string path)
{
  // Create a new FILE_DELETED post
  addPostToQueue(FILE_DELETED, path);
}

/**
 * Adds a Post to the posts queue indicating the modification of the file located at path.
 *
 * @param path The path of the file.
 */
void Updater::postFileModified(std::string path)
{
  // Create a new FILE_MODIFIED post
  addPostToQueue(FILE_MODIFIED, path);
}

/**
 * Adds a plain text message Post to the posts queue.
 *
 * @param message A plain text message.
 */
void Updater::postMessage(std::string message)
{
  // Create a new MESSAGE_POSTED post
  addPostToQueue(MESSAGE_POSTED, message);
}

/**
 * Adds a Post object to posts queue containing the convergence status of the user simulation.
 * Status must be >=1 and <=100. If the value of status is less than 0 then the value will be set to 0.
 * If the value of status is greater than 100 then the value will be set to 100.
 *
 * @param status The convergence value to post.
 */
void Updater::updateConvergence(int status)
{
  // If status is less than 0, set status to 0
  if (status < 0)
    status = 0;

  // If status is greater than 100, set status to 100
  else if (status > 100)
    status = 100;

  // Create a new CONVERGENCE_UPDATED post
  std::stringstream ss;
  ss << status;
  addPostToQueue(CONVERGENCE_UPDATED, ss.str());
}

/**
 * Adds a Post object to posts queue containing the progress of the user simulation.
 * Status must be >=1 and <=100. If the value of status is less than 0 then the value will be set to 0.
 * If the value of status is greater than 100 then the value will be set to 100.
 *
 * @param status The progress value to post.
 */
void Updater::updateProgress(int status)
{
  // If status is less than 0, set status to 0
  if (status < 0)
    status = 0;

  // If status is greater than 100, set status to 100
  else if (status > 100)
    status = 100;

  // Create a new PROGRESS_UPDATED post
  std::stringstream ss;
  ss << status;
  addPostToQueue(PROGRESS_UPDATED, ss.str());
}

/**
 * Adds a Post object to the posts queue.
 *
 * @param type A PostType literal.
 * @param message The string message assigned to the Post.
 */
void Updater::addPostToQueue(PostType type, std::string message)
{
  // Lock the thread using a lock_guard object
  std::mutex mutex;
  std::lock_guard<std::mutex> lock(mutex);

  // Create a new post
  MooseSharedPointer<Post> postPtr(new Post(type, message));

  // Add the post to the posts queue
  updaterThread->push(postPtr);
}

/**
 * Sets the ignoreSslPeerVerification flag. If ignoreSslPeerVerification flag is
 * set to true then cURL will skip peer certificate verification for HTTPS urls.
 * This flag should only be set to true for testing purposes.
 *
 * @param ignoreSslPeerVerification The value for the ignoreSslPeerVerification flag.
 */
void Updater::setIgnoreSslPeerVerification(bool ignoreSslPeerVerification)
{
  // Set the instance value to the value in ignoreSslPeerVerification.
  this->ignoreSslPeerVerification = ignoreSslPeerVerification;
}

/**
 * Creates the thread object initialized with a reference to the threadProcess() operation.
 * Returns whether the thread was created successfully.
 *
 * @return true if the thread was started successfully.
 */
bool Updater::start()
{
  // If thread has not been created and the property map is valid
  if (!threadCreated && goodPropertyMap)
  {
    // Call placement new to allocate a task of type UpdaterThread
    // using the current innermost cancellation group.  Believe it or
    // not, this is "idiomatic TBB code".
    // https://www.threadingbuildingblocks.org/docs/help/reference/task_scheduler/task_allocation.htm
    updaterThread = new UpdaterThread(propertyMap, errorLoggerPtr, ignoreSslPeerVerification, noProxyFlag);
    updaterThread->execute();

    // Set flag to true
    threadCreated = true;

    // Create a new UPDATER_STARTED post
    addPostToQueue(UPDATER_STARTED, "");
  }

  // Return the value of threadCreated
  return threadCreated;
}

/**
 * Stops the thread by calling the interrupt operation on thread.
 * Returns whether the thread was interrupted successfully.
 *
 * @return true if the thread was stopped successfully.
 */
bool Updater::stop()
{
  // If thread has been created
  if (threadCreated)
  {
    // Create a new UPDATER_STOPPED post
    addPostToQueue(UPDATER_STOPPED, "");

    // Pause this thread for sleepTime seconds
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    // Stop the thread
    updaterThread->stopThread();

    // Set threadCreated back to false
    threadCreated = false;

    // We have stopped the updater
    // so we dump the errors into an error file.
    errorLoggerPtr->dumpErrors();
  }

  // Return not threadCreated
  return !threadCreated;
}

/**
 * The method passed to the thread constructor which transmits all
 * Posts in the posts queue to url with cURL in JSON format.
 * The thread is then put to sleep for 1000 milliseconds.
 */
void UpdaterThread::runThreaded()
{
  NetworkingToolFactory factory;

  // Get the item id from the propertyMap
  std::string itemId = propertyMap.at(ITEM_ID);

  // Get the item id from the propertyMap
  std::string clientKey = propertyMap.at(CLIENT_KEY);

  // Get the url from the propertyMap
  std::string url = propertyMap.at(URL);

  std::string tool = propertyMap.at(NETWORKING_TOOL);

  // Create a new INetworkingTool object to transmit posts
  networkingTool = factory.createNetworkingTool(tool);

  // Set the ignoreSslPeerVerification flag in networkTool
  networkingTool->setIgnoreSslPeerVerification(ignoreSslPeerVerification);
  networkingTool->setNoProxyFlag(noProxyFlag);

  // Start an infinite loop
  while (!stop)
  {
    // If we have posts to transmit
    if (!empty())
    {
      // Lock the thread using a lock_guard object
      std::mutex mutex;
      std::lock_guard<std::mutex> lock(mutex);

      // Create the transmission string and add initial json string to it
      std::string transmission = "{\"item_id\":\"" + itemId
        + "\", \"client_key\":\"" + clientKey + "\", \"posts\":[";

      // Get the first post
      PostPtr postPtr;

      if (this->pop(postPtr))
      {
        // Get the JSON from the post
        std::string json = postPtr->getJSON();

        // Add the json to the posts array in the transmission
        transmission += json;

        // If posts is still non empty, add a comma separator
        if (!empty())
          transmission += ",";

        // Add the final characters to the transmission
        transmission += "]}";

        // Transmit post to url and return an error if there is one
        std::string error = networkingTool->post(url,
                                              transmission,
                                              propertyMap[USERNAME],
                                              propertyMap[PASSWORD]);

        // Check to see if there was an error, and log it.
        if (error != "")
        {
           errorLoggerPtr->logError(error + "\nStopping ICE HTTP Update.");
           stop.store(true);
        }

      }
    }

    // Pause this thread for sleepTime seconds
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  return;
}

/**
 * Returns the contents of the updater.properties file as a string.
 *
 * @return The string contents of the updater.properties file if it exists in the current directory.
 */
std::string Updater::getPropertyFileContents()
{
  // Create a variable to hold the properties filename
  std::string propertiesFilename = "updater.properties";

  // Declare variable to holds contents of local file.
  std::string contents;

  // Declare input file stream.
  std::ifstream file;

  // Create an input file stream with the local test file.
  file.open(propertiesFilename.data());

  // Create a string buffer to store file contents.
  std::stringstream buffer;

  // Read contents of file into string buffer.
  buffer << file.rdbuf();

  // Convert buffer into a string and assign to contents.
  contents = buffer.str();

  // Close the file
  file.close();

  // Return contents.
  return contents;
}

/**
 * Creates and returns a PropertyMap type from a string formatted as a Java properties file.
 *
 * @param propertyString A string formatted as a Java properties file containing name/value pairs.
 * @return A PropertyMap comprised of the name/value pairs in the propertyString.
 */
PropertyMap Updater::getPropertyMap(std::string propertyString)
{
  // Create an empty map associating a PropertyType with a string value
  PropertyMap propertyMap;

  // Create a vector to hold the lines in the properties string
  std::vector<std::string> lines;

  // Split propertyString into lines and store in a vector
  lines = split(propertyString, '\n');

  // Loop over all tokens
  for (unsigned int i = 0; i < lines.size(); i++)
  {
    std::string line = lines[i];

    // Create a new vector to hold the name and value
    std::vector<std::string> pair;

    // If the "=" character is found in this line
    if (line.find("=") != std::string::npos)
    {
      // Split the line into a property/value pair
      pair = split(line, '=');

      // Get the property name
      std::string property = pair[0];

      // Get the property value
      std::string value = pair[1];

      // FIXME! This switch can be removed with a map. ~JJB 20140404 15:06

      // If the property is "item_id"
      if (property == "item_id")
      {
        // Insert PropertyType ITEM_ID and the value
        propertyMap.insert(PropertyMap::value_type(ITEM_ID, value));
        // If the property is "url"
      }
      else if (property == "url")
      {
        // Insert PropertyType URL and the value
        propertyMap.insert(PropertyMap::value_type(URL, value));
        // If the property is "client_key"
      }
      else if (property == "client_key")
      {
        // Insert PropertyType CLIENT_KEY and the value
        propertyMap.insert(PropertyMap::value_type(CLIENT_KEY, value));
      }
      else if (property == "username")
      {
        // Insert the username into the map
        propertyMap.insert(PropertyMap::value_type(USERNAME, value));
      }
      else if (property == "password")
      {
        // Insert the password into the map
        propertyMap.insert(PropertyMap::value_type(PASSWORD, value));
      }
      else if (property == "networkingTool")
      {
        propertyMap.insert(PropertyMap::value_type(NETWORKING_TOOL, value));
      }
    }
  }

  // Return the property map
  return propertyMap;
}

/**
 * Validates the propertyMap object and writes any errors to the error logger.
 *
 * @return True if the property map contains good values.
 */
bool Updater::validatePropertyMap()
{
  // Declare boolean flag and set value to true.
  bool goodPropertyMap = true;

  // If the map has less than 5 key/value pairs.
  if (propertyMap.size() < 5)
  {
    // Then log error and set flag to false.
    errorLoggerPtr->logError("The property map contains less than the five default values.");
    goodPropertyMap = false;
  }

  // If the map does not have an ITEM_ID key.
  if (propertyMap.find(ITEM_ID) == propertyMap.end())
  {
    // Then log error and set flag to false.
    errorLoggerPtr->logError("The property map does not contain a value for \"item_id\".");
    goodPropertyMap = false;
  }
  else if (propertyMap.at(ITEM_ID).empty())
  {
    // Then log error and set flag to false.
    errorLoggerPtr->logError("The property map contains an empty value for \"item_id\".");
    goodPropertyMap = false;
  }

  // If the map does not have an CLIENT_KEY key.
  if (propertyMap.find(CLIENT_KEY) == propertyMap.end())
  {
    // Then log error and set flag to false.
    errorLoggerPtr->logError("The property map does not contain a value for \"client_key\".");
    goodPropertyMap = false;
  }
  else if (propertyMap.at(CLIENT_KEY).empty())
  {
    // Then log error and set flag to false.
    errorLoggerPtr->logError("The property map contains an empty value for \"client_key\".");
    goodPropertyMap = false;
  }
  else if (propertyMap.at(CLIENT_KEY).size() != 40)
  {
    // Then log error and set flag to false.
    errorLoggerPtr->logError("The property map contains a value for \"client_key\" that is not exactly 40 characters.");
    goodPropertyMap = false;
  }

  // If the map does not have an URL key.
  if (propertyMap.find(URL) == propertyMap.end())
  {
    // Then log error and set flag to false.
    errorLoggerPtr->logError("The property map does not contain a value for \"url\".");
    goodPropertyMap = false;
  }
  else if (propertyMap.at(URL).empty())
  {
    // Then log error and set flag to false.
    errorLoggerPtr->logError("The property map contains an empty value for \"url\".");
    goodPropertyMap = false;
  }

  // If the map does not have an username key.
  if (propertyMap.find(USERNAME) == propertyMap.end())
  {
    // Then log error and set flag to false.
    errorLoggerPtr->logError("The property map does not contain a value for \"username\".");
    goodPropertyMap = false;
  }
  else if (propertyMap.at(USERNAME).empty())
  {
    // Then log error and set flag to false.
    errorLoggerPtr->logError("The property map contains an empty value for \"username\".");
    goodPropertyMap = false;
  }

  // If the map does not have an password key.
  if (propertyMap.find(PASSWORD) == propertyMap.end())
  {
    // Then log error and set flag to false.
    errorLoggerPtr->logError("The property map does not contain a value for \"password\".");
    goodPropertyMap = false;
  }
  else if (propertyMap.at(PASSWORD).empty())
  {
    // Then log error and set flag to false.
    errorLoggerPtr->logError("The property map contains an empty value for \"password\".");
    goodPropertyMap = false;
  }

  // Return the flag.
  return goodPropertyMap;
}

/**
 * Initializes ICEupdater by creating the property map and setting threadCreated to false.
 * Called only from the constructors.
 *
 * @param propertyString A string formatted as a Java properties file containing name/value pairs.
 */
void Updater::initialize(std::string propertyString)
{
  // Get the map of configuration properties
  propertyMap = getPropertyMap(propertyString);

  // Set threadCreated to false
  threadCreated = false;

  // Set the ignoreSslPeerVerification flag to false by default
  ignoreSslPeerVerification = false;

  // Create an ErrorLoggerPtr object.
  errorLoggerPtr = ErrorLoggerPtr(new ErrorLogger());

  // Validate the propertyMap and store the results
  goodPropertyMap = validatePropertyMap();

  // If the propertyMap was not successfully validated,
  // dump the errors into an error file.
  if (!goodPropertyMap)
    errorLoggerPtr->dumpErrors();
}

#endif // LIBMESH_HAVE_CXX11_THREAD && LIBMESH_HAVE_CXX11_CONDITION_VARIABLE
