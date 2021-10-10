// Compile this by itself to a .o and ignore all the warnings.
// Then, the main build can include the .o and
//  not be polluted by all the warnings.

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
