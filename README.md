# vroom
Virtual Reality OpenGL geometry server.

Vroom is a virtual reality environment where multiple programs can run and share a space in the VR world. The goal is to create a system similar to the client/server model of X11 or Wayland, but instead of exchanging window or 2D buffer information between the client and the server, 3D geometry information is exchanged. The clients load and transfer data objects representing 3D geometry to the server, and the server composes this into a unified VR environment.

The idea is that by centralising the details of VR within a server, prototyping of new 3D UI ideas can be done fast and with minimal repetition of setup code from the client side. The client protocol is implemented using Google Protocol Buffers, and thus clients can be written in many different languages appropriate to the use-case.

The general idea is that clients set up data buffers of geometry, transformation and texture information, and make these available to the server with instructions on how to render the scene.
