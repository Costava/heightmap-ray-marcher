#ifndef AABB_HPP
#define AABB_HPP

#include <limits>
#include "glm/glm.hpp"
#include "Ray.hpp"

// Axis Aligned Bounding Box intersection with a ray

bool intersection(glm::dvec3 *out, struct Ray ray, glm::dvec3 c0, glm::dvec3 c1);

double distance(struct Ray ray, glm::dvec3 c0, glm::dvec3 c1);

#endif
