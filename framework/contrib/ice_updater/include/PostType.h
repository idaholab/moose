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

#ifndef POSTTYPE_H
#define POSTTYPE_H

/**
 * The PostType enumeration consists of literals representing each type of message.
 */
enum PostType {

    /**
     * This literal means that the message is associated with a file creation.
     */
    FILE_CREATED = 0,

    /**
     * This literal means that the message is associated with a file modification.
     */
    FILE_MODIFIED,

    /**
     * This literal means that the message is associated with a file deletion.
     */
    FILE_DELETED,

    /**
     * This literal means that the message is associated with a plain text message post.
     */
    MESSAGE_POSTED,

    /**
     * This literal means that the message is associated with a progress update.
     */
    PROGRESS_UPDATED,

    /**
     * This literal means that the message is associated with a convergence update.
     */
    CONVERGENCE_UPDATED,

    /**
     * This literal means that the message is associated with an updater start event.
     */
    UPDATER_STARTED,

    /**
     * This literal means that the message is associated with an updater stop event.
     */
    UPDATER_STOPPED

};

#endif
