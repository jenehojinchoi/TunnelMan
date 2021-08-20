#include "StudentWorld.h"
#include "GraphObject.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <queue>
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
    m_numOfProtestors = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
// init
////////////////////////////////////////////////////////////////////////////////////////////////////////

int StudentWorld::init()
{
    initEarth();
    initTunnelMan();
    ++m_tick;
    
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::initEarth()
{
    for(int i = 0; i < VIEW_WIDTH; ++i) {
        for(int j = 0; j < MAX_COORDINATE; ++j) {
            if(!(i >= 30 && i <= 33 && j >= 4)) {
                m_earth[i][j] = new Earth(i,j);
            }
        }
    }
}

void StudentWorld::initTunnelMan()
{
    m_tunnelMan = new TunnelMan(this);
}


void StudentWorld::TunnelManActorsDoSomething()
{
    m_tunnelMan->doSomething();
    for (std::vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); ++it)
        (*it)->doSomething();
}

int StudentWorld::move()
{
    while(m_tunnelMan->isAlive())
    {
        TunnelManActorsDoSomething();
        initProtesters();
        ++m_tick;
        return GWSTATUS_CONTINUE_GAME;
    }
    
    decLives();
    playSound(SOUND_PLAYER_GIVE_UP);
    return GWSTATUS_PLAYER_DIED;
}

//int StudentWorld::findOptimalPath(int startX, int startY, int goalX, int goalY, GraphObject::Direction &initialStep)
//{
//
//}

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

bool StudentWorld::shiftCoordinates(int &x, int &y, const int amountToShift, GraphObject::Direction directionToShift) const
{
    switch(directionToShift) {
        case GraphObject::down:
        {
            y -= amountToShift;
            if (y < 0) return false;
            break;
        }
            
        case GraphObject::up:
        {
            y += amountToShift;
            
            if (y > MAX_COORDINATE) return false;
            break;
        }
            
        case GraphObject::left:
        {
            x -= amountToShift;
            
            if (x < 0) return false;
            break;
        }
            
        case GraphObject::right:
        {
            x += amountToShift;
            
            if (x > MAX_COORDINATE) return false;
            break;
        }
            
        case GraphObject::none:
            return false;
    }
    return true;
}

bool StudentWorld::isThereBoulderInDirection(int x, int y, GraphObject::Direction direction, Actor *actor)
{
    if(!shiftCoordinates(x, y, 1, direction)) return true;
    
    vector<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); ++it) {
        if ((*it)->getID() == TID_BOULDER) {
            double distance = getDistance(x, y, (*it)->getX(), (*it)->getY());
            if (distance <= 3.0) return true;
        }
    }
    
    return false;
}

// checking for earth should be made as another function
bool StudentWorld::isThereEarthInDirection(int x, int y, GraphObject::Direction direction)
{
    switch (direction) //Determines whether to shift the x or y coordinate, and by how much
    {
        case GraphObject::right:
        case GraphObject::up:
        {
            shiftCoordinates(x, y, 4, direction);
            break;
        }
        case GraphObject::left:
        case GraphObject::down:
        {
            shiftCoordinates(x, y, 1, direction);
            break;
        }
        case GraphObject::none:
            return true;
    }
    
    if (direction == GraphObject::right || direction == GraphObject::left) {
        for (int i = y; i < y + 4; ++i) {
            if (m_earth[x][i]) return true;
        }
    } else {
        for (int i = x; i < x + 4; ++i) {
            if (m_earth[i][y]) return true;
        }
    }
    
    return false;
}


bool StudentWorld::isThereTunnelManInLine(int x, int y, GraphObject::Direction &direction)
{
    int tunnelManX = m_tunnelMan->getX();
    int tunnelManY = m_tunnelMan->getY();
    
    if (x == tunnelManX) {
        if (y > tunnelManY) direction = GraphObject::down;
        else direction = GraphObject::up;
    }
    
    else if (y == tunnelManY) {
        if (x > tunnelManX) direction = GraphObject::left;
        else direction = GraphObject::right;
    }
    
    else return false; // not in a same x or y (not in a straight line)
    
    while(!isThereEarthInDirection(x, y, direction) && !isThereBoulderInDirection(x, y, direction, nullptr)) {
        shiftCoordinates(x, y, 1, direction);
        if (x == tunnelManX && y == tunnelManY) return true;
    }
    
    return false;   //There was either Earth or a boulder blocking the path to TunnelMan
}

bool StudentWorld::isProtestorFacingTunnelMan(int x, int y, GraphObject::Direction direction)
{
    int amountToShift = 1;
    if (direction == GraphObject::left || direction == GraphObject::down)
        amountToShift = 4;
    if (!shiftCoordinates(x, y, amountToShift, direction)) return false;
    
    for (int i = x; i < x + SPRITE_WIDTH; ++i) {
        for (int j = y; j < y + SPRITE_HEIGHT; ++j) {
            if (i == m_tunnelMan->getX() && j == m_tunnelMan->getY()) return true;
        }
    }
    
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// at point
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
            if (m_earth[i][j] != nullptr) {
                delete m_earth[i][j];
                m_earth[i][j] = nullptr;
            }
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

//GraphObject::Direction StudentWorld::getDirectionToExit(int x, int y)
//{
//    GraphObject::Direction d;
//    findOptimalPath(x, y, MAX_COORDINATE, MAX_COORDINATE, d);
//    return d;
//}
//

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// keeping track of games
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void StudentWorld::decreaseNumOfProtestors()
{
    --this->m_numOfProtestors;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void StudentWorld::initProtesters()
{
    int T = min(15, int(2 +getLevel() * 1.5));
    int ticksToWaitBetweenMoves = max(25, 200-(int)getLevel());
    
    if (this->m_numOfProtestors > T) return;
    
    if (this->m_tick % ticksToWaitBetweenMoves == 0 || this->m_tick==0) {
        m_actors.push_back(new Protestor(this, this->getLevel()));
        ++this->m_numOfProtestors;
    }
}


