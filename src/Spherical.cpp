#include "Spherical.hpp"

Spherical::Spherical(glm::dvec3 pos, double ha, double va, double hf, double ar) {
	cam_pos = pos;

	hang = ha;
	vang = va;
	hfov = hf;
	aspect_ratio = ar;

	vfov = hfov / aspect_ratio;

	ul_hang = hang + (hfov / 2.0);
	ul_vang = vang - (vfov / 2.0);
}

struct Ray Spherical::GetRay(double w, double h) {
	double ha = ul_hang - w * hfov;
	double va = ul_vang + h * vfov;

	glm::dvec3 pos = cam_pos;
	glm::dvec3 dir(
		sin(va) * cos(ha),
		sin(va) * sin(ha),
		cos(va)
	);

	struct Ray ray = {pos, dir};

	return ray;
}
