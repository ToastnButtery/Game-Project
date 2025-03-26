#include "Unit.h"
#include "Game.h"


const float Unit::speed = 1.5f;
const float Unit::size = 0.48f;




Unit::Unit(SDL_Renderer* renderer, Vector2D setPos) :
	pos(setPos), timerJustHurt(0.25f) {
	texture = TextureLoader::loadTexture(renderer, "Unit.bmp");

}



void Unit::update(float dT, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits) {
	timerJustHurt.countDown(dT);


	float distanceToTarget = (level.getTargetPos() - pos).magnitude();

	if (distanceToTarget < 0.5f) {
		healthCurrent = 0;
	}
	else {

		float distanceMove = speed * dT;
		if (distanceMove > distanceToTarget)
			distanceMove = distanceToTarget;


		Vector2D directionNormal(level.getFlowNormal((int)pos.x, (int)pos.y));

		if ((int)pos.x == (int)level.getTargetPos().x && (int)pos.y == (int)level.getTargetPos().y)
			directionNormal = (level.getTargetPos() - pos).normalize();

		Vector2D posAdd = directionNormal * distanceMove;


		bool moveOk = true;
		for (int count = 0; count < listUnits.size() && moveOk; count++) {
			auto& unitSelected = listUnits[count];
			if (unitSelected != nullptr && unitSelected.get() != this &&
				unitSelected->checkOverlap(pos, size)) {

				Vector2D directionToOther = (unitSelected->pos - pos);

				if (directionToOther.magnitude() > 0.01f) {
					Vector2D normalToOther(directionToOther.normalize());
					float angleBtw = abs(normalToOther.angleBetween(directionNormal));
					if (angleBtw < 3.14159265359f / 4.0f)

						moveOk = false;
				}
			}
		}

		if (moveOk) {

			const float spacing = 0.35f;
			int x = (int)(pos.x + posAdd.x + copysign(spacing, posAdd.x));
			int y = (int)(pos.y);
			if (posAdd.x != 0.0f && level.isTileWall(x, y) == false)
				pos.x += posAdd.x;

			x = (int)(pos.x);
			y = (int)(pos.y + posAdd.y + copysign(spacing, posAdd.y));
			if (posAdd.y != 0.0f && level.isTileWall(x, y) == false)
				pos.y += posAdd.y;
		}
	}
}



void Unit::draw(SDL_Renderer* renderer, int tileSize) {
	if (renderer != nullptr) {

		if (timerJustHurt.timeSIsZero() == false)
			SDL_SetTextureColorMod(texture, 255, 0, 0);
		else
			SDL_SetTextureColorMod(texture, 255, 255, 255);


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



bool Unit::checkOverlap(Vector2D posOther, float sizeOther) {
	return (posOther - pos).magnitude() <= (sizeOther + size) / 2.0f;
}



bool Unit::isAlive() {
	return (healthCurrent > 0);
}



Vector2D Unit::getPos() {
	return pos;
}



void Unit::removeHealth(int damage) {
	if (damage > 0) {
		healthCurrent -= damage;
		if (healthCurrent < 0)
			healthCurrent = 0;

		timerJustHurt.resetToMax();
	}
}
