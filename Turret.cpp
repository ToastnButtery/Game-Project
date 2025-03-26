#include "Turret.h"


const float Turret::speedAngular = MathAddon::angleDegToRad(180.0f), Turret::weaponRange = 5.0f;




Turret::Turret(SDL_Renderer* renderer, Vector2D setPos) :
	pos(setPos), angle(0.0f), timerWeapon(1.0f) {
	textureMain = TextureLoader::loadTexture(renderer, "Turret.bmp");
	textureShadow = TextureLoader::loadTexture(renderer, "Turret Shadow.bmp");
	mix_ChunkShoot = SoundLoader::loadSound("Turret Shoot.ogg");
}



void Turret::update(SDL_Renderer* renderer, float dT, std::vector<std::shared_ptr<Unit>>& listUnits,
	std::vector<Projectile>& listProjectiles) {

	timerWeapon.countDown(dT);


	if (auto unitTargetSP = unitTarget.lock()) {
		if (unitTargetSP->isAlive() == false ||
			(unitTargetSP->getPos() - pos).magnitude() > weaponRange) {

			unitTarget.reset();
		}
	}


	if (unitTarget.expired())
		unitTarget = findEnemyUnit(listUnits);

	if (updateAngle(dT))
		shootProjectile(renderer, listProjectiles);
}


bool Turret::updateAngle(float dT) {

	if (auto unitTargetSP = unitTarget.lock()) {

		Vector2D directionNormalTarget = (unitTargetSP->getPos() - pos).normalize();


		float angleToTarget = directionNormalTarget.angleBetween(Vector2D(angle));


		float angleMove = -copysign(speedAngular * dT, angleToTarget);
		if (abs(angleMove) > abs(angleToTarget)) {

			angle = directionNormalTarget.angle();
			return true;
		}
		else {

			angle += angleMove;
		}
	}

	return false;
}


void Turret::shootProjectile(SDL_Renderer* renderer, std::vector<Projectile>& listProjectiles) {

	if (timerWeapon.timeSIsZero()) {
		listProjectiles.push_back(Projectile(renderer, pos, Vector2D(angle)));


		if (mix_ChunkShoot != nullptr)
			Mix_PlayChannel(-1, mix_ChunkShoot, 0);

		timerWeapon.resetToMax();
	}
}



void Turret::draw(SDL_Renderer* renderer, int tileSize) {
	drawTextureWithOffset(renderer, textureShadow, 5, tileSize);
	drawTextureWithOffset(renderer, textureMain, 0, tileSize);
}


void Turret::drawTextureWithOffset(SDL_Renderer* renderer, SDL_Texture* textureSelected,
	int offset, int tileSize) {
	if (renderer != nullptr && textureSelected != nullptr) {

		int w, h;
		SDL_QueryTexture(textureSelected, NULL, NULL, &w, &h);
		SDL_Rect rect = {
			(int)(pos.x * tileSize) - w / 2 + offset,
			(int)(pos.y * tileSize) - h / 2 + offset,
			w,
			h };
		SDL_RenderCopyEx(renderer, textureSelected, NULL, &rect,
			MathAddon::angleRadToDeg(angle), NULL, SDL_FLIP_NONE);
	}
}



bool Turret::checkIfOnTile(int x, int y) {
	return ((int)pos.x == x && (int)pos.y == y);
}



std::weak_ptr<Unit> Turret::findEnemyUnit(std::vector<std::shared_ptr<Unit>>& listUnits) {

	std::weak_ptr<Unit> closestUnit;
	float distanceClosest = 0.0f;


	for (auto& unitSelected : listUnits) {

		if (unitSelected != nullptr) {

			float distanceCurrent = (pos - unitSelected->getPos()).magnitude();

			if (distanceCurrent <= weaponRange &&
				(closestUnit.expired() || distanceCurrent < distanceClosest)) {

				closestUnit = unitSelected;
				distanceClosest = distanceCurrent;
			}
		}
	}

	return closestUnit;
}
