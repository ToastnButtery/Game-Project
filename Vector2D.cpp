#include "Vector2D.h"



Vector2D Vector2D::normalize() {
	float magnitudeStored = magnitude();
	if (magnitudeStored > 0.0f) {
		x /= magnitudeStored;
		y /= magnitudeStored;
	}

	return *this;
}
