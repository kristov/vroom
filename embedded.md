# Choosing an embedded language

## Options

I want the options to be mature and widely used already for stability. Both Lua and Guile immediately stood out as good options in this regard. Lua has the advantage of being associated with the gaming scene already. I also liked Duktape as Javascript is a widely known language.

* Lua
* Guile
* Chicken Scheme
* Duktape

## Requirements

### Give users the ability to generate geometry, textures and shaders

In theory an entire program could be written and shipped to the server without loading any assets from the client over to the server. This would allow full flexibility on how people choose to write their programs.

### Heavily restrict the use of external libraries/modules

I do not want users to be able to load a network module and fire up a web server for example. I know it restricts freedom but it would be a security nightmare. The server should not become the dumping ground for all applications, only the place where user interface code is executed.

### Do not allow loading anything from the filesystem

I do not want any client to be able to load a program on the server that uses server permissions to access the filesystem in any way. This includes importing modules from the filesystem. And especially linking to C libraries.

### Execute the scripting environent step-by-step

If the scripting environment does not have this feature, and infinite loop in a client program could completely lock up rendering and user input. It also would not allow for frame rendering code to be stopped if it would drop the frame rate below an acceptable amount.

## Lua

I do not like the idea of mapping tightly packed float or integer data (geometries, indicies, normals and textures) to Lua's `table` type ([Tables are the only "container" type in Lua.](http://lua-users.org/wiki/TablesTutorial)) from C, as it looks like there would be an expensive conversion operation when shifting data back and forward from Lua-land. Perhaps there is a way to expose some opaque reference to the data and restrict usage via an interface in Lua somehow. However I like the idea of being able to just write bytes into an array without going through some clunky interface.

It looks possible to sandbox Lua by not allowing anything and then whitelisting functionality. See [here](http://lua-users.org/wiki/SandBoxes).

[Arcan](https://arcan-fe.com) uses Lua, and Arcan rocks.

## Guile

I like that Guile has support for numeric vector types that seem to fit nicely into the type of data being manipulated (u8, s8, u16, s16, u32, s32, u64, s64, f32, f64). I could not find any information on restricting modules in Guile except [this unanswered question](https://stackoverflow.com/questions/54640307/sandboxing-guile-by-deleting-unwanted-libraries). For example to make it impossible to use threads from Guile - this worries me.

## Duktape

Seems possible to sandbox it [fairly well](https://github.com/svaarala/duktape/blob/master/doc/sandboxing.rst). While there are no fancy vector types a plain Javascript array maps ok-ish to arrays of ints and floats.

## SGScript

Has some interesting "why?" [answers](http://www.sgscript.org/docs/sgscript.docs/why-sgscript). For example:

* Game math library ("A library with vector/matrix objects")
* Native arrays

Major con: there is zero information on the web about it.

