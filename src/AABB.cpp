#include "AABB.hpp"

// Functions `intersection` and `distance`
// are adapted from the JavaScript implementations from
// https://github.com/stackgl/ray-aabb-intersection
// which has the license:
/*
The MIT License (MIT)
Copyright (c) 2015 Hugh Kennedy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.
*/

bool
intersection(glm::dvec3 *out, struct Ray ray, glm::dvec3 c0, glm::dvec3 c1) {
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
