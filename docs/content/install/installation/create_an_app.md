## Create an Application
MOOSE is designed for building custom applications, therefore if you plan on working with MOOSE
then you should create an application.

Your application is where code and input files should be created for your particular problem.

To create an application the following steps must be followed.
##### (1) Run the Stork Script

```bash
cd ~/projects
./moose/scripts/stork.sh YourAppName
```

Running this script will create a folder named "YourAppName" in the projects directory, this
application will automatically link against MOOSE. Obviously, the "YourAppName" should be the
name you want to give to your application; consider the use of an acronym. We prefer animal names for applications, but you are free to choose whatever name suits your needs.

##### (2) Compile and Test Your Application

```bash
cd ~/projects/YourAppName
make -j4
./run_tests -j4
```

If your application is working correctly, you should see one passing test. This indicates that
your application is ready to be further developed.
