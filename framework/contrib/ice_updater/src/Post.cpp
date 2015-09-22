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
#include "Post.h"

/**
 * Constructor that sets the PostType and message for this post.
 *
 * @param type The PostType associated with this Post object.
 * @param message The plain text message stored in the Post.
 */
Post::Post(PostType type, std::string message)
{
  // Assign type to instance variable.
  this->type = type;

  // If the message is greater than 256 characters, then truncate.
  if (message.length() > 256)
    message = message.substr(0, 256);

  // Assign message to instance variable.
  this->message = message;
}


/**
 * Returns a JSON formatted string containing the post message and type.
 *
 * @return A JSON formatted string containing the post message and type.
 */
std::string Post::getJSON()
{
  // Get the string representation of the PostType enum value.
  std::string typeString = Post::getPostTypeString(type);

  // Create the return JSON string.
  std::string json = "{\"type\":\"" + typeString + "\",\"message\":\"" + message + "\"}";

  // Return the json string;
  return json;
}

/**
 * Returns a string representation of the provided PostType.
 *
 * @param postType A PostType.
 * @return A string representation of the provided PostType.
 */
std::string Post::getPostTypeString(PostType postType)
{
  // Declare and initialize local string variable.
  std::string postTypeString = "";

  // Switch on each enum type and set its string representation
  switch(postType)
  {
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

  // Return the string representation of the selected enum type.
  return postTypeString;
}
