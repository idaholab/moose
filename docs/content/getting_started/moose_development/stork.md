# Stork
Creating your very own MOOSE application is a cinch.  Once you have installed the MOOSE framework, do the following:


### Run the stork script
For your app name, consider the use of an acryonym. We prefer animal names for applications, but you are free to choose whatever name suits your needs. Outside of the MOOSE directory in `~/projects` type the following into a terminal:

```bash
$MOOSE_DIR/scripts/stork.sh YourAppName
```

where `$MOOSE_DIR` is the path to your MOOSE installation (e.g. `/home/you/projects/moose` or similar).  This will create a directory named `yourappname` with everything you need to get started.

### Build your app

```bash
cd yourappname
make -j4
```

You have successfully built your first MOOSE app.  The application binary is a file called `yourappname-opt`.

### Commit your changes

When you ran the `stork.sh` script, it printed out the following directions:


```bash
To store your changes on github:
    1. log in to your account
    2. Create a new repository named YourAppName
    3. in this terminal run the following commands:
         cd foopar
         git remote add origin https://github.com/YourGitHubUserName/YourAppName
         git commit -m "initial commit"
         git push -u origin master
```

Follow them to start tracking your project using version control with git.

### Get social!

[Join the mailing list](https://groups.google.com/forum/#!forum/moose-users)

Tweet about it using the Twitter button in the column to the right!

### Register Your Application
When you are ready, register your application so others can see what you are doing and possibly use or enhance your code. Simply add your application to the list here: [TrackedApps](http://mooseframework.org/wiki/TrackedApps)


### What now?

Head over to the [Documentation section](http://mooseframework.com/documentation/) to view all the various forms of documentation and walk through the example applications to find out how to solve your equations with MOOSE.
