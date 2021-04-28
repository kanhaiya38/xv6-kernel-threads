#define CLONE_VM      0x00000100 // Set if VM shared between processes.
#define CLONE_FS      0x00000200 // Set if fs info shared between processes.
#define CLONE_FILES   0x00000400 // Set if open files shared between processes.
#define CLONE_PARENT  0x00008000 // Set if we want to have the same parent as the cloner.
#define CLONE_THREAD  0x00010000 // Set to add to same thread group.
