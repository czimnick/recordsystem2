||windows|mac os x|ubuntu|
|:--:|:--:|:--:|:--:|
|**master**|[![Build status](https://ci.appveyor.com/api/projects/status/u1pewm7ewm4muepj?svg=true)](https://ci.appveyor.com/project/czimnick/recordsystem2)|[![Build Status](https://travis-ci.org/q3df/recordsystem2.svg?branch=master)](https://travis-ci.org/q3df/recordsystem2)|[![Build Status](https://travis-ci.org/q3df/recordsystem2.svg?branch=master)](https://travis-ci.org/q3df/recordsystem2)|

Dependency
----------
> #### Linux (ubuntu 16.04 x64 tested)
> ```
> dpkg --add-architecture i386
> apt-get update
> apt-get install build-essential gcc-multilib p7zip-full g++-multilib libboost-dev:i386 libboost-program-options-dev:i386 python p7zip p7zip-full
> ```
> #### Mac OS X 10.9.2
> !currently not supported!
> ```
> xcode 5.1 and console utils have to installed on the system
> ```
> #### Windows
> ```
> all dependencies are includes in the repository and will be build
> ```

Build (command line)
--------------------
> ```
> git clone https://gitlab.q3df.org/megadaddeln/recordsystem2.git
> cd recordsystem2
> git submodule update --init --recursive
> ```
> 
> > #### Windows (in vs2010 cmd shell)
> > ```
> > create_projects.bat
> > build_projects.bat <Debug|Release>
> > ```
> > #### Linux (bash shell)
> > ```
> > ./create_projects.sh
> > ./build_projects.sh <Debug|Release>
> > ```
> > #### Mac OS X 10.9.2 (bash shell)
> > ```
> > ./create_projects.sh
> > ./build_projects.sh <Debug|Release>
> > # or you can open the xcode project and build from there
> > ```

Build (windows vs2010 / vs2012)
-------------------------------
> ```
> git clone https://gitlab.q3df.org/megadaddeln/recordsystem2.git
> cd recordsystem2
> git submodule update --init
> create_projects.bat
> ```
> now you can open the generated recordsystem.sln solution!>


Installation
-------------------------------
> > ### Windows (console)
> > ```
> > # DEAMON
> > cd <git-repo>/build/<Debug|Releas>/
> > mklink qagamex86.dll current_client_version.dll # for proxymod self update
> > ./recordsystemd.exe
> > # after starting you have to set the apikey (debug currently)
> > # type following command into the daemon console:
> > set apikey "secretkey"
> >
> > # IOQ3DED PROXYMOD
> > cd <q3-directory-root>
> > mkdir defrag\q3df_proxymod
> > xcopy <git-repo>\build\<Debug|Release>\qagamex86.dll defrag/q3df_proxymod/qagamex86.dll
> > mklink defrag/q3df_proxymod/qagamex86.so defrag/qagamex86.dll
> > ioq3ded.x86.exe +set fs_game defrag +set vm_game 0 +set rs_api_key "secretkey" +map st1
> > ```
> >
> > ### Linux (ubuntu 12.04 x64)
> > ```
> > # DEAMON
> > cd <git-repo>/out/<Debug|Releas>/
> > ln -s lib.target/libqagamei386.so current_client_version.so # for proxymod self update
> > ./recordsystemd
> > # after starting you have to set the apikey (debug currently)
> > # type following command into the daemon console:
> > set apikey "secretkey"
> > 
> > # IOQ3DED PROXYMOD
> > cd <q3-directory-root>
> > mkdir defrag/q3df_proxymod
> > cp <git-repo>/out/<Debug|Release>/lib.target/libqagamex86.so defrag/q3df_proxymod/qagamei386.so
> > ln -s defrag/q3df_proxymod/qagamei386.so defrag/qagamei386.so
> > ./ioq3ded.i386 +set fs_game defrag +set vm_game 0 +set rs_api_key "secretkey" +map st1
> > ```
