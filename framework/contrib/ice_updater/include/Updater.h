/*******************************************************************************
 * Copyright (c) 2012, 2014 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *   Initial API and implementation and/or initial documentation - Jay Jay Billings,
 *   Jordan H. Deyton, Dasha Gorin, Alexander J. McCaskey, Taylor Patterson,
 *   Claire Saunders, Matthew Wang, Anna Wojtowicz
 *******************************************************************************/

#ifndef UPDATER_H
#define UPDATER_H

#include <string>
#include <queue>
#include <map>
#include <tbb/mutex.h>
#include <tbb/concurrent_queue.h>
#include "Post.h"
#include "PropertyType.h"
#include "ErrorLogger.h"
#include "MooseTypes.h"

using namespace std;

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
typedef map<PropertyType, string> PropertyMap;

/**
 * A PostPtrQueue is a queue template typed for PostPtr objects.
 */
typedef tbb::concurrent_queue<PostPtr> PostPtrQueue;

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
class Updater {

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
	 * A mutex to synchronize the posts queue.</p>
	 */
	tbb::mutex mutex;

//	/**
//	 * A PostPtrQueue object to contain PostPtr to Post objects
//	 * used for transmission.
//	 */
//	PostPtrQueue posts;

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
	 *
	 */
	UpdaterThread * updaterThread;

	/**
	 * Adds a Post object to the posts queue.
	 *
	 * @param type A PostType literal.
	 * @param message The string message assigned to the Post.
	 */
	void addPostToQueue(PostType type, string message);

	/**
	 * Returns the contents of the updater.properties file as a string.
	 *
	 * @return The string contents of the updater.properties file if it exists in the current directory.
	 */
	string getPropertyFileContents();

	/**
	 * Creates and returns a PropertyMap type from a string formatted as a Java properties file.
	 *
	 * @param propertyString A updater.properties formatted string.
	 * @return A PropertyMap comprised of the name/value pairs in the propertyString.
	 */
	PropertyMap getPropertyMap(string propertyString);

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
	void initialize(string propertyString);

public:

	/**
	 * The Constructor. Here, the Constructor will search for properties file in the current directory.
	 * This file must contain the url, item id and client key used for transmission.
	 */
	Updater();

	/**
	 * The Constructor. Here, the Constructor requires an input stream in the form of the
	 * updater.properties file format.
	 *
	 * @param stream An input stream in the form of the updater.properties file format.
	 */
	Updater(istream &stream);

	/**
	 * The Destructor.
	 */
	~Updater();

	/**
	 * Adds a Post object to the posts queue indicating the creation of the file located at path.
	 *
	 * @param path The path of the file.
	 */
	void postFileCreated(string path);

	/**
	 * Adds a Post object to the posts queue indicating the deletion of the file located at path.
	 *
	 * @param path The path of the file.
	 */
	void postFileDeleted(string path);

	/**
	 * Adds a Post to the posts queue indicating the modification of the file located at path.
	 *
	 * @param path The path of the file.
	 */
	void postFileModified(string path);

	/**
	 * Adds a plain text message Post to the posts queue.
	 *
	 * @param message A plain text message.
	 */
	void postMessage(string message);

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
	 * Status must be &gt;=1 and &lt;=100. If the value of status is less than 0 then the value will be set to 0.
	 * If the value of status is greater than 100 then the value will be set to 100.
	 *
	 * @param status The convergence value to post.
	 */
	void updateConvergence(int status);

	/**
	 * Adds a Post object to posts queue containing the progress of the user simulation.
	 * Status must be &gt;=1 and &lt;=100. If the value of status is less than 0 then the value will be set to 0.
	 * If the value of status is greater than 100 then the value will be set to 100.
	 *
	 * @param status The progress value to post.
	 */
	void updateProgress(int status);

};

/**
 *
 */
class UpdaterThread: public tbb::task, public PostPtrQueue {
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
	 * A mutex to synchronize the posts queue.</p>
	 */
	tbb::mutex mutex;

	tbb::atomic<bool> stop;

public:

	/**
	 *
	 */
	UpdaterThread(PropertyMap map, ErrorLoggerPtr error,
	bool ssl) :
			propertyMap(map), errorLoggerPtr(error), ignoreSslPeerVerification(
					ssl) {
		stop.store(false);// = false;
	}

	/**
	 * The method passed to the thread constructor which transmits all Posts in the
	 * posts queue to url with cURL in JSON format.
	 * The thread is then put to sleep for 1000 milliseconds.
	 */
	tbb::task* execute();

	/**
	 *
	 */
	void stopThread() {
		stop.store(true);
	}

};

#endif
