#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>
#include <stdlib.h>
#include "vroom.h"

/**
 * @file vrms_client.h
 * @author Chris Eade
 * @date 19th January 2018
 *
 * @brief Client interface for VRoom server
 *
 * This is the C API for client programs to use the VRoom server. Clients
 * connect to the server via a socket and send and receive messages (which are
 * defined in vroom.proto). The general pattern of usage is:
 *
 *  # Call vrms_connect() to create a client object and establish a connection
 *  # Create a scene object representing a single visual context
 *  # Create a memory object to store data to be shared with the server
 *  # Load geometry and texture data using the memory chunk
 *  # Construct meshes representing linkages between geometry and texture
 *  # Create program data for instructing VRoom how to render the scene
 *  # Initialize a program and run it
 *
 */

typedef struct vrms_client_interface vrms_client_interface_t;

typedef struct vrms_client {
    int32_t socket;
    uint32_t scene_id;
    vrms_client_interface_t* interface;
} vrms_client_t;

typedef struct vrms_client_interface {
    uint32_t (*create_scene)(vrms_client_t* client, char* name);
    uint32_t (*create_memory)(vrms_client_t* client, int32_t fd, uint32_t size);
    uint32_t (*create_object_data)(vrms_client_t* client, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, vrms_data_type_t type);
    uint32_t (*create_object_texture)(vrms_client_t* client, uint32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);
    uint32_t (*run_program)(vrms_client_t* client, uint32_t program_id, uint32_t register_id);
    uint32_t (*set_skybox)(vrms_client_t* client, uint32_t texture_id);
    uint32_t (*destroy_scene)(vrms_client_t* client);
    uint32_t (*destroy_object)(vrms_client_t* client, uint32_t object_id);
} vrms_client_interface_t;

/**
 * @brief Create a new connection to VRoom
 *
 * Creates a new connection to VRoom. This will attempt to make a socket
 * connection to the server and fail if the server is not running. Failure
 * means the pointer returned is NULL.
 *
 * @code{.c}
 * vrms_client_t* client = vrms_connect();
 * @endcode
 * @return Returns an a pointer to a new client object.
 */
vrms_client_t* vrms_connect();

/**
 * @brief Create a new scene
 *
 * A scene is a visual space for the program to create 3D objects in. A program
 * can only create one scene. It exists because it is possible in some cases to
 * interact with the server without a scene.
 *
 * @code{.c}
 * uint32_t scene_id = vrms_client_create_scene(client, "Scene Name");
 * @endcode
 * @return Returns an object_id representing the scene.
 */
uint32_t vrms_client_create_scene(vrms_client_t* client, char* name);

/**
 * @brief Create a shared memory chunk
 *
 * The basis of VRoom is the transfer of chunks of data from the client to the
 * server. This is used for everything from transferring vertex and texture
 * data, to transferring rendering programs and register values for VM
 * initalization.
 *
 * All clients will need to create a chunk of shared memory using this
 * function. The handle returned by this function is then used to tell later
 * functions where they can find the data for that object.
 *
 * @code{.c}
 * uint32_t = (client, );
 * @endcode
 * @return A new object id
 */
uint32_t vrms_client_create_memory(vrms_client_t* client, int32_t fd, uint32_t size);

/**
 * @brief Create a data object
 *
 * This creates an object representing some data within a shared memory chunk.
 * It has three length variables that describe the data in a data object. The
 * memory_length refers to the total length of the data object in bytes. The
 * item_length refers to the length in bytes of a single item in the data
 * object. For a vertex containing three floats this is sizeof(float) * 3. The
 * data_length refers to the length in bytes of a value inside an item. For a
 * vertex this is sizeof(float). If you want to know how many floats make up a
 * vertex it is item_length / data_length. If you want to know how many
 * verticies in the data object it is memory_length / item_length.
 *
 * @code{.c}
 * uint32_t data_id = vrms_client_create_object_data(client, memory_id, memory_offset, memory_length, item_length, data_length, type);
 * @endcode
 * @param memory_id The memory object this data object is in
 * @param memory_offset The offset into this memory object where the data begins in bytes
 * @param memory_length The total length of this data object in bytes
 * @param type
 * @return A new object id
 */
uint32_t vrms_client_create_object_data(vrms_client_t* client, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, vrms_data_type_t type);

/**
 * @brief Create a texture object
 *
 * Creates a new texture object. There are two types of texture object a)
 * regular 2D textures which are intended to be combined with a geometry object
 * and UV mappings for a textured mesh and b) Cube map textures intended to be
 * combined with a skybox object.
 *
 * For cube map the width and height should be the same value, and represent a
 * square image of one of the sides of the cube. All 6 cube images should be
 * loaded into memory in the order XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG.
 *
 * @code{.c}
 * uint32_t texture_id = vrms_client_create_object_texture(client, data_id, width, height, format, type);
 * @endcode
 * @param data_id A data object containing a texture
 * @param width The width of the texture
 * @param height The height of the texture
 * @param format The pixel format
 * @param type What type of texture (2D or Cube map)
 * @return A new object id
 */
uint32_t vrms_client_create_object_texture(vrms_client_t* client, uint32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);

/**
 * @brief Run a program
 *
 * This does two things: a) loads the program referred to by program_id into
 * the scene and makes it the currently running program and b) sets the
 * registers for the VM according to an array of 8 32 bit integer values
 * located in a memory object at the offset and length supplied.
 *
 * The registers can be used to set object ids for meshes or memory ids for
 * lists of objects that can be referred to by indicies.
 *
 * @code{.c}
 * uint32_t ok = vrms_client_run_program(client, program_id, register_id);
 * @endcode
 * @param program_id Object id of a program to be run
 * @param register_id A data object containing regester initialization values
 * @return A status
 */
uint32_t vrms_client_run_program(vrms_client_t* client, uint32_t program_id, uint32_t register_id);

/**
 * @brief Set the skybox
 *
 * @code{.c}
 * uint32_t ok = vrms_client_set_skybox(client, texture_id);
 * @endcode
 * @param texture_id Object id of a cubemap texture
 * @return A status
 */
uint32_t vrms_client_set_skybox(vrms_client_t* client, uint32_t texture_id);

/**
 * @brief Destroy an object
 *
 * Destroying an object destroys that object on the server, freeing up GPU
 * memory (if the object resides there)
 *
 * @code{.c}
 * uint32_t ok = vrms_client_destroy_object(client, object_id);
 * @endcode
 * @param object_id Object id of a program to be run
 * @return Returns 1 on success and 0 on failure
 */
uint32_t vrms_client_destroy_object(vrms_client_t* client, uint32_t object_id);

/**
 * @brief Destroy a scene
 *
 * Destroying a scene destroys all objects within the scene. This includes any
 * memory objects, meaning probably shared memory should be freed after calling
 * this.
 *
 * @code{.c}
 * uint32_t ok = vrms_client_destroy_scene(client);
 * @endcode
 * @return Returns 1 on success and 0 on failure
 */
uint32_t vrms_client_destroy_scene(vrms_client_t* client);

#endif
