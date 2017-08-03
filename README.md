# HideProcess

#### Update: Now works for both 64bit and 32bit architecture! Tested on: 

 * Windows 10 Enterprise Edition x64 Build 15063.rs2_release.170317-1834
 * Windows 7 SP1 x86

![Demo](https://github.com/landhb/HideProcess/blob/master/img/demo.PNG?raw=true "Demo")

### Writeup

For more information on the concepts used here please check out my [article](http://www.landhb.me/posts/v9eRa/a-basic-windows-dkom-rootkit-pt-1/).

### Limitations

Although Windows 10 x64 didn't BSOD during testing, this rootkit does not bypass PatchGuard or driver signing requirements so it's always a possibility.

Please use a VM whenever you run this. Current tests on Windows 10 observe no BSOD even a full hour after unlinking the process.

### Compiling The Driver 

The driver has a number of dependencies and you'll need to compile it using msbuild or visual studio. I used Visual Studio during the development process. You'll need:

1. [The Windows 10 SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk)
2. [WDK 10](https://msdn.microsoft.com/en-us/library/windows/hardware/ff557573(v=vs.85).aspx)

Once those are setup and integrated with Visual Studio, start a new empty KMDF (Kernel Mode Driver Framework) project and import the files in the /driver folder. 

Under Debug -> [ProjectName] Properties -> Driver Settings -> General, make sure your Target OS Version is Windows 7 and the Target Platform is Desktop.

Then under Build -> Configuration Manager, make sure the Platform is Win32, and x86 is selected under "Active solution platform".

Now you should be able to use Build -> Build [ProjectName] to build the project. This will generate a .sys file if everything went well. Then put the .sys file in c:\Windows\System32\drivers\[ProjectName].sys, or change the following define statement in loader.c to the path you've specified:

#define DRIVER "c:\\\\Windows\\System32\\drivers\\Rootkit.sys"

#### Compiling The Loader

For the loader you can simply use the makefile and mingw to cross compile it. 

```
sudo apt-get install mingw-w64
```

Then you can create a 32-bit Windows executable using the makefile with:

```
make 32bit
```

And a 64-bit Windows executable with:

```
make 64bit
```
