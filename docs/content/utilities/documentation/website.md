# Creating a Web-site

The primary motivation behind the [MOOSE documentation system](documentation/index.md) was to create a customizable, maintainable web-site for the MOOSE framework and modules. However, it was important to the the MOOSE developers that this system be available to application developers as well
to enable for MOOSE-based applications to be well-documented using a common syntax.

The first step for creating a web-site is to document your code, which is detailed here: [Documenting Code](documentation/code.md). Once this is complete then you should continue with the steps listed on this page that include creating [content](#creating-web-site-content), [page layout](#web-site-layout), [viewing](#viewing-your-web-site), and [deploying](#web-site-deployment) the site.

## Creating Web-Site Content

Adding content to the web-site is as simple as creating a markdown file with content, this markdown file should be located in the "docs_dir", which is specifed in your configuration file. See the [configuration](documentation/setup.md#configuration) on the setup page.

## Web-site Layout

The final step when preparing an application site is to define the website page layout, this is done in "pages.yml" within
the docs directory. The information within this file mimics the [mkdocs pages](http://www.mkdocs.org/user-guide/configuration/#pages)
configuration, with one exception: it is possible to include other markdown files. For example, the "pages.yml" for
the MOOSE website includes the following.

!text docs/pages.yml

Notice, that the framework and the modules each have include statements pointing to another "pages.yml" files. This
file is generated for object in the application and is placed in the "install" directory (see [Setup](content/utilities/documentation/setup.md)).

## Viewing your Web-site
When you want to view your documentation website, you must serve the site to a local server. This is done as:
```text
./moosedocs.py serve
```

To see a list of additional options for this command run with the "-h" flag.
```text
./moosedocs serve -h
```

## Web-site Deployment
To deploy the website, run the script with the 'build' option. This will create the site in the location is given
by "site_dir" in your MkDocs Configure File (mkdocs.yml). This site can then be copied to a server for
hosting.

```text
./moosedocs.py build
```

To see a list of additional options for this command run with the "-h" flag.
```text
./moosedocs build -h
```
