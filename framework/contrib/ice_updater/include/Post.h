/*******************************************************************************
 * Copyright (c) 2015, UT-Battelle, LLC.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in
 *  the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *  contributors may be used to endorse or promote products derived
 *  from this software without specific prior written permission.
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
#ifndef POST_H
#define POST_H

#include <string>
#include "PostType.h"

/**
 * The Post class is a data structure used by the Updater to hold a
 * string message and its associated PostType.
 */
class Post
{
private:
  /**
   * The plain text message stored in the Post.
   */
  std::string message;

  /**
   * The PostType associated with this Post object.
   */
  PostType type;

public:
  /**
   * Constructor that sets the PostType and message for this post.
   *
   * @param type The PostType associated with this Post object.
   * @param message The plain text message stored in the Post.
   */
  Post(PostType type, std::string message);

  /**
   * Empty Destructor.
   */
  ~Post() {}

  /**
   * Returns a JSON formatted string containing the post message and type.
   *
   * @return A JSON formatted string containing the post message and type.
   */
  std::string getJSON();

  /**
   * Returns a string representation of the provided PostType.
   *
   * @param postType A PostType literal.
   * @return A string representation of the provided PostType.
   */
  static std::string getPostTypeString(PostType postType);
};

#endif
