# Create an Appplication
Creating your very own MOOSE application is a cinch: you do it by "Forking the Stork!"... which is our application template:

---
## Choose a new application name.
* Consider the use of an acryonym. We prefer animal names for applications, but you are free to choose whatever name suits your needs.

---
## Fork Stork!
* Go to the Stork GitHub Page and Fork the Stork! by clicking the "Fork" button in the upper right of the page.

---
## Rename your fork
* Click on your repository, then the Settings button
* Rename your repository
!!! note
    Rename your repository on Github to match your animal name!

---
## Clone your fork
* Clone your fork to your workstation in the projects directory we made earlier (if you didn't make a projects directory then you'll want to just make sure you clone your application right next to your moose directory)

    <pre>
    cd ~/projects
    git clone https://github.com/[username]/[app_name].git
    </pre>

* Run the make_new_application.py script

    <pre>
    cd app_name
    ./make_new_application.py
    </pre>

!!! note
    This script will modify your cloned repository preparing it for immediate use!

---
## Commit your changes
* Commit your changes back to your local repository and push.

    <pre>
    git commit -a -m 'Beginning my new application'
    git push
    </pre>

---
## Register Your Application
* When you are ready, register your application so others can see what you are doing and possibly use or enhance your code. Simply add your application to the list here: TrackedApps

---
### What now?
* Head over to the Documentation section to view all the various forms of documentation and walk through the example applications to find out how to solve your equations with MOOSE.