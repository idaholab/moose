#!/usr/bin/env python
# ******************************************************************************
# Copyright (c) 2015 UT-Battelle, LLC.
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v1.0
# which accompanies this distribution, and is available at
# http://www.eclipse.org/legal/epl-v10.html
#
# Contributors:
#   Andrew Bennett - Initial API and implementation and/or initial documentation
#
# ******************************************************************************
# The Python ICE Installer
# ******************************************************************************

from __future__ import print_function
import os
import sys
import glob
import time
import stat
import errno
import shutil
import tarfile
import zipfile
import fnmatch
import platform
import datetime
import argparse
import itertools
import subprocess


if sys.version_info >= (3,0):
    import urllib.request as urllib2
else:
    import urllib2

def parse_args(args):
    """ Parse command line arguments and return them. """
    parser = argparse.ArgumentParser(description="ICE Installer script.",
                formatter_class=argparse.ArgumentDefaultsHelpFormatter,
                fromfile_prefix_chars='@')

    parser.add_argument('-u', '--update', nargs='*', default=['all'],
            choices=("all", "none", "VisIt", "HDFJava", "ICE"),
            help='The packages to update.  Leave blank to update all available packages.')
    parser.add_argument('-p', '--prefix', default=os.path.abspath(os.path.join(".","ICE")),
            help="The location to download and install ICE.")
    parser.add_argument("--unstable", action='store_true', help='Install the unstable nightly version of ICE.')
    parser.add_argument("--with-hdfjava", help="The path to an installation of HDFJava")
    parser.add_argument("--with-visit", help="The path to an installation of VisIt")
    parser.add_argument("--skip-download", action='store_true',
                        help='Do not download new packages, use previously downloaded ones.')
    parser.add_argument('--cleanup', action='store_true',
                        help='Remove downloaded packages after installation.')

    opts = parser.parse_args(args)

    # If update option was given blank set it to update everything
    if opts.update == [] or 'all' in opts.update:
        opts.update = ['ICE', 'VisIt', 'HDFJava']

    if opts.with_hdfjava is not None and 'HDFJava' in opts.update:
        print("")
        print("--------------------------- WARNING -----------------------------")
        print("Options used to install HDFJava and use an existing installation.")
        print("We will try to use the existing installation.  If this does not work")
        print("try running again without the --with-hdfjava option.")
        print("--------------------------- WARNING -----------------------------")
        print("")

    if opts.with_visit is not None and 'VisIt' in opts.update:
        print("")
        print("--------------------------- WARNING -----------------------------")
        print("Options used to install HDFJava and use an existing installation.")
        print("We will try to use the existing installation.  If this does not work")
        print("try running again without the --with-hdfjava option.")
        print("--------------------------- WARNING -----------------------------")
        print("")
    return opts


def mkdir_p(path):
    """ Operates like mkdir -p in a Unix-like system """
    try:
        os.makedirs(path)
    except OSError as e:
        if os.path.exists(path) and os.path.isdir(path):
            pass
        else:
            print("")
            print("--------------------------- ERROR -----------------------------")
            print("Cannot create directory " + path + ".  File already exists.")
            print("Either delete this file, or specify a different installation")
            print("location by using the --prefix option.")
            print("--------------------------- ERROR -----------------------------")
            print("")
            exit()


def get_os_and_arch():
    """ Do some converting and enforcing of OS and architecture settings. """
    allowed_os = ['Windows', 'Darwin', 'Linux']
    allowed_arch = ['x86_64', 'x86']
    arch_type = platform.machine()
    os_type = platform.system()
    if arch_type == "AMD64": arch_type = "x86_64"
    # TODO: Add more processing for different things
    if os_type not in allowed_os or arch_type not in allowed_arch:
        print("ERROR: Incorrect architecture or operating system.")
        exit()
    return os_type, arch_type


def print_header(opts, os_type, arch_type):
    print("Preparing to install ICE...")
    print("")
    if not opts.skip_download:
        print("Downloading and installing:")
    else:
        print("Installing the following packages:")
    for pkg in opts.update:
        print("  " + pkg)


def get_package_file(pkg, os_type, arch_type):
    package_files = {"ICE" : {"Windows" : {"x86_64" : "ice.product-win32.win32.x86_64.zip"     ,
                                           "x86"    : "ice.product-win32.win32.x86.zip"        },
                              "Darwin"  : {"x86_64" : "ice.product-macosx.cocoa.x86_64.zip"    ,
                                           "x86"    : "ice.product-macosx.cocoa.x86.zip"       },
                              "Linux"   : {"x86_64" : "ice.product-linux.gtk.x86_64.zip"       ,
                                           "x86"    : "ice.product-linux.gtk.x86.zip"          }},
                 "VisIt"   : {"Windows" : {"x86_64" : "visit2.10.0_x64.exe"                     ,
                                           "x86"    : "visit2.10.0.exe"                         },
                              "Darwin"  : {"x86_64" : "VisIt-2.10.0.dmg"                        },
                              "Linux"   : {"x86_64" : "visit2_10_0.linux-x86_64-rhel6.tar.gz"   }},
                 "HDFJava" : {"Windows" : {"x86_64" : "HDFView-2.11-win64-vs2012.zip"          ,
                                           "x86"    : "HDFView-2.11-win32-vs2012.zip"          },
                              "Darwin"  : {"x86_64" : "HDFView-2.11.0-Darwin.dmg"                     },
                              "Linux"   : {"x86_64" : "HDFView-2.11-centos6-x64.tar.gz"        }}}
    return package_files[pkg][os_type][arch_type]


def download_packages(opts, os_type, arch_type):
    """
    Pull down the appropriate packages for the given run, OS type, and machine architecture

    Args:
        opts: the list of options selected
        os_type: the operating system to download for
        arch_type: the architecture of the system
    """
    packages = opts.update
    if packages == [] or os_type == None or arch_type == None:
        return
    date = (datetime.date.today()- datetime.timedelta(1)).isoformat().replace('-','')
    package_urls = {"ICE" : "http://eclipseice.ornl.gov/downloads/ice/",
                    "VisIt" : "http://eclipseice.ornl.gov/downloads/visit/",
                    "HDFJava" : "http://www.hdfgroup.org/ftp/HDF5/hdf-java/current/bin/"}
    # TODO: If a site from packag_urls is down we can try to download from one of these
    backup_urls = {"ICE"     : "http://sourceforge.net/projects/niceproject/files/nightly/nice/",
                    "VisIt"   : "http://portal.nersc.gov/project/visit/releases/2.9.1/",
                    "HDFJava" : "http://www.hdfgroup.org/ftp/HDF5/hdf-java/current/bin/"}
    if opts.unstable:
        package_urls['ICE'] = "http://eclipseice.ornl.gov/downloads/ice/unstable-nightly/" + date + '/'
    else:
        package_urls['ICE'] = "http://eclipseice.ornl.gov/downloads/ice/stable-nightly/" + date + '/'
    files = dict()
    for pkg in packages:
        fname = get_package_file(pkg, os_type, arch_type)
        files[pkg] = fname
        if not opts.skip_download:
            print("Downloading " + pkg + ":")
            url = package_urls[pkg] + fname
            u = urllib2.urlopen(url)
            f = open(fname, 'wb')
            info = {k.lower():v for k,v in dict(u.info()).items()}
            fsize = int(info['content-length'])
            dl_size = 0
            block = 8192
            while True:
                buffer = u.read(block)
                if not buffer: break
                dl_size += len(buffer)
                f.write(buffer)
                status = r"  %5.2f%%" % (dl_size * 100. / fsize)
                status = status + chr(8)*(len(status)+1)
                print(status,end='')
            print("")
    return files


def unzip_package(pkg, file_path, out_path):
    """ Unzips file_path to out_path """
    print("Unpacking " + file_path + "....")
    mkdir_p(out_path)
    if 'Darwin' not in get_os_and_arch():
        pkg = zipfile.ZipFile(file_path)
        pkg.extractall(out_path)
    else:
        unzip_cmd = ['unzip', '-q', '-o', file_path, '-d', out_path]
        subprocess.call(unzip_cmd)
    return out_path


def untar_package(pkg, file_path, out_path):
    """ Untars file_path to out_path """
    print("Unpacking " + file_path + "....")
    mkdir_p(out_path)
    pkg = tarfile.open(file_path)
    dir_name = os.path.commonprefix(pkg.getnames())
    if os.path.isdir(os.path.join(out_path, dir_name)):
        shutil.rmtree(os.path.join(out_path,dir_name))
    pkg.extractall(out_path)
    pkg.close()
    return dir_name

def undmg_package(pkg, file_path, out_path):
    """ Extracts contents of file_path to out_path """
    print("Unpacking " + file_path + " to " + out_path + "....")
    mnt_point = os.path.join(out_path, 'mnt')
    mkdir_p(mnt_point)
    mount_cmd = ['hdiutil', 'attach', '-mountpoint', mnt_point, file_path, '-quiet']
    unmount_cmd = ['hdiutil', 'detach', mnt_point, '-force', '-quiet']
    subprocess.Popen(mount_cmd)
    time.sleep(3)
    subprocess.Popen(unmount_cmd)
    time.sleep(3)
    subprocess.Popen(mount_cmd)
    time.sleep(3)
    content = find_dir(mnt_point, "Resources")
    if content is None:
        return
    print("  Copying " + content + " to " + os.path.join(out_path,pkg) + "....")
    if os.path.exists(os.path.join(out_path, pkg)):
        shutil.rmtree(os.path.join(out_path, pkg))
    shutil.copytree(content, os.path.join(out_path,pkg))
    time.sleep(3)
    subprocess.Popen(unmount_cmd)


def unpack_packages(opts, pkg_files):
    """ Delegates unpacking of packages """
    dirs = dict()
    for pkg, archive in pkg_files.items():
        if archive.endswith(".tar.gz") or archive.endswith(".tgz") or archive.endswith(".tar"):
            dirs[pkg] = untar_package(pkg, archive, opts.prefix)
        elif archive.endswith(".zip"):
            dirs[pkg] = unzip_package(pkg, archive, opts.prefix)
        elif archive.endswith(".dmg"):
            dirs[pkg] = undmg_package(pkg, archive, opts.prefix)
        elif archive.endswith(".exe"):
            dirs[pkg] = archive
    return dirs


def find_file(dir, fname):
    """ Warning: this only finds the first file that matches """
    if fname == "*.app":
        if glob.glob(os.path.join(dir,fname)):
            return glob.glob(os.path.join(dir,fname))[0]
    for root, dirs, files in os.walk(dir):
        for basename in files:
            if fnmatch.fnmatch(basename, fname):
                filename = os.path.join(root, basename)
                return filename
    return None


def find_dir(dir, dirname):
    """ Warning: this only finds the first directory that matches """
    for root, dirs, files in os.walk(dir):
        for dir in dirs:
            if fnmatch.fnmatch(dir, dirname):
                return os.path.join(root, dir)
    return None


def nix_install(opts, pkg_dirs):
    """ Install packages for *nix """
    if "HDFJava" in pkg_dirs.keys():
        print("Installing HDFJava...")
        install_script = find_file(opts.prefix, "HDFView*.sh")
        if install_script is not None:
            install_cmd = [install_script, "--exclude-subdir", "--prefix="+os.path.join(opts.prefix,pkg_dirs['HDFJava'])]
            subprocess.call(install_cmd)

    hdf_path = opts.with_hdfjava if opts.with_hdfjava else opts.prefix
    hdf_libdir = find_file(hdf_path, "libhdf.a")
    if hdf_libdir is None:
        print("")
        print("--------------------------- ERROR -----------------------------")
        print("Could not find a usable HDFJava library.  Try downloading")
        print("a fresh copy using this installer by providing the --update")
        print("")
        print("Alternatively you may specify the location of an existing")
        print("HDFJava installation using the --with-hdfjava option.")
        print("option without any arguments")
        print("--------------------------- ERROR -----------------------------")
        print("")
        exit()
    hdf_libdir = os.path.abspath(os.path.dirname(hdf_libdir))

    visit_path = opts.with_visit if opts.with_visit is not None else opts.prefix
    visit_bin_dir = find_file(visit_path, "visit")
    if visit_bin_dir is None:
        print("")
        print("--------------------------- ERROR -----------------------------")
        print("Could not find a usable VisIt executable.  Try downloading")
        print("a fresh copy using this installer by providing the --update")
        print("option without any arguments")
        print("")
        print("Alternatively you may specify the location of an existing")
        print("VisIt installation using the --with-visit option.")
        print("--------------------------- ERROR -----------------------------")
        print("")
        exit()
    visit_bin_dir = os.path.abspath(os.path.dirname(visit_bin_dir))

    ice_preferences = find_file(opts.prefix, "ICE.ini")
    if ice_preferences == None:
        print("")
        print("--------------------------- ERROR -----------------------------")
        print("Could not find a usable ICE preferences file.  Try downloading")
        print("a fresh copy using this installer by providing the --update")
        print("option without any arguments")
        print("--------------------------- ERROR -----------------------------")
        print("")
        exit()
    ice_preferences = os.path.abspath(ice_preferences)
    shutil.move(ice_preferences, ice_preferences + ".bak")
    with open(ice_preferences + ".bak") as infile:
        filedata = infile.read()

    if visit_bin_dir is not None:
        filedata = filedata.replace("-Dvisit.binpath=@user.home/visit/bin", "-Dvisit.binpath=" +
                                    visit_bin_dir)
        with open(ice_preferences, 'w') as outfile:
            outfile.write(filedata)

    if hdf_libdir is not None:
        with open(ice_preferences, 'a') as outfile:
            outfile.write("-Djava.library.path=" + hdf_libdir)


def windows_install(opts, pkg_dirs):
    """ Install packages for Windows """
    if "HDFJava" in pkg_dirs.keys():
        print("Installing HDFJava...")
        install_script = find_file(opts.prefix, "HDFView*.exe")
        install_cmd = [install_script]
        subprocess.call(install_cmd, shell=True)

    hdf_libdir = os.path.dirname(find_file("C:\\", "libhdf.lib"))
    if hdf_libdir is None:
        print("ERROR: Could not find HDF Java libraries.")
        exit()

    if "VisIt" in pkg_dirs.keys():
        print("Installing VisIt...")
        install_script = find_file(os.getcwd(), "visit*.exe")
        install_cmd = [install_script]
        subprocess.call(install_cmd)

    visit_bin_dir = os.path.dirname(find_file("C:\\", "visit*.exe"))
    if visit_bin_dir is None:
        print("ERROR: Could not find VisIt executable.")
        exit()

    ice_preferences = find_file(opts.prefix, "ICE.ini")
    if ice_preferences is None:
        print("ERROR: Could not find ICE preferences directory.")
        exit()
    shutil.move(ice_preferences, ice_preferences + ".bak")
    with open(ice_preferences + ".bak") as infile:
        filedata = infile.read()

    filedata = filedata.replace("-Dvisit.binpath=@user.home/visit/bin", "-Dvisit.binpath=" +
                                os.path.join(os.path.abspath(opts.prefix),visit_bin_dir))

    with open(ice_preferences, 'w') as outfile:
        outfile.write(filedata)

    with open(ice_preferences, 'a') as outfile:
        outfile.write("-Djava.library.path=" + hdf_libdir)


def linux_post(opts, pkgs):
    """ Post installation for Linux """
    print("Generating desktop file for ICE...")
    with open(os.path.join(opts.prefix,"splash.bmp"),'wb') as f:
        f.write(urllib2.urlopen('https://raw.githubusercontent.com/eclipse/ice/master/org.eclipse.ice.client.rcp/splash.bmp').read())
    if 'SUDO_USER' in os.environ:
        user = os.environ['SUDO_USER']
    else:
        user = os.environ['USER']
    os.chmod(os.path.join(opts.prefix, "ICE"), stat.S_IXUSR | \
                                               stat.S_IRUSR | \
                                               stat.S_IWUSR | \
                                               stat.S_IRGRP | \
                                               stat.S_IWGRP | \
                                               stat.S_IROTH | \
                                               stat.S_IWOTH)

    mkdir_p(os.path.join('/home', user, ".local", "share", "applications"))
    with open(os.path.join('/home', user, ".local", "share", "applications","ICE.desktop"),'w') as f:
        f.write("[Desktop Entry]")
        f.write("\nType=Application")
        f.write("\nName=ICE")
        f.write("\nExec=" + os.path.join(opts.prefix, "ICE"))
        f.write("\nComment=Eclipse Integrated Computational Environment")
        f.write("\nIcon=" + os.path.join(opts.prefix, "splash.bmp"))
        f.write("\nTerminal=true")
        f.write("\nCategories=Programming")
        f.write("\n")


def osx_post(opts, pkgs):
    """ Post installation for OS X """
    mkdir_p(os.path.join(opts.prefix, "ICE.app", "Contents", "MacOS"))
    script_path = os.path.join(opts.prefix, "ICE.app", "Contents", "Info.plist")
    visit_libdir = os.path.dirname(find_file(opts.prefix, "libvisit*"))
    plutil_cmd = ['plutil', '-replace', 'CFBundleExecutable', '-string', 'ice.sh', script_path]
    lsregister_cmd = ['/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister',
                      '-v', '-f', os.path.join(opts.prefix, 'ICE.app')]
    ln_cmd = ['ln', '-sf', os.path.abspath(os.path.join(opts.prefix, "ICE.app")), os.path.join(os.path.expanduser("~"),'Applications','ICE.app')]
    with open(os.path.join(opts.prefix, "ICE.app", "Contents", "MacOS", "ice.sh"), 'w') as f:
        f.write('#!/bin/bash')
        f.write('\nsource ~/.bash_profile')
        f.write('\nexport DYLD_LIBRARY_PATH=' + visit_libdir + ':$DYLD_LIBRARY_PATH')
        f.write('\nexec `dirname $0`/ICE $0')
        f.write('\n')
    os.chmod(os.path.join(opts.prefix, "ICE.app", "Contents", "MacOS", "ice.sh"), stat.S_IXUSR)
    ice_preferences = find_file(opts.prefix, 'ICE.ini')
    with open(ice_preferences, 'a') as f:
        f.write("\n-Xdock:name=Eclipse ICE")
    subprocess.Popen(plutil_cmd)
    subprocess.Popen(lsregister_cmd)
    subprocess.Popen(ln_cmd)


def windows_post(opts, pkgs):
    """ Post installation for Windows """
    pass

''' Execute Git with the given arguments '''
def git(*args):
    return subprocess.check_call(['git'] + list(args))

''' Execute the sh command with the given arguments '''
def executeShellScript(*args):
    return subprocess.check_call(['sh'] + list(args))

''' Execute make with the given arguments '''
def executeMake(*args):
    return subprocess.check_call(['make'] + list(args))

def main():
    """ Run the full installer. """
    opts = parse_args(sys.argv[1:])
    os_type, arch_type = get_os_and_arch()

    install_funct = {"Windows" : windows_install,
                "Darwin" : nix_install,
                "Linux" : nix_install}
    post_funct = {"Windows" : windows_post,
                  "Darwin" : osx_post,
                  "Linux" : linux_post}

    print_header(opts, os_type, arch_type)
    pkg_files = download_packages(opts, os_type, arch_type)
    pkg_dirs = unpack_packages(opts, pkg_files)
    install_funct[os_type](opts, pkg_dirs)
    post_funct[os_type](opts, pkg_dirs)

    # Get the Moose directory
    script_path = sys.path[0]
    moose_dir = os.path.dirname(script_path)

    # Remove bad files from moose repo
    os.chdir(moose_dir)
    subprocess.check_call("rm -rf *.dmg *.zip ICE/mnt", shell=True)

    # Clone ASIO and kick off a Moose build
    os.chdir(moose_dir+"/framework/contrib/")
    if (not os.path.exists("asio")):
        git("clone", "https://github.com/chriskohlhoff/asio")
        os.chdir("asio")
        subprocess.check_call("mv asio/include .", shell=True)
        subprocess.check_call("rm -rf asio", shell=True)

    os.chdir(moose_dir)
    executeShellScript('scripts/update_and_rebuild_libmesh.sh', '--enable-cxx11')
    executeMake("-j"+os.environ['MOOSE_JOBS'], "-C", "framework")
    executeMake("-j"+os.environ['MOOSE_JOBS'], "-C", "test")

if __name__ == '__main__':
    main()
