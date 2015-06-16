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

#ifndef POST_H
#define POST_H

#include <string>
#include "PostType.h"

using namespace std;

/**
 * The Post class is a data structure used by the Updater to hold a
 * string message and its associated PostType.
 */
class Post {

private:

    /**
     * The plain text message stored in the Post.
     */
    string message;

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
    Post(PostType type, string message);

    /**
     * The Destructor.
     */
    ~Post();

    /**
     * Returns a JSON formatted string containing the post message and type.
     *
     * @return A JSON formatted string containing the post message and type.
     */
    string getJSON();

    /**
     * Returns a string representation of the provided PostType.
     *
     * @param postType A PostType literal.
     * @return A string representation of the provided PostType.
     */
    static string getPostTypeString(PostType postType);

};

#endif
