
typedef struct 

/*
    runtime
        render_manager
            render [thread 1, card 0]
            render [thread 2, card 1]
    scene
        scene 1 [render thread 1]
        scene 2 [render thread 1]
        scene 3 [render thread 2]

Runtime has a render_manager. For DRM this is responsible for monitoring card
and/or screen hotplug events and creating new card render threads (or screen
render threads). The runtime_manager broadcasts back to runtime the available
screens when there are changes. Runtime maintains a list of scenes. Scenes
decide what screen to be attached to. Screen capabilities (MONO only, STEREO,
dimensions etc) are sent to clients so they can decide where to display. For
example wayvroom might prefer a MONO screen if available.

Render manager interface:

    render_manager_init():
        Does initial system setup, create threads for currently attached
        resources. For X11 runs glutInit(). Starts render threads for each
        card, each having it's own render loop. Within each render loop the
        render thread has access to the scenes and render_vm's for those
        scenes.

    render_manager_assign_scene_to_screen():
        Assigns a scene to a screen.

    render_manager_set_screen_mode():
        Sets the MONO or STEREO mode to a screen. Render manager defaults when
        a screen is created based on if it thinks it's an HMD or not.

    render_manager_queue_add_data_load(screen_id, ...): 
        Loads data to the GPU for that screen.

    render_manager_queue_add_texture_load(screen_id, ...):
        Loads data to the GPU for that screen.

    render_manager_queue_update_matrix(screen_id, ...):
        Update a system matrix for the screen.

Render interface:

    render_init():
        Creates buffers, initializes screens, creates X11 windows etc.

    render_run():
        Enters the render loop. Locks scenes while rendering.

Simplest render_manager:

    Creates a single screen on a single card. 

Render:

    This is a virtual screen. It could be two separate physical screens on two
    separate cards, one left eye one right eye.

    screen modes:
        MONO, STEREO_SPLIT, STEREO_LEFT, STEREO_RIGHT

    List of screens and the mode they are in.


*/
