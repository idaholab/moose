!config navigation breadcrumbs=False scrollspy=False

# [!icon!error_outline tight=True] Page not found style=font-size:250%

The page you requested is not available in the current MooseDocs build.

You may have used the `--config` argument without including the required configuration file. For example, if you wanted to serve both the main MOOSE website and the workshop, you would run the following commands:

!listing
cd modules/doc
./moosedocs.py build --serve --config config.yml workshop_config.yml

Or, you would simply use the default list of configuration files by omitting the `--config` option.
