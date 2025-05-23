#include "Game.h"
#include <iostream>
#include "SDL2/SDL_ttf.h"
#include <sstream>



Game::Game(SDL_Window* window, SDL_Renderer* renderer, int windowWidth, int windowHeight) :
    placementModeCurrent(PlacementMode::wall),
    level(renderer, windowWidth / tileSize, windowHeight / tileSize),
    spawnTimer(0.25f), roundTimer(5.0f),gameOver(false),endTime(0),windowWidth(windowWidth), windowHeight(windowHeight), gameOverTexture(nullptr){
       running=true;

       if(TTF_Init()==-1){
        std::cerr<<" Failed to initialize SDL_TTF: "<<TTF_GetError()<<std::endl;

       }
       startTime=SDL_GetTicks();

      font = TTF_OpenFont("Data/Fonts/fast99.ttf", 24);
      if (!font) std::cerr << "Failed to load fonts: " << TTF_GetError() << std::endl;

        baseMaxHealth=10;
        baseHealth=baseMaxHealth;
       basePosition=Vector2D(windowWidth/(2*tileSize), windowHeight/(2*tileSize));

    if (window != nullptr && renderer != nullptr) {

        textureOverlay = TextureLoader::loadTexture(renderer, "Overlay.bmp");


        mix_ChunkSpawnUnit = SoundLoader::loadSound("Spawn Unit.ogg");

        //Store the current times for the clock.
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
        font=nullptr;
    }
    if(mix_ChunkSpawnUnit){
        Mix_FreeChunk(mix_ChunkSpawnUnit);
        mix_ChunkSpawnUnit=nullptr;
    }

    if(gameOverTexture){
        SDL_DestroyTexture(gameOverTexture);
        gameOverTexture=nullptr;
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

            case SDL_SCANCODE_R:
                resetBase();
                listUnits.clear();
                listTurrets.clear();
                listProjectiles.clear();
                std::cout<<"Game Restarted!"<<std::endl;
                break;

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
    if(gameOver) return;

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
        if(enemyPos.distanceTo(basePosition)<0.5f){
            baseHealth--;
            std::cout << "Base Health: " << baseHealth << std::endl;

            if(baseHealth<=0){
                std::cout<<"Game Over!"<<std::endl;
                gameOver=true;
                endTime=SDL_GetTicks();
                finalWave=waveNumber;
                return;
            }
            it=listUnits.erase(it);
            increment=false;
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
                waveNumber++;

            spawnUnitCount = 15+(waveNumber/5)*5;
            if(spawnUnitCount>30) spawnUnitCount=30;
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
    if (gameOver) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (!gameOverTexture) {
            gameOverTexture = TextureLoader::loadTexture(renderer, "Data/Images/GameOverScreen.bmp");
            if (!gameOverTexture) {
                std::cerr << "Failed to load Game Over screen!" << std::endl;
            }
        }

        if (gameOverTexture) {
            int w, h;
            SDL_QueryTexture(gameOverTexture, NULL, NULL, &w, &h);
            SDL_Rect dstRect = { windowWidth / 2 - w / 2, windowHeight / 3, w, h };
            SDL_RenderCopy(renderer, gameOverTexture, NULL, &dstRect);
        }


        Uint32 elapsedTime = (endTime - startTime) / 1000;
        std::string timeText = "Time Survived: " + std::to_string(elapsedTime) + " s";
        SDL_Color textColor = { 255, 255, 255, 255 };

        SDL_Surface* timeSurface = TTF_RenderText_Solid(font, timeText.c_str(), textColor);
        if (timeSurface) {
            SDL_Texture* timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface);
            if (timeTexture) {
                SDL_Rect timeRect = { windowWidth / 2 - timeSurface->w / 2, windowHeight / 2 - 40, timeSurface->w, timeSurface->h };
                SDL_RenderCopy(renderer, timeTexture, NULL, &timeRect);
                SDL_DestroyTexture(timeTexture);
            } else {
                std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
            }
            SDL_FreeSurface(timeSurface);
        }


        std::string restartText = "Press R to Restart";
        SDL_Surface* restartSurface = TTF_RenderText_Solid(font, restartText.c_str(), textColor);
        if (restartSurface) {
            SDL_Texture* restartTexture = SDL_CreateTextureFromSurface(renderer, restartSurface);
            if (restartTexture) {
                SDL_Rect restartRect = { windowWidth / 2 - restartSurface->w / 2, windowHeight / 2, restartSurface->w, restartSurface->h };
                SDL_RenderCopy(renderer, restartTexture, NULL, &restartRect);
                SDL_DestroyTexture(restartTexture);
            } else {
                std::cerr << "Failed to create restart texture: " << SDL_GetError() << std::endl;
            }
            SDL_FreeSurface(restartSurface);
        }
         std::string waveText = "Wave Reached: " + std::to_string(finalWave);
        SDL_Surface* waveSurface = TTF_RenderText_Solid(font, waveText.c_str(), textColor);
        if (waveSurface) {
            SDL_Texture* waveTexture = SDL_CreateTextureFromSurface(renderer, waveSurface);
            if (waveTexture) {
                SDL_Rect waveRect = { windowWidth / 2 - waveSurface->w / 2, windowHeight / 2 + 40, waveSurface->w, waveSurface->h };
                SDL_RenderCopy(renderer, waveTexture, NULL, &waveRect);
                SDL_DestroyTexture(waveTexture);
            } else {
                std::cerr << "Failed to create wave texture: " << SDL_GetError() << std::endl;
            }
            SDL_FreeSurface(waveSurface);
        }



        SDL_RenderPresent(renderer);
        return;
    }



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


    SDL_Rect healthBarBackground = { 40, 10, 200, 20 };
    SDL_Rect healthBar = { 40, 10, (int)(200 * ((float)baseHealth / baseMaxHealth)), 20 };

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthBarBackground);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &healthBar);

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

void Game::resetBase(){
    baseHealth=baseMaxHealth;
    spawnTimer.resetToMax();
    roundTimer.resetToMax();
    gameOver=false;
    startTime=SDL_GetTicks();
    waveNumber=1;
    finalWave=0;
    std::cout<< "Base health reset!"<<std::endl;
}
