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

#include "Post.h"

/**
 * Constructor that sets the PostType and message for this post.
 *
 * @param type The PostType associated with this Post object.
 * @param message The plain text message stored in the Post.
 */
Post::Post(PostType type, string message) {

    //Assign type to instance variable.
    this->type = type;

    //If the message is greater than 256 characters, then truncate.
    if(message.length()>256) {
        message = message.substr(0, 256);
    }

    //Assign message to instance variable.
    this->message = message;
}

/**
 * The Destructor.
 */
Post::~Post() {

    return;
}

/**
 * Returns a JSON formatted string containing the post message and type.
 *
 * @return A JSON formatted string containing the post message and type.
 */
string Post::getJSON() {

    //Get the string representation of the PostType enum value.
    string typeString = Post::getPostTypeString(type);

    //Create the return JSON string.
    string json = "{\"type\":\"" + typeString + "\",\"message\":\"" + message + "\"}";

    //Return the json string;
    return json;
}

/**
 * Returns a string representation of the provided PostType.
 *
 * @param postType A PostType.
 * @return A string representation of the provided PostType.
 */
string Post::getPostTypeString(PostType postType) {

    //Declare and initialize local string variable.
    string postTypeString = "";

    //Switch on each enum type and set its string representation
    switch(postType) {

    case FILE_CREATED:
        postTypeString = "FILE_CREATED";
        break;
    case FILE_DELETED:
        postTypeString = "FILE_DELETED";
        break;
    case FILE_MODIFIED:
        postTypeString = "FILE_MODIFIED";
        break;
    case MESSAGE_POSTED:
        postTypeString = "MESSAGE_POSTED";
        break;
    case PROGRESS_UPDATED:
        postTypeString = "PROGRESS_UPDATED";
        break;
    case CONVERGENCE_UPDATED:
        postTypeString = "CONVERGENCE_UPDATED";
        break;
    case UPDATER_STARTED:
        postTypeString = "UPDATER_STARTED";
        break;
    case UPDATER_STOPPED:
        postTypeString = "UPDATER_STOPPED";
        break;
    }

    //Return the string representation of the selected enum type.
    return postTypeString;
}
