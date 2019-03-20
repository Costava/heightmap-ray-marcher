#include "Perspective.hpp"

Perspective::Perspective(glm::dvec3 pos, glm::dvec3 lk, glm::dvec3 u, double hf, double ar) {
	cam_pos = pos;
	look = lk;
	up = u;
	hfov = hf;
	aspect_ratio = ar;

	double half_plane_width = tan(hfov / 2.0);
	double half_plane_height = half_plane_width / ar;

	right = glm::cross(look, up);
	right = glm::normalize(right);

	upper_left = cam_pos + look + half_plane_height * up - half_plane_width * right;

	glm::dvec3 lower_left = cam_pos + look - half_plane_height * up - half_plane_width * right;
	glm::dvec3 upper_right = cam_pos + look + half_plane_height * up + half_plane_width * right;

	plane_right = upper_right - upper_left;
	plane_down = lower_left - upper_left;
}

struct Ray Perspective::GetRay(double w, double h) {
	glm::dvec3 pos = cam_pos;
	glm::dvec3 dir = glm::normalize((upper_left + w * plane_right + h * plane_down) - cam_pos);

	struct Ray ray = {pos, dir};

	return ray;
}
