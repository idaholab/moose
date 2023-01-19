# MooseDocs Configuration

In order to use the MooseDocs `build` command, one or more configuration files are required. By
default the `moosedocs.py` script looks for any file in the same directory as itself with a name
that ends with "config.yml" (the `--config` option can be used to specify the files directly). As
the extension suggests, these must be [YAML] files. The configurations must each contain a `Content`
section, and optionally the additional sections described here to fine-tune your MooseDocs build.

## Content

The content section is typically a list of directories that contain the markdown content that
is desired to be rendered to HTML, as shown below. It is possible to use environment variables
within the list. In particular, `MOOSE_DIR` and `ROOT_DIR` are always available, where `ROOT_DIR`
is the directory of your application.

```yaml
Content:
    - doc/content
    - ${MOOSE_DIR}/framework/doc/content
```

The path may also include `*` and `**` wildcards. The `*` wildcard will match any single directory
or filename. For example, `doc/content/*/minimal` will include all sub-directories of the content
directory that contain a `minimal` directory. The `**` wildcard allows for arbitrary levels of
nesting. For example `doc/content/**/minimal` will include the `minimal` directory for any
directory below the content directory, regardless of depth.

### Content Across Multiple Configurations id=multiconfigs

For the case of multiple configuration files, +redundant content is not permitted+. That is, any two
configurations may not specify the same file of any type---directories are the only exception. The
content of each is added to a shared pool so the complete, combined list of files will be available
to all translators. Importantly, the respective configurations of content files will be honored
during translation allowing different pages to use different readers, renderers, executioners, and
extensions, which provides an extraordinary amount of flexibility about how your website is built.

As a good example of how to resolve content conflicts, consider the main
MOOSE website along with the [workshop/index.md alternative=missing_config.md].
It is possible to build the [main site configuration](modules/doc/config.yml language=yaml) by
itself by running the following commands:

!listing language=bash
cd modules/doc
./moosedocs.py build --config config.yml

Or the [workshop configuration](tutorials/darcy_thermo_mech/doc/config.yml language=yaml) by

!listing language=bash
cd tutorials/darcy_thermo_mech/doc
./moosedocs.py build

However, these two configurations specify some of the same content. Thus, we made use of the YAML
`!include` syntax to create a separate workshop configuration specialized for a combined build and
it is shown in [workshop_config]. In this file, we made sure to not add any files already added by
the main site configuration, which left only the actual workshop pages. All of the media,
bibliographies, JavaScript, etc., that it needs will still be available in the combined build. Then
the configurations of the Executioner, Renderer, and Extensions were simply imported from the
original file.

!listing modules/doc/workshop_config.yml language=yaml id=workshop_config
         caption=Configuration for the MOOSE workshop designed to build alongside the main website
                 that takes advantage of the `!include` syntax.

!alert tip title=Backup links
If you find yourself creating autolinks across pages from different configurations and you are
concerned about them failing when only building one of those configurations, consider using the
`alternative` setting (see the [MooseDocs/extensions/autolink.md] page for more information).

### Advanced Content

There is also a more advanced interface for the content, but it is required to explain how these
directories are used. When building content all relevant files (e.g., markdown) are extracted from
the supplied list of directories into a set, with the path relative to the supplied directory. This
unique set of files is then copied to the destination directory of the build, with markdown files
being rendered to html first. For example, the following illustrates how the multiple source
files are combined in the destination directory.

```text
doc/content/index.md -> ${HOME}/.local/share/moose/site/index.html
${MOOSE_DIR}/framework/doc/content/utilities/index.md -> ${HOME}/.local/share/moose/site/utilties/index.md
```

In the advanced mode the root location can be specified for each location, in this case the
content configuration section contains a dictionary of dictionaries as shown below. The top-level key
is an arbitrary name, the second level has two keys available: "root_dir" and "content". The
root_dir is the directory to include and "content" is a list of folders and/or files within the
root_dir to consider.


```yaml
Content:
    app:
        root_dir: doc/content
    framework:
        root_dir: ${MOOSE_DIR}/framework/doc/content/utilities
        content:
            - MooseDocs
```

Given the above configuration, the files within the MooseDocs folder are now included directly
within the destination folder (i.e., the "utilities" directory is removed) as shown below.

```text
doc/content/index.md -> ${HOME}/.local/share/moose/site/index.html
${MOOSE_DIR}/framework/doc/content/utilities/MooseDocs/setup.md -> ${HOME}/.local/share/moose/site/MooseDocs/setup.md
```

## Extensions

The Extensions section is used to enable non-default or custom extensions as well as change the
configuration for existing extensions. The list of default extensions is shown in
[default-extensions]. For example, the following snippet changes the settings for the
[common.md] extension. Note, the first level key name ("globals") is arbitrary and the
"type" key name under that is required. The value matches with the python package loaded, as in
[default-extensions].

```yaml
Extensions:
    globals:
        type: MooseDocs.extensions.common
        shortcuts: !include framework/doc/globals.yml
```

The various extensions as well as links to the documentation for each extension, which includes
the available configuration options, is found on the [specification page](MooseDocs/specification.md).

!listing common/load_config.py
         id=default-extensions
         caption=List of default MooseDocs extensions.
         start=DEFAULT_EXTENSIONS
         end=]
         include-end=True


## Renderer

The Renderer section allows for the type of renderer to be selected, the default is the
`MaterializeRenderer`. This is the only complete renderer object and it does have many available
options, please refer to the source code for the available options. An example
Renderer section is shown below.

!listing modules/doc/config.yml language=yaml start=Renderer end=Extensions

## Reader

The Reader section allows the reader object to be defined, by default the `MarkdownReader` objects
is used. Currently, this is the only type of reader and there are no configuration options, so this
section is not necessary.
