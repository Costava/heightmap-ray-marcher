#include "AABB.hpp"

// Adapted from the JavaScript implementation from
//  https://github.com/stackgl/ray-aabb-intersection

bool intersection(glm::dvec3 *out, struct Ray ray, glm::dvec3 c0, glm::dvec3 c1) {
	double d = distance(ray, c0, c1);

	if (d == std::numeric_limits<double>::infinity()) {
		return false;
	}
	else if (d < 0.0) {
		return false;
	}
	else {
		out->x = ray.pos.x + d * ray.dir.x;
		out->y = ray.pos.y + d * ray.dir.y;
		out->z = ray.pos.z + d * ray.dir.z;
	}

	return true;
}

double distance(struct Ray ray, glm::dvec3 c0, glm::dvec3 c1) {
	double lo = -std::numeric_limits<double>::infinity();
	double hi = +std::numeric_limits<double>::infinity();

	double ro[3] = {ray.pos.x, ray.pos.y, ray.pos.z};
	double rd[3] = {ray.dir.x, ray.dir.y, ray.dir.z};
	double aabb0[3] = {c0.x, c0.y, c0.z};
	double aabb1[3] = {c1.x, c1.y, c1.z};

	for (int i = 0; i < 3; ++i) {
		double dim_lo = (aabb0[i] - ro[i]) / rd[i];
		double dim_hi = (aabb1[i] - ro[i]) / rd[i];

		if (dim_lo > dim_hi) {
			double tmp = dim_lo;
			dim_lo = dim_hi;
			dim_hi = tmp;
		}

		if (dim_hi < lo || dim_lo > hi) {
			return std::numeric_limits<double>::infinity();
		}

		if (dim_lo > lo) lo = dim_lo;
		if (dim_hi < hi) hi = dim_hi;
	}

	return (lo > hi) ? std::numeric_limits<double>::infinity() : lo;
}
