#ifndef ORTHOGRAPHIC_HPP
#define ORTHOGRAPHIC_HPP

#include <cmath>

#include "glm/glm.hpp"

#include "ImagePlane.hpp"

class Orthographic: public ImagePlane {
public:
	struct Ray GetRay(double w, double h);

	glm::dvec3 cam_pos;
	glm::dvec3 look;
	glm::dvec3 up;
	double ortho_width;
	int screen_width;
	int screen_height;

	glm::dvec3 right;

	glm::dvec3 upper_left;
	glm::dvec3 plane_right;
	glm::dvec3 plane_down;

	Orthographic(glm::dvec3 pos, glm::vec3 lk, glm::vec3 u, double ow, int sw, int sh);
};

#endif
