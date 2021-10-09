#ifndef IMAGEPLANE_HPP
#define IMAGEPLANE_HPP

#include "Ray.hpp"

class ImagePlane {
public:
	// w and h are [0, 1] meaning how far to go along the width and height
	//  of the image plane from the upper left corner
	virtual struct Ray GetRay(double w, double h) = 0;

	// Does nothing,
	//  but necessary to be able to `delete` an instance of ImagePlane
	virtual ~ImagePlane() {}
};

#endif
