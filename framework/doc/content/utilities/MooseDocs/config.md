# MooseDocs Configuration

In order to use MooseDocs build command a configuration file is required. By default the
`moosedocs.py` script looks for a `config.yml` file (the `--config` option can be used if a different
name is desired). As the extension suggests, this file is a [YAML] file. The configuration file must
contain a `Content` section and may optionally contain additional sections, as listed below, to
configure your MooseDocs build.

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
