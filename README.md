# vroom
Virtual Reality OpenGL geometry server.

Vroom is a virtual reality environment where multiple programs can run and share a space in the VR world. The goal is to create a system similar to the client/server model of X11 or Wayland, but instead of exchanging window or 2D buffer information between the client and the server, 3D geometry information is exchanged. The clients load and transfer data objects representing 3D geometry to the server, and the server composes this into a unified VR environment.

The idea is that by centralising the details of VR within a server, prototyping of new 3D UI ideas can be done fast and with minimal repetition of setup code from the client side. The client protocol is implemented using Google Protocol Buffers, and thus clients can be written in many different languages appropriate to the use-case.

The general idea is that clients set up data buffers of geometry, transformation and texture information, and make these available to the server with instructions on how to render the scene.

## Dependencies

* protobuf-c-compiler - Client/Server communication
* libprotobuf-c-dev - Headers required for compiling protobuf C programs
* freeglut3-dev - OpenGL
* libconfig8-dev - HMD configuration file (~/.vroomrc)
* libev-dev - Event handling

  sudo apt-get install protobuf-c-compiler freeglut3-dev libconfig8-dev libev-dev

Follow the instructions for building OpenHMD. They boil down to:

  git clone https://github.com/OpenHMD/OpenHMD.git
  cd OpenHMD/
  sudo apt-get install autoconf libtool
  sudo apt-get install libhidapi-dev
  ./autogen.sh
  ./configure
  make
  sudo make install

You then need to make the user that you are running vroom as able to read input. There are several ways to do this: a) run vroom as root (not recommended), b) select a group your user is already likely to be in (ie: plugdev) and add udev rules that assign that group ownership to the devices, c) add your user to the "input" group (recommended).

For option b) this is an example for the Oculus Rift DK1:

  sudo echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="2833", MODE="0666", GROUP="plugdev"' >> /etc/udev/rules.d/83-hmd.rules
  sudo udevadm control --reload-rules
  sudo udevadm info --query=property --name /dev/hidraw2 | grep DEVPATH
  sudo udevadm test [that DEVPATH]

Which basically means when a usb device is plugged in with a vendor id of 2833, set it's group ownership to "plugdev" and set the device permissions to readable by everyone. Obviously you would need to do this for each device you want to expose to your user. Also I believe the 0666 permission is too open here and actualy negates the group ownership anyway since all users can now read this device.

For option c) this is the course of action:

  sudo gpasswd -a $USER input
  sudo echo 'SUBSYSTEM=="usb", MODE="0660", GROUP="input"' >> /etc/udev/rules.d/99-hid.rules

Then to compile vroom:

  make server
  make client

Run the server:

  ./server

Run the client (will sleep for 10 seconds then exit):

  ./client

## Ok, so what?

The client is connecting to the server using protocol buffers. The client generates data buffers (shared chunks of memory) and sends links to that memory over a socket. That data gets loaded into the GPU 

## Separation of concerns

I believe this is the correct separation of concerns. The server handles stereoscopic rendering, and handling the headset orientation input. This means that every individual client application doesen't need to do that stuff. Imagine if every 2D graphical application had to take full control of the screen, do all it's own keyboard and mouse input handling etc. That's crazy, so instead we have 2D windowing systems where the client application receives keyboard or mouse events (and other events) and knows how to render 2D elements on the server, like pictures and squares. Obviously this is limiting compared to having full control over the screen, but the benefits are obvious.

So I am trying to demonstrate a similar thing but for 3D environments (and VR). At first I wondered about having some sort of scene graph based API, so you would for example send cubes and spheres to the server for rendering, and handle things like collisions server-side, but realised quickly this would be too limiting. So instead the API is fairly close to the bare-bones OpenGL API, in terms of sending data buffers.

In fact perhaps in some ways it would be better to literally emulate the OpenGL API as a drop-in replacement, shaders and all. Or at least the non-immediate mode parts of the OpenGL API.

## TO-DO's

This is completely not finished, but some things I would like to do in the near future:

* There are two separate textures for left and right views. Since each eye is rendered independently, I could just use one and clear it between eye renders, saving some memory - particularly for the Pi GPU.
* Multiple programs can connect to the server and render scenes. Rather than build some sort of organiser into the server, I would rather extend the API to have another program do the organising. Exactly the same concept as a Window Manager in X. Call it a scene manager.
* Build a proper client library, and make bindings for other languages.
* Build a nicer client library, using an external CSG library to do the heavy lifting

## Input handling

I want to be able to plug in any HID device, and use the report descriptor to expose data endpoints that can be bound to matrix transformations. For example pressing and holding the "w" key can be bound to increasing a Z axis (forward motion) transformation and moving the mouse can be bound to some rotation matrix for a pointer or body movement.

First I investigated the hidapi library, however it has no function for extracting the report descriptor. I then tried using libusb and was able to get somewhere, however the raw communication with the device for reports was complicated (async interface). I considered using libusb for extracting the report descriptor, and then initializing hidapi but it was a bit Frankenstein-ish. I found hidraw examples that look clean but are not cross-platform.
