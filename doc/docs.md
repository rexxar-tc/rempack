# Docs

This document outlines the general theory of operation for building applications for the ReMarkable 2 tablet. This document also includes details for configuring CLion to work natively with this cross-platform toolchain through a Docker build agent.

Naturally, this guide is focused on my particular build setup. However, the general concepts should apply to your situation. There's also quite a few things that assume you're using an RM2 tablet, but again it's mostly applicable to other devices.

Of particular note, everything in this repo assumes you're using an SSH connection over wifi. You can also do this over the USB connection, you'll just need to adjust the SSH target, see below. This is mostly a consequence of my particular setup, but more on that later.

### Warning:

If you use a device other than an RM2, proceed with caution. I've made my best effort to note where things are RM2-specific, but I can't claim I've covered everything. Use your judgement and don't blindly run commands and scripts without understanding what they do. I don't have a device other than the RM2, therefore I cannot guarantee anything at all will work for other devices.

## General theory

This repo is based upon the `rmkit` framework. This has brought with it a few idiosyncrasies, most notably requiring Python and the custom `CPY` preprocessor (I think). Many of the scripts in this repo, including a majority of the CMake files are centered around rmkit. It largely doesn't influence the build process itself, but I'll make note of anything specific.

The RM tablets use an Arm processor. Therefore, when building with an x86 system, we must use a cross-compiling toolchain. You *can* do this natively on your host machine by installing the toolchains manually, but this is **not** recommended, and I won't go into the details here.

Toltec provides a set of Docker images pre-configured with everything required to cross-compile for these tablets. This is the recommended approach, and generally a lot more reliable. Besides which, your project must be buildable under a Toltec container if you want to list your program in their opkg repo (and you do).

The build process itself is actually pretty straightforward. CMake will invoke Docker, which does the actual build and stuffs the output files into a set directory. From there you just upload to the tablet and run your program (this is the complicated bit).

This build system is managed through a mix of CMake and bash scripts. The complicated part was putting together these scripts and figuring out the data flow. Fortunately for you, that hard work is already done, and you are encouraged to steal any of these scripts that you find useful.

You can, of course, use these Docker images the hard way. You can invoke Docker manually to run `make` or `cmake` or whatever build process you want. This is how the CMakeLists.txt in this project works under the hood. See that file for more information.

In general, you *can* build your project in x86 mode for local debugging. There's a couple of options for emulating the tablet on an x86 machine for this purpose, but I haven't spent the time or effort to get that working well. See `CMakePresets.json` and `rm-toolchain.cmake`.

While the build scripts and configurations in this repo do include x86 and local native cross-compilation, I do not recommend using them. x86 mode *may* be useful as a way to just check if your code compiles without invoking Docker, but the produced binaries aren't very useful without an emulator. Local cross-compilation technically does work, but can get extremely messy. You need several system-wide packages to support compilation, as well as downloading some toolchain files from ReMarkable's website which aren't exactly public. It is strongly recommended to use the Toltec images in all circumstances.

# rmkit

rmkit is a bit of an odd duck. It's written in a custom dialect that is somewhere between Python and C++. Literally it is C++ with Python syntax, which uses a custom python preprocessor `cpy` to translate back to C++ before build. It also emits a single gargantuan header file which contains the rmkit framework and all of its dependencies.

If you wish to inspect the rmkit code, I very much do not recommend opening this header. It's too large for any IDE to do anything useful with. Instead, pull down the source code from their github. If you don't want to read the `cpy` syntax, there's a command you can run to produce C++ header files. I'll document this later once I find the command again.

rmkit (and its dependencies) require a few flags to be set. This is normally done through `#define` statements before the first `#include <rmkit.h>`, but I've opted to have CMake manage that to reduce C-style clutter at the top of my files. Totally optional, see the comments in `CMakeLists.txt` for more information.

There are a few more rmkit-specific implementation details here, but it's all documented in `CMakeLists.txt`.

Additionally, because rmkit is a single-header library, this has some quite severe consequences in how you must structure your code. **ALL** of your code which relies on rmkit types **MUST** be in the same Translation Unit. see: [Microsoft](https://learn.microsoft.com/en-us/cpp/cpp/program-and-linkage-cpp?view=msvc-170), [Wikipedia](https://en.wikipedia.org/wiki/Translation_unit_(programming))

Essentially, this means you can't have multiple independent classes which include `rmkit.h`. All dependent code must be inlined into a single class, which is why almost everything in this repo is defined in header files. Due to this, build times can get quite excessive as you must recompile *everything* every time. `ccache` cannot save you, you simply must recompile the entirety of rmkit every time you make a change. I've tried many workarounds and have come up empty each time. This is primarily a limitation of C++ single-header libraries in general, but rmkit's particular implementation makes things more difficult.

# Docker

If using CLion, see the specific instructions in the next section.

Basically, just run `docker run -i -v /path/to/project/root:/mnt/project --pull missing -w /mnt/project ghcr.io/toltec-dev/toolchain:v3.1 <some command>`
This mounts your project to `/mnt/project` inside the container, then you can just work out of there. `-w` is for `w`orking directory. 

I don't recommend running `make` or `cmake` without specifying a new output folder. If you just blindly make in the root, you'll clobber a bunch of file permissions from the docker user's UID. Or something like that anyway. Just set your output directory to something like `/mnt/project/docker-build` and it'll be fine. 
 
The Toltec images are from https://github.com/toltec-dev/toolchain

There are multiple images with different tool sets preloaded. We're mainly concerned with `toolchain` and `python`. See the readme at the github repo for a complete list.

rmkit needs the `python` flavor, and you have to `pip install okp` before building.

CMake was a CMistake.

# CLion

CLion natively supports docker build agents. But first, a note about build configurations:

When you load this repo, CLion will pull in `CMakePresets.json` and pre-populate all the defined build configurations. They all pretty much do what they say on the tin: debug and release builds in x86 mode, native cross-compile on the host machine, or cross-compile with Docker. Unless you have a very specific reason, I recommend disabling the x86 and native (non-docker) modes in all situations. They're listed mainly for completeness.

You may need to come back to the build configuration menu and set the toolchain to `Docker` once you have that set up:

First, pull down the docker image you want: `docker pull ghcr.io/toltec-dev/python:v3.1`

This project requires python as a dependency of rmkit. Check the Toltec repo for a list of their other images.

Open Settings and navigate to `Build, Execution, Deployment -> Toolchains`. Add a new toolchain and select Docker. Select the image you just downloaded and leave the rest of the settings alone. You'll see a few errors related to the debugger, but you can ignore it.

From here you can run the build and the output files will be at `cmake-build-*-native-docker`. You can use `run_app.sh path/to/executable` to automatically upload the binary to your tablet and start the program. Be aware that doing this will kill xochitl and remux (if installed) and you'll have to manually restart those services to restore the tablet to normal operation. This could probably be automated but SSH is hard.

For tighter integration and runtime debugging, I use a remote GDB run target with some scripts that allows CLion to directly upload binaries and control execution on the tablet. The system also uploads a GDB server for realtime debugging and breakpoints. 

**WARNING:** breakpoints don't always work as expected, and will frequently fail to link the executing code to your source. All you'll see is raw disassembled binary. I'm not sure exactly what the problem is, but my workaround has been to insert a very long delay (~30s) into program startup. If I manually suspend execution during that delay, GDB usually figures out what's happening and I can debug normally. From there, I can do normal debugging tasks like setting breakpoints on the fly and catching exceptions. Though sometimes that still doesn't work, or after some time debugging GDB will forget what it's doing and go back to giving you raw disassembly. Honestly I have not even the slightest clue why this happens, it's all very arcane.

To set this up, edit your run configurations (in the dropdown next to the build and run buttons) then add a new `Remote GDB Server` target. Ensure the target and executable fields point to the project you wish to debug. In the `Credentials` field, set up your SSH connection. **Do not** set your password in here. Use SSH keys instead. Set the upload path to `/tmp/CLion/debug`, in `target remote args` you need to specify the hostname and port for the GDB server. `remarkable:1234` should work for most configurations. Leave the GDB server field as default, then in `GDB Server Args` you should set `--wrapper /opt/bin/rm2fb-client -- :1234 /tmp/CLion/debug/your_executable_name_here`. If you're using a tablet other than the RM2, you'll need to adjust this field. Most likely you'll need to delete everything before `:1234` but I can't test this. Finally, in the `before launch` section, you need to add an external tool. Click the `+` and choose external tool. For the program, select the `setup_debugger.sh` script.

Now, in your run target dropdown, ensure you have the Docker build target selected, as well as your new run target. Then you can just hit the debug button and your program should magically appear on your tablet. Any sort of `printf` calls from your program will appear in the debug console in CLion.

Again, this setup is rather brittle and may require tweaking on your part. I've tried to document the steps that work for me, and that's the best I can do for you.

As for the peculiarities of my setup, I use Jetbrains Gateway to run CLion remotely on my build server. rmkit takes a bit of grunt to build, and that server is the most powerful machine I have. Due to this, I can't realistically use USB to connect to my tablet, so I use SSH over wifi instead. The wifi performance of the RM2 is *very* poor, under 1Mbps as I recall. Use USB if you can.

Also worth mentioning: CLion also supports *remote* docker build agents. You could theoretically run a docker server on a remote machine and use the same workflow as above. I haven't tried this, my Docker-fu is not strong enough.

# SSH

The first part of this section is specific to the ReMarkable series. Please research your device and follow the recommended steps, if any.

It is **imperative** that you set up SSH keys. If you don't, you risk **permanently** losing access to, or outright bricking your device. Under some situations, the SSH password may be regenerated and the password displayed to you in xochitl **will be incorrect!** If this happens, your only recourse involves external hardware. Therefore, you **must** set up an SSH key. Your key will persist on the device and will always allow you back in if the worst happens.

See [this guide](https://remarkable.guide/guide/access/ssh.html) for detailed instructions on generating and installing your SSH key.

Most of the scripts in this repo provide a way to override the SSH target, which can allow you to use USB, or other non-standard network configurations. These scripts **DO NOT** offer any way to provide a password for SSH. You don't need that, as you have a key installed.

To override the SSH target for any script, you can invoke them with an environment variable: `REMARKABLE_HOST=192.168.0.123 run_app.sh cmake-build-debug-native-docker/example_file`.

You may also override the user name. By default, all scripts attempt to connect to `root@remarkable`. By setting the `RM_USER` variable, you can change this to run under a different account. Make sure that this user exists on your tablet, you have an SSH key installed for that account, and that the account has all required permissions. You'll know if you need to change this, otherwise leave it alone.

Again, all the scripts here assume you can connect to SSH at `root@remarkable` on port 22. The RM2 has a hostname of `remarkable` and most (all?) modern-ish routers should be able to translate that to an IP address. Notably, this does not work for USB, which is why the overrides exist. If this does not work, and attempting to `ping remarkable` returns a 'host not found' or similar error, you'll need to specify your IP address manually as above. The address can be found in the network settings on your tablet.

Also, ensure that your tablet is turned on and connected to wifi. They tend to shut down the wireless connection if not used, and obviously won't connect while in sleep mode.

# Build server

rmkit has an automated build server that posts compiled binaries, including stb.arm.o

This isn't enough. We have the full rmkit.h file, but we're missing two things: font_embed.h and stb.x86.o

font_embed is a free font serialized to a byte array with some metadata. It's in the rmkit repo at `src/rmkit/`. You have to `#define FONT_EMBED_H "rel/path/to/font_embed.h"` somewhere to tell rmkit where to look. The path can be excluded if CMake includes the path for you.

The stb library is an image parser of some description. It's available as source, but rmkit optimized their build system by compiling stb once and sharing the binary with all projects. So we need the .o file to build against. This is available at `src/vendor/stb` and we can compile it with the normal tools to target x86.

So, we need to grab the release of `rmkit.h` from the server, and also clone the git repo so we can build `stb` and to also nab `font_embed.h`. Honestly might just copy or recreate the font and distribute it directly. Would save a step.