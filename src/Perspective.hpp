#ifndef PERSPECTIVE_HPP
#define PERSPECTIVE_HPP

#include <cmath>

#include "glm/glm.hpp"

#include "ImagePlane.hpp"

class Perspective: public ImagePlane {
public:
	struct Ray GetRay(double w, double h);

	glm::dvec3 cam_pos;
	glm::dvec3 look;
	glm::dvec3 up;
	double hfov;
	double aspect_ratio;

	glm::dvec3 right;

	glm::dvec3 upper_left;
	glm::dvec3 plane_right;
	glm::dvec3 plane_down;

	// Camera position, look direction, up direction, horizontal FOV, aspect ratio
	Perspective(glm::dvec3 pos, glm::dvec3 lk, glm::dvec3 u, double hf, double ar);
};

#endif
