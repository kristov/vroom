#include <stdint.h>
#include <stdlib.h>
#include "vrms.h"

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

typedef struct vrms_client {
    int32_t socket;
    uint32_t scene_id;
} vrms_client_t;

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
 * uint32_t scene_id = vrms_create_scene(client, "Scene Name");
 * @endcode
 * @return Returns an object_id representing the scene.
 */
uint32_t vrms_create_scene(vrms_client_t* client, char* name);

/**
 * @brief Destroy a scene
 *
 * Destroying a scene destroys all objects within the scene. This includes any
 * memory objects, meaning probably shared memory should be freed after calling
 * this.
 *
 * @code{.c}
 * uint32_t ok = vrms_destroy_scene(client);
 * @endcode
 * @return Returns 1 on success and 0 on failure
 */
uint32_t vrms_destroy_scene(vrms_client_t* client);

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
uint32_t vrms_client_create_memory(vrms_client_t* client, uint8_t** address, size_t size);

/**
 * @brief Create a data object
 *
 * This creates an object representing some data within a shared memory chunk.
 *
 * @code{.c}
 * uint32_t data_id = vrms_client_create_data_object(client, memory_id, memory_offset, memory_length, item_length, data_lengthi, type);
 * @endcode
 * @param memory_id
 * @param memory_offset
 * @param memory_length
 * @param item_length
 * @param data_length
 * @param type
 * @return A new object id
 */
uint32_t vrms_client_create_data_object(vrms_client_t* client, int32_t memory_id, uint32_t memory_offset, uint32_t memory_length, uint16_t item_length, uint16_t data_length, vrms_data_type_t type);

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
 * uint32_t texture_id = vrms_client_create_texture_object(client, data_id, width, height, format, type);
 * @endcode
 * @param data_id A data object containing a texture
 * @param width The width of the texture
 * @param height The height of the texture
 * @param format The pixel format
 * @param type What type of texture (2D or Cube map)
 * @return A new object id
 */
uint32_t vrms_client_create_texture_object(vrms_client_t* client, int32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);

/**
 * @brief Create a new geometry object
 *
 * Creates a new geometry object from the supplied vertex, normal and index
 * data. This defines the physical structure of an object but it is not
 * renderable itself and needs to be attached to a mesh before that mesh object
 * is rendered.
 *
 * @code{.c}
 * uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);
 * @endcode
 * @param vertex_id A data object containing vertex data
 * @param normal_id A data object containing normals
 * @param index_id A data object containing index values
 * @return A new object id
 */
uint32_t vrms_client_create_geometry_object(vrms_client_t* client, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id);

/**
 * @brief Create a colored mesh
 *
 * Creates a mesh from the supplied geometry object with a solid color of RGBA.
 *
 * @code{.c}
 * uint32_t = vrms_client_create_mesh_color(client, geometry_id, r, g, b, a);
 * @endcode
 * @param geometry_id The object id of a geometry
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 * @param a Alpha component
 * @return A new object id
 */
uint32_t vrms_client_create_mesh_color(vrms_client_t* client, uint32_t geometry_id, float r, float g, float b, float a);

/**
 * @brief Create a textured mesh
 *
 * Creates a mesh from the supplied geometry object, texture object and UV
 * mapping data. A geometry object must be created first containing the vertex
 * and index data.
 *
 * @code{.c}
 * uint32_t mesh_id = vrms_client_create_mesh_texture(client, geometry_id, texture_id, uv_id);
 * @endcode
 * @param geometry_id The object id of a geometry
 * @param texture_id The object id of a texture
 * @param uv_id A data object containing UV mapping coordinates
 * @return A new object id
 */
uint32_t vrms_client_create_mesh_texture(vrms_client_t* client, uint32_t geometry_id, uint32_t texture_id, uint32_t uv_id);

/**
 * @brief Create a new program
 *
 * Loads a program from a memory object.
 *
 * @code{.c}
 * uint32_t program_id = vrms_client_create_program(client, data_id);
 * @endcode
 * @param data_id
 * @return A new object id
 */
uint32_t vrms_client_create_program_object(vrms_client_t* client, uint32_t data_id);

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
 * @brief Update a system matrix
 *
 * Used currently by the openhmd input driver as a way to set the view matrix
 * from a client.
 *
 * @code{.c}
 * uint32_t ok = vrms_client_update_system_matrix(client, data_id, data_index, matrix_type, update_type);
 * @endcode
 * @param data_id A data object containing matricies
 * @param data_index An index into that array where the matrix is stored
 * @param matrix_type View or projection matrix
 * @param update_type Multiply or set
 * @return OK
 */
uint32_t vrms_client_update_system_matrix(vrms_client_t* client, uint32_t data_id, uint32_t data_index, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type);

/**
 * @brief Create a Skybox
 *
 * Creates a Skybox background for the scene. It is a special type of object
 * due to the significan differences between rendering regular geometry and
 * rendering a skybox. A skybox must be included in the render program in order
 * to be shown.
 *
 * @code{.c}
 * uint32_t skybox_id = vrms_client_create_skybox(client, texture_id, size);
 * @endcode
 * @param texture_id An object id for a loaded texture. The texture must be of
 * the type VRMS_TEXTURE_CUBE_MAP
 * @return A new object id
 */
uint32_t vrms_client_create_skybox(vrms_client_t* client, uint32_t texture_id);
