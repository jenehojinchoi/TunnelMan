#include "StudentWorld.h"
#include "GraphObject.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
    return new StudentWorld(assetDir);
}

StudentWorld::StudentWorld(string assetDir): GameWorld(assetDir) {}

StudentWorld::~StudentWorld()
{
    cleanUp();
}

void StudentWorld::cleanUp()
{
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 60; y++) {
            delete m_earth[x][y];
        }
    }
    vector<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++) {
        delete *it;
    }
    m_actors.clear();
    delete m_tunnelMan;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////
// init
////////////////////////////////////////////////////////////////////////////////////////////////////////

int StudentWorld::init()
{
    makeEarthField();
    m_tunnelMan = new TunnelMan(this);
    ++tick;
    
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::askPlayerAndObjectsToDoSomething()
{
    m_tunnelMan->doSomething();
    for (std::vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); ++it)
        (*it)->doSomething();
}

int StudentWorld::move()
{
    while(m_tunnelMan->isAlive())
    {
        askPlayerAndObjectsToDoSomething();
        ++tick;
        return GWSTATUS_CONTINUE_GAME;
    }
    
    decLives();
    playSound(SOUND_PLAYER_GIVE_UP);
    return GWSTATUS_PLAYER_DIED;
}

void StudentWorld::makeEarthField()
{
    for(int i = 0; i < VIEW_WIDTH; ++i) {
        for(int j = 0; j < VIEW_HEIGHT - SPRITE_WIDTH; ++j) {
            if(!(i >= 3 && i <= 33 && j >= 4)) {
                m_earth[i][j] = new Earth(i,j);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// distance functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double StudentWorld::getDistance(int x1, int y1, int x2, int y2)
{
    return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

double StudentWorld::getDistanceFromTunnelMan(int x, int y)
{
    return getDistance(x, y, m_tunnelMan->getX(), m_tunnelMan->getY());
}

bool StudentWorld::isThereBoulderinDirection(int x, int y, GraphObject::Direction direction, Actor *actor)
{
    vector<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++) {
        if ((*it)->getID() == TID_BOULDER) {
            double distance = getDistance(x, y, (*it)->getX(), (*it)->getY());
            if (distance < 3.0)
                return false;
        }
    }
    return true;
}

// checking for earth should be made as another function
bool StudentWorld::isThereEarthInDirection(int x, int y, GraphObject::Direction direction)
{
//    switch(direction){
//        case GraphObject::right:
//            if (x + 4 > 60) return false;
//            break;
//        case GraphObject::up:
//            if (y - 4 > 60) return false;
//            break;
//        case GraphObject::left:
//            if (x - 4 < 0) return false;
//            break;
//        case GraphObject::down:
//            if (y - 4 > 60) return false;
//            break;
//        case GraphObject::none:
//            return false;
//    } return true;
    
    if (direction == GraphObject::right || direction == GraphObject::left) {
        for (int i = y; i < y + 4; ++i) {
            if (m_earth[x][i]) return false;
        }
    } else {
        for (int i = x; i < x + 4; ++i) {
            if (m_earth[i][y]) return false;
        }
    }
    
    return true;
}


bool StudentWorld::isThereEarthAtPoint(int x, int y)
{
    for (int i = x; i < x + SPRITE_WIDTH; ++i) {
        for (int j = y; j < y + SPRITE_WIDTH; ++j) {
            if (m_earth[i][j])
                return true;
        }
    }
    return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// action functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void StudentWorld::digEarth(int x, int y)
{
    // earth is 4 by 4, so iterate through x & y coordinates
    for (int i = x; i < x + 4; i++) {
        for (int j = y; j < y + 4; j++) {
            m_earth[i][j] = nullptr;
        }
    }
    playSound(SOUND_DIG);
}

void StudentWorld::activateSonar(int x, int y, int radius)
{
    int a, b;
    vector<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ((*it)->getID() == TID_BARREL || (*it)->getID() == TID_GOLD) {
            a = (*it)->getX();
            b = (*it)->getY();
            
            if (getDistance(x, y, a, b) < 12.0) // sonar range distance is 12.
                (*it)->setVisible(true);
        }
    }
    
    playSound(SOUND_SONAR);
}

void StudentWorld::dropGold(Gold* gold)
{
    m_droppedGold.push_back(gold);
}
