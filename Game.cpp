#include "Game.h"
#include <iostream>
#include <string>



Game::Game(SDL_Window* window, SDL_Renderer* renderer, int windowWidth, int windowHeight) :
    placementModeCurrent(PlacementMode::wall),
    level(renderer, windowWidth / tileSize, windowHeight / tileSize),
    spawnTimer(0.25f), roundTimer(5.0f) {

        if(TTF_Init()==-1){
            std::cerr<<"Failed to initialize SDL_TTF: " <<TTF_GetError()<<std::endl;
        }

        startTime=SDL_GetTicks();

        TTF_Init();
        font=TTF_OpenFont("Data/Fonts/fast99.ttf", 24);
        if(!font) std::cerr<<"Failed to load fonts : "<<TTF_GetError()<<std::endl;


        baseMaxHealth = 10;
baseHealth = baseMaxHealth;
basePosition = Vector2D(windowWidth / (2 * tileSize), windowHeight / (2 * tileSize)); // Tâm màn hình



    if (window != nullptr && renderer != nullptr) {

        textureOverlay = TextureLoader::loadTexture(renderer, "Overlay.bmp");


        mix_ChunkSpawnUnit = SoundLoader::loadSound("Spawn Unit.ogg");


        auto time1 = std::chrono::system_clock::now();
        auto time2 = std::chrono::system_clock::now();


        const float dT = 1.0f / 60.0f;



        bool running = true;
        while (running) {

            time2 = std::chrono::system_clock::now();
            std::chrono::duration<float> timeDelta = time2 - time1;
            float timeDeltaFloat = timeDelta.count();


            if (timeDeltaFloat >= dT) {

                time1 = time2;

                processEvents(renderer, running);
                update(renderer, dT);
                draw(renderer);
            }
        }
    }
}


Game::~Game() {

    TextureLoader::deallocateTextures();
    SoundLoader::deallocateSounds();

    if(font){
        TTF_CloseFont(font);
        font = nullptr;
    }
    TTF_Quit();
}



void Game::processEvents(SDL_Renderer* renderer, bool& running) {
    bool mouseDownThisFrame = false;


    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;

        case SDL_MOUSEBUTTONDOWN:
            mouseDownThisFrame = (mouseDownStatus == 0);
            if (event.button.button == SDL_BUTTON_LEFT)
                mouseDownStatus = SDL_BUTTON_LEFT;
            else if (event.button.button == SDL_BUTTON_RIGHT)
                mouseDownStatus = SDL_BUTTON_RIGHT;
            break;
        case SDL_MOUSEBUTTONUP:
            mouseDownStatus = 0;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.scancode) {

            case SDL_SCANCODE_ESCAPE:
                running = false;
                break;


            case SDL_SCANCODE_1:
                placementModeCurrent = PlacementMode::wall;
                break;
            case SDL_SCANCODE_2:
                placementModeCurrent = PlacementMode::turret;
                break;


            case SDL_SCANCODE_H:
                overlayVisible = !overlayVisible;
                break;
                case SDL_SCANCODE_R:
    resetBase();
    listUnits.clear();
    listTurrets.clear();
    listProjectiles.clear();
    std::cout << "Game restarted!" << std::endl;
    break;

            }
        }
    }



    int mouseX = 0, mouseY = 0;
    SDL_GetMouseState(&mouseX, &mouseY);

    Vector2D posMouse((float)mouseX / tileSize, (float)mouseY / tileSize);

    if (mouseDownStatus > 0) {
        switch (mouseDownStatus) {
        case SDL_BUTTON_LEFT:
            switch (placementModeCurrent) {
            case PlacementMode::wall:

                level.setTileWall((int)posMouse.x, (int)posMouse.y, true);
                break;
            case PlacementMode::turret:

                if (mouseDownThisFrame)
                    addTurret(renderer, posMouse);
                break;
            }
            break;


        case SDL_BUTTON_RIGHT:

            level.setTileWall((int)posMouse.x, (int)posMouse.y, false);

            removeTurretsAtMousePosition(posMouse);
            break;
        }
    }
}



void Game::update(SDL_Renderer* renderer, float dT) {

    updateUnits(dT);


    for (auto& turretSelected : listTurrets)
        turretSelected.update(renderer, dT, listUnits, listProjectiles);


    updateProjectiles(dT);

    updateSpawnUnitsIfRequired(renderer, dT);
}


void Game::updateUnits(float dT) {

    auto it = listUnits.begin();
    while (it != listUnits.end()) {
        bool increment = true;

        if ((*it) != nullptr) {
            (*it)->update(dT, level, listUnits);
            Vector2D enemyPos = (*it)->getPos();
if (enemyPos.distanceTo(basePosition) < 0.5f) {  // Nếu kẻ địch đến gần căn cứ
    baseHealth--;  // Giảm 1 máu căn cứ
    if (baseHealth <= 0) {
        std::cout << "Base Destroyed! Game Over!" << std::endl;
        running = false;
        return;
    }
    it = listUnits.erase(it);
    increment = false;
}


            if ((*it)->isAlive() == false) {
                it = listUnits.erase(it);
                increment = false;
            }
        }

        if (increment)
            it++;
    }
}


void Game::updateProjectiles(float dT) {

    auto it = listProjectiles.begin();
    while (it != listProjectiles.end()) {
        (*it).update(dT, listUnits);


        if ((*it).getCollisionOccurred())
            it = listProjectiles.erase(it);
        else
            it++;
    }
}


void Game::updateSpawnUnitsIfRequired(SDL_Renderer* renderer, float dT) {
    spawnTimer.countDown(dT);


    if (listUnits.empty() && spawnUnitCount == 0) {
        roundTimer.countDown(dT);
        if (roundTimer.timeSIsZero()) {
            spawnUnitCount = 15;
            roundTimer.resetToMax();
        }
    }


    if (spawnUnitCount > 0 && spawnTimer.timeSIsZero()) {
        addUnit(renderer, level.getRandomEnemySpawnerLocation());


        if (mix_ChunkSpawnUnit != nullptr)
            Mix_PlayChannel(-1, mix_ChunkSpawnUnit, 0);

        spawnUnitCount--;
        spawnTimer.resetToMax();
    }
}



void Game::draw(SDL_Renderer* renderer) {

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_RenderClear(renderer);


    level.draw(renderer, tileSize);


    for (auto& unitSelected : listUnits)
        if (unitSelected != nullptr)
            unitSelected->draw(renderer, tileSize);


    for (auto& turretSelected : listTurrets)
        turretSelected.draw(renderer, tileSize);


    for (auto& projectileSelected : listProjectiles)
        projectileSelected.draw(renderer, tileSize);


    if (textureOverlay != nullptr && overlayVisible) {
        int w = 0, h = 0;
        SDL_QueryTexture(textureOverlay, NULL, NULL, &w, &h);
        SDL_Rect rect = { 40, 40, w, h };
        SDL_RenderCopy(renderer, textureOverlay, NULL, &rect);
    }
    SDL_Rect healthBarBackground = {40, 10, 200, 20};  // Nền thanh máu
SDL_Rect healthBar = {40, 10, (int)(200 * ((float)baseHealth / 10)), 20};  // Thanh máu thực tế

// Vẽ nền (màu đỏ - thể hiện mất máu)
SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
SDL_RenderFillRect(renderer, &healthBarBackground);

// Vẽ thanh máu còn lại (màu xanh)
SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
SDL_RenderFillRect(renderer, &healthBar);

Uint32 elapsedTime = (SDL_GetTicks() - startTime)/1000;
std::string timeText ="Time survived: "+ std::to_string(elapsedTime) + " s";

SDL_Color textColor = {255, 255, 255, 255};

SDL_Surface* textSurface =TTF_RenderText_Solid(font, timeText.c_str(), textColor);
if( textSurface){
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect = {40, 40, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

    SDL_RenderPresent(renderer);
}



void Game::addUnit(SDL_Renderer* renderer, Vector2D posMouse) {
    listUnits.push_back(std::make_shared<Unit>(renderer, posMouse));
}



void Game::addTurret(SDL_Renderer* renderer, Vector2D posMouse) {
    Vector2D pos((int)posMouse.x + 0.5f, (int)posMouse.y + 0.5f);
    listTurrets.push_back(Turret(renderer, pos));
}


void Game::removeTurretsAtMousePosition(Vector2D posMouse) {
    for (auto it = listTurrets.begin(); it != listTurrets.end();) {
        if ((*it).checkIfOnTile((int)posMouse.x, (int)posMouse.y))
            it = listTurrets.erase(it);
        else
            it++;
    }


}
void Game::resetBase() {
    baseHealth = baseMaxHealth;  // Khôi phục máu căn cứ về giá trị ban đầu
    std::cout << "Base reset! Health restored to " << baseHealth << std::endl;
}
