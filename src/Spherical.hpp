#ifndef SPHERICAL_HPP
#define SPHERICAL_HPP

#include <cmath>

#include "glm/glm.hpp"

#include "ImagePlane.hpp"

class Spherical: public ImagePlane {
public:
	struct Ray GetRay(double w, double h);

	glm::dvec3 cam_pos;
	double hang;
	double vang;
	double hfov;
	double aspect_ratio;

	double vfov;

	// Upper left horiz and vert angles
	double ul_hang;
	double ul_vang;

	Spherical(glm::dvec3 pos, double ha, double va, double hf, double ar);
};

#endif
