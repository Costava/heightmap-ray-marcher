#include "Orthographic.hpp"

Orthographic::Orthographic(glm::dvec3 pos, glm::vec3 lk, glm::vec3 u, double ow, int sw, int sh) {
	cam_pos = pos;
	look = lk;
	up = u;
	ortho_width = ow;
	screen_width = sw;
	screen_height = sh;

	right = glm::cross(look, up);

	upper_left = cam_pos - (screen_width / 2.0) * ortho_width * right + (screen_height / 2.0) * ortho_width * up;

	plane_right = screen_width * ortho_width * right;
	plane_down = screen_height * ortho_width * -up;
}

struct Ray Orthographic::GetRay(double w, double h) {
	glm::dvec3 pos = upper_left + w * plane_right + h * plane_down;

	struct Ray ray = {pos, look};

	return ray;
}
