# Creating a Web-site

The primary motivation behind the [MOOSE documentation system](utilities/documentation/index.md) was to create a customizable, maintainable web-site for the MOOSE framework and modules. However, it was important to the the MOOSE developers that this system be available to application developers as well
to enable for MOOSE-based applications to be well-documented using a common syntax.

The first step for creating a web-site is to document your code, which is detailed here: [Documenting Code](documentation/code.md). Once this is complete then you should continue with the steps listed on this page that include creating [content](#creating-web-site-content), [page layout](#web-site-layout), [viewing](#viewing-your-web-site), and [deploying](#web-site-deployment) the site.

## Creating Web-Site Content
Adding content to the web-site is as simple as creating a markdown file with content, this markdown file should be located in the "contents" directory.

## Web-site Layout
The final step when preparing an application site is to define the website navigation, this is done in "navigation.yml" within
the documentation directory. This file is used to define the navigation menu located at the top of the website if you are using the default
template.

!text docs/navigation.yml

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
