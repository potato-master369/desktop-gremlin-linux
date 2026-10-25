#!/usr/bin/python3
# desktop-gremlin-linux dlpm
#--------------------------------------------
# Recipe manager for downloading gremlins
#--------------------------------------------

#Copyright <YEAR> <COPYRIGHT HOLDER>
#
# Permission is hereby granted, free of 
# charge, to any person obtaining a copy of 
# this software and associated documentation 
# files (the “Software”), to deal in the 
# Software without restriction, including 
# without limitation the rights to use, copy, 
# modify, merge, publish, distribute, sublicense, 
# and/or sell copies of the Software, and to 
# permit persons to whom the Software is 
# furnished to do so, subject to the 
# following conditions:
#
# The above copyright notice and this permission 
# notice shall be included in all copies or 
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT 
# WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT 
# SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
# IN CONNECTION WITH THE SOFTWARE OR THE USE 
# OR OTHER DEALINGS IN THE SOFTWARE.

import sys
import os
import stat
import getopt
import shutil
import subprocess

verbose = 0
ask = 0
force = 0

def usage() -> None:
    print("Usage: dlpm [ACTION] [OPTIONS] <PKGLIST>")
    print("\nACTIONS:")
    print("\tinstall (also works as upgrade should the recipe be changed)")
    print("\tremove")
    print("\tinit (do not supply PKGLIST>")
    print("\tlist (supply PKGLIST as either @world (installed packages) or @all (all packages))")
    exit(1)

def dlpm_init() -> int:
    world_path = "/usr/share/desktop-gremlin-linux/dlpm/world"
    init_dlpm = input(" -> Initialize DLPM? [Y/n] ")
    if init_dlpm.lower() == "y":
        print("    -> Start init of DLPM")
        print("     -> Create DLPM folder")
        try:
            os.makedirs("/usr/share/desktop-gremlin-linux/dlpm/", exist_ok=True)
        except PermissionError:
            print("     !!! PERMISSION DENIED\n\nAre you running as root?")
            return -2
        except OSError as e:
            print(f"     !!! UNKNOWN OS ERROR: {e}")
            return -2

        print("     -> Create DLPM world file")
        try:
            with open(world_path, "a") as x:
                pass
        except PermissionError:
            print("     !!! PERMISSION DENIED\n\nAre you running as root?")
            return -2
        except OSError as e:
            print(f"     !!! UNKNOWN OS ERROR: {e}")
            return -2

        print("     -> Copy recipe folder")
        recipepath = input("     -> Recipe folder path [./recipes]: ")
        if not recipepath:
            recipepath = "./recipes"

        try:
            with open(os.path.join(recipepath, "timestamp")) as x:
                pass
        except FileNotFoundError:
            print("     !!! This is not a valid recipe folder! The timestamp file was not found!")
            return -2
        except OSError as e:
            print(f"     !!! UNKNOWN OS ERROR: {e}")
            return -2

        shutil.copytree(recipepath, "/usr/share/desktop-gremlin-linux/dlpm/recipes", dirs_exist_ok=True)
        print("     -> Init complete. Re-run your command.")
        return 4
    else:
        print("     !!! INIT REJECTED")
        return 12

def dlpm_install(toinstall: str) -> int:
    print(f" -> Installing package {toinstall}")
    world_path = "/usr/share/desktop-gremlin-linux/dlpm/world"
    
    try:
        if os.path.exists(world_path):
            print("    -> Reading world file")
            with open(world_path, "r") as f:
                for line in f:
                    if line.strip() == toinstall and force == 0:
                        print("     -> The file is currently already installed.")
                        disregardcinstall = input("Re-install anyway? [Y/N] ")
                        if disregardcinstall.lower() != "y":
                            print("     -> Quitting.")
                            return -2

    except Exception as e:
        print(f" !!! Error reading world file: {e}")

    if not os.path.exists(world_path):
        print(f" !!! The world file was not found! ({world_path})")
        init_dlpm = input(" -> Initialize DLPM? [Y/n] ")
        if init_dlpm.lower() == "y":
            print("    -> Start init of DLPM")
            print("     -> Create DLPM folder")
            try:
                os.makedirs("/usr/share/desktop-gremlin-linux/dlpm/", exist_ok=True)
            except PermissionError:
                print("     !!! PERMISSION DENIED\n\nAre you running as root?")
                return -2
            except OSError as e:
                print(f"     !!! UNKNOWN OS ERROR: {e}")
                return -2

            print("     -> Create DLPM world file")
            try:
                with open(world_path, "a") as x:
                    pass
            except PermissionError:
                print("     !!! PERMISSION DENIED\n\nAre you running as root?")
                return -2
            except OSError as e:
                print(f"     !!! UNKNOWN OS ERROR: {e}")
                return -2

            print("     -> Copy recipe folder")
            recipepath = input("     -> Recipe folder path [./recipes]: ")
            if not recipepath:
                recipepath = "./recipes"

            try:
                with open(os.path.join(recipepath, "timestamp")) as x:
                    pass
            except FileNotFoundError:
                print("     !!! This is not a valid recipe folder! The timestamp file was not found!")
                return -2
            except OSError as e:
                print(f"     !!! UNKNOWN OS ERROR: {e}")
                return -2

            shutil.copytree(recipepath, "/usr/share/desktop-gremlin-linux/dlpm/recipes", dirs_exist_ok=True)
            print("     -> Init complete. Re-run your command.")
            return 4
        else:
            print("     !!! INIT REJECTED")
            return 12

    print(" -> Install")
    print(" -> Calling script")
    install_script_path = f"/usr/share/desktop-gremlin-linux/dlpm/recipes/{toinstall}/install"
    
    if verbose > 0:
        print(f" [ v ] Called script is {install_script_path}")
        
    try:
        cm = os.stat(install_script_path).st_mode
        os.chmod(install_script_path, cm | stat.S_IXUSR)
        
        result = subprocess.run(["sh", install_script_path], text=True, check=False)

        if result.returncode != 0:
            print(f" !!! script returned non-zero return value: {result.returncode}")
            return result.returncode
            
    except Exception as e:
        print(f" !!! An error occurred running the script: {e}")
        return -2

    try:
        installed_packages = []
        if os.path.exists(world_path):
            with open(world_path, "r") as f:
                installed_packages = [line.strip() for line in f if line.strip()]

        if toinstall in installed_packages:
            installed_packages.remove(toinstall)

        installed_packages.append(toinstall)

        with open(world_path, "w") as f:
            for pkg in installed_packages:
                f.write(f"{pkg}\n")
                
    except Exception as e:
        print(f" !!! Failed to update world file: {e}")

    print(" [ * ] Install finish!")
    return 0

def dlpm_remove(toinstall: str) -> int:
    print(f" -> Removing package {toinstall}")
    world_path = "/usr/share/desktop-gremlin-linux/dlpm/world"
    foundpkg = 0
    
    try:
        if os.path.exists(world_path):
            print("    -> Reading world file")
            with open(world_path, "r") as f:
                for line in f:
                    if line.strip() == toinstall and force == 0:
                        print("     -> The file is currently already installed.")
                        foundpkg = 1
            if foundpkg == 0:
                print(f"     -> Package {toinstall} is not installed.")
                return 67

    except Exception as e:
        print(f" !!! Error reading world file: {e}")

    if not os.path.exists(world_path):
        print(f" !!! The world file was not found! ({world_path})")
        init_dlpm = input(" -> Initialize DLPM? [Y/n] ")
        if init_dlpm.lower() == "y":
            print("    -> Start init of DLPM")
            print("     -> Create DLPM folder")
            try:
                os.makedirs("/usr/share/desktop-gremlin-linux/dlpm/", exist_ok=True)
            except PermissionError:
                print("     !!! PERMISSION DENIED\n\nAre you running as root?")
                return -2
            except OSError as e:
                print(f"     !!! UNKNOWN OS ERROR: {e}")
                return -2

            print("     -> Create DLPM world file")
            try:
                with open(world_path, "a") as x:
                    pass
            except PermissionError:
                print("     !!! PERMISSION DENIED\n\nAre you running as root?")
                return -2
            except OSError as e:
                print(f"     !!! UNKNOWN OS ERROR: {e}")
                return -2

            print("     -> Copy recipe folder")
            recipepath = input("     -> Recipe folder path [./recipes]: ")
            if not recipepath:
                recipepath = "./recipes"

            try:
                with open(os.path.join(recipepath, "timestamp")) as x:
                    pass
            except FileNotFoundError:
                print("     !!! This is not a valid recipe folder! The timestamp file was not found!")
                return -2
            except OSError as e:
                print(f"     !!! UNKNOWN OS ERROR: {e}")
                return -2

            shutil.copytree(recipepath, "/usr/share/desktop-gremlin-linux/dlpm/recipes", dirs_exist_ok=True)
            print("     -> Init complete. Re-run your command.")
            return 4
        else:
            print("     !!! INIT REJECTED")
            return 12

    print(" -> Remove")
    print(" -> Calling script")
    install_script_path = f"/usr/share/desktop-gremlin-linux/dlpm/recipes/{toinstall}/remove"
    
    if verbose > 0:
        print(f" [ v ] Called script is {install_script_path}")
        
    try:
        cm = os.stat(install_script_path).st_mode
        os.chmod(install_script_path, cm | stat.S_IXUSR)
        
        result = subprocess.run(["sh", install_script_path], text=True, check=False)

        if result.returncode != 0:
            print(f" !!! script returned non-zero return value: {result.returncode}")
            return result.returncode
            
    except Exception as e:
        print(f" !!! An error occurred running the script: {e}")
        return -2

    try:
        installed_packages = []
        if os.path.exists(world_path):
            with open(world_path, "r") as f:
                installed_packages = [line.strip() for line in f if line.strip()]

        if toinstall in installed_packages:
            installed_packages.remove(toinstall)


        with open(world_path, "w") as f:
            for pkg in installed_packages:
                f.write(f"{pkg}\n")
                
    except Exception as e:
        print(f" !!! Failed to update world file: {e}")

    print(" [ * ] Removal finish!")
    return 0

if __name__ == "__main__":
    print(" desktop-gremlin-linux DownLoad Package Manager")
    print("------------------------------------------------\n")
    if len(sys.argv) > 2:
        action = sys.argv[1]
        if action == "install":
            # install
            argv = sys.argv[2:]
            try:
                arguments, values = getopt.getopt(argv, "vfa", [])
            except getopt.error as err:
                print(str(err))
                usage()
                # NOTREACHED
            for ca, cv in arguments:
                if ca == "-v":
                    print(" -> verbose on")
                    verbose += 1
                elif ca == "-a":
                    if verbose > 0:
                        print(" -> will ask for confirm")
                    ask = 1
                elif ca == "-f":
                    if verbose > 0:
                        print(" -> will force install")
                    force = 1
                else:
                    usage()
                    # NOTREACHED
            for toinstall in values: # Parse the remaining non-option arguments
                # Install it
                res = dlpm_install(toinstall)
                if res != 0:
                    print(f" !!! ERROR on dlpm_install: {res}")
                    exit(1)
            exit(0)
        if action == "remove":
            # remove
            argv = sys.argv[2:]
            try:
                arguments, values = getopt.getopt(argv, "vfa", [])
            except getopt.error as err:
                print(str(err))
                usage()
                # NOTREACHED
            for ca, cv in arguments:
                if ca == "-v":
                    print(" -> verbose on")
                    verbose += 1
                elif ca == "-a":
                    if verbose > 0:
                        print(" -> will ask for confirm")
                    ask = 1
                elif ca == "-f":
                    if verbose > 0:
                        print(" -> will force removal")
                    force = 1
                else:
                    usage()
                    # NOTREACHED
            for toinstall in values: # Parse the remaining non-option arguments
                # Install it
                result = dlpm_remove(toinstall)
                if result != 0 and result != 67:
                    print(f" !!! ERROR REMOVING: {result}")
            exit(0)
        else:
            print("Unknown action:", action)
            usage()
            # NOTREACHED
    elif len(sys.argv) == 2:
        if sys.argv[1] == "init":
            dlpm_init()
        else:
            usage()
            # NOTREACHED
    else:
        usage()
        # NOTREACHED
