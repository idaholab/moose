**Note:** All code shared within a MOOSE module will be made publicly available. Please do not publish any code or information that is export controlled or proprietary in nature.

###1. Create a new module ###

To create a new module, run the following command:

```bash
$MOOSE_DIR/scripts/stork.sh module YourModuleName
```

Please choose a name that is concise, descriptive for your physics. 
Your new module will be created inside of the `$MOOSE_DIR/modules/` directory.

###2. Register your new module###
Edit inside of the `ModuleApp.C` and the `modules.mk` files. Instructions for registering your module are contained within each of these files.

###3. Commit and push###
Commit your changes, push to Github, and put in a pull request.

