#include "Projectile.h"


const float Projectile::speed = 10.0f, Projectile::size = 0.2f,
	Projectile::distanceTraveledMax = 20.0f;




Projectile::Projectile(SDL_Renderer* renderer, Vector2D setPos, Vector2D setDirectionNormal) :
	pos(setPos), directionNormal(setDirectionNormal) {
	texture = TextureLoader::loadTexture(renderer, "Projectile.bmp");
}



void Projectile::update(float dT, std::vector<std::shared_ptr<Unit>>& listUnits) {

	float distanceMove = speed * dT;
	pos += directionNormal * distanceMove;

	distanceTraveled += distanceMove;
	if (distanceTraveled >= distanceTraveledMax)
		collisionOccurred = true;

	checkCollisions(listUnits);
}



void Projectile::draw(SDL_Renderer* renderer, int tileSize) {
	if (renderer != nullptr) {

		int w, h;
		SDL_QueryTexture(texture, NULL, NULL, &w, &h);
		SDL_Rect rect = {
			(int)(pos.x * tileSize) - w / 2,
			(int)(pos.y * tileSize) - h / 2,
			w,
			h };
		SDL_RenderCopy(renderer, texture, NULL, &rect);
	}
}



bool Projectile::getCollisionOccurred() {
	return collisionOccurred;
}



void Projectile::checkCollisions(std::vector<std::shared_ptr<Unit>>& listUnits) {

	if (collisionOccurred == false) {

		for (int count = 0; count < listUnits.size() && collisionOccurred == false; count++) {
			auto& unitSelected = listUnits[count];
			if (unitSelected != nullptr && unitSelected->checkOverlap(pos, size)) {
				unitSelected->removeHealth(1);
				collisionOccurred = true;
			}
		}
	}
}
