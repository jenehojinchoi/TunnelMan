#include "StudentWorld.h"
#include "GraphObject.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <queue>
#include <sstream>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
    return new StudentWorld(assetDir);
}

StudentWorld::StudentWorld(string assetDir): GameWorld(assetDir), m_tick(0), m_numOfProtestors(0) {}

StudentWorld::~StudentWorld()
{
    cleanUp();
}

int StudentWorld::move()
{
    while(m_tunnelMan->isAlive())
    {
        setDisplayText();
        TunnelManActorsDoSomething();
        initProtestors();
        // TODO: destroy dead objects
        initSonarAndWaterPool();
        ++m_tick;
        
        if (m_numOfBarrelsLeft == 0)
            return GWSTATUS_FINISHED_LEVEL;
        
        return GWSTATUS_CONTINUE_GAME;
    }
    
    decLives();
    playSound(SOUND_PLAYER_GIVE_UP);
    return GWSTATUS_PLAYER_DIED;
}

void StudentWorld::cleanUp()
{
    // destroy earth
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 60; y++) {
            delete m_earth[x][y];
        }
    }
    
    // destroy actors
    vector<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++) {
        delete *it;
    }
    
    m_actors.clear();
    m_numOfProtestors = 0;
    
    // destroy tunnelman
    delete m_tunnelMan;
}

void StudentWorld::setDisplayText()
{
    stringstream s;
    s << "Lvl: " << setw(2) << getLevel() << " ";
    s << "Lives: " << getLives() << " ";
    s << "Hlth: " << setw(3) << m_tunnelMan->getHitPoint() * 10 << "% ";
    s << "Wtr: " << setw(2) << m_tunnelMan->getSquirt() << " ";
    s << "Gld: " << setw(2) << m_tunnelMan->getGold() << " ";
    s << "Oil Left: " << setw(2) << this->m_numOfBarrelsLeft << " ";
    s << "Sonar: " << setw(2) << m_tunnelMan->getSonar() << " ";
    s << "Scr: " << setw(6) << setfill('0') << getScore();
    
    setGameStatText(s.str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// init
////////////////////////////////////////////////////////////////////////////////////////////////////////

int StudentWorld::init()
{
    std::cout << "StudentWorld::init" << "\n";
    initEarth();
    initTunnelMan();
    
    initBarrelsAndGold();
    m_tick = 0;
    
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

void StudentWorld::initBarrelsAndGold()
{
    this->m_numOfBarrelsLeft = fmin((2 + getLevel()), 21);
    int numOfGold = fmax((5 - getLevel() / 2), 2);
    generateBarrelsAndGold(m_numOfBarrelsLeft, 'B');
    generateBarrelsAndGold(numOfGold, 'G');
}

void StudentWorld::initSonarAndWaterPool()
{
    int goodieChance = (getLevel() * 25) + 300;
    
    if (rand() % goodieChance == 0) {
        
        int tickLife = fmax(100, 300 - 10 * getLevel());
        int sonarChance = rand() % 5;
        
        // Sonar
        if (sonarChance == 0) {
            //m_actors.push_back(new Sonar(this, 0, MAX_COORDINATE, tickLife)));
        }
        
        // Water pool
        else {
            int x, y;
            generateRandomCoordinates(x, y);

            //Generate new random coordinates until the area is clear of earth and objects
            while (isThereEarthAtPoint(x, y) || isThereObjectAtPoint(x, y)) {
                generateRandomCoordinates(x, y);
            }
            m_actors.push_back(new WaterPool(this, x, y, tickLife));
        }
    }
}

void StudentWorld::generateRandomCoordinates(int &x, int &y) const
{
    x = rand() % (MAX_COORDINATE + 1);
    y = rand() % (MAX_COORDINATE - SPRITE_WIDTH + 1);
}

void StudentWorld::generateBarrelsAndGold(const int numOfObjects, const char object)
{
    int x, y;
    generateRandomCoordinates(x, y);

    for (int i = 0; i < numOfObjects; ++i) {
        //Generate new coordinates until area is clear
        while (isThereObjectAtPoint(x, y) || ((x > (30 - SPRITE_WIDTH)) && x < 34 && y > 0)) {
            generateRandomCoordinates(x, y);
        }

        switch(object)
        {
            // barrel of oil
            case 'B':
            {
                m_actors.push_back(new OilBarrel(this, x, y));
                std::cout<<"generateBarrelsAndGold: OilBarrel!!" << "\n";
                break;
            }
            // gold
            case 'G':
            {
                m_actors.push_back(new Gold(this, x, y, true, false));
                std::cout<<"generateBarrelsAndGold: Gold!!" << "\n";
                break;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// distance functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double StudentWorld::getDistance(int x1, int y1, int x2, int y2) const
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
    
    if (x < 0 || x >= VIEW_WIDTH || y < 0 || y >= VIEW_HEIGHT) return false;
    
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


bool StudentWorld::isThereObjectAtPoint(const int x, const int y)
{
    double d;
    std::vector<Actor*>::iterator it;
    it = m_actors.begin();
    
    while (it != m_actors.end()) {
        d = getDistance(x, y, (*it)->getX(), (*it)->getY());
        if (d < 6) return true;
        ++it;
    }
    return false;
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// keeping track of games
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void StudentWorld::decreaseNumOfProtestors()
{
    --this->m_numOfProtestors;
}

void StudentWorld::decreaseNumOfBarrels()
{
    --this->m_numOfBarrelsLeft;
}

void StudentWorld::addToInventory(std::string object)
{
    if (object == "gold") {
        m_tunnelMan->increaseGold();
        return;
    }
    if (object == "sonar") {
        m_tunnelMan->increaseSonar();
        return;
    }
    if (object == "squirt") {
        m_tunnelMan->increaseSquirt();
        return;
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void StudentWorld::initProtestors()
{
    int T = min(15, int(2 +getLevel() * 1.5));
    int ticksToWaitBetweenMoves = max(25, 200-(int)getLevel());
    
    if (this->m_numOfProtestors > T) return;
    
    if (this->m_tick % ticksToWaitBetweenMoves == 0 || this->m_tick == 0) {
        // TODO: hardcore and regular not implemented yet
        // probability, chance
        // compare and make conditional loop
        m_actors.push_back(new Protestor(this, this->getLevel()));
        ++this->m_numOfProtestors;
    }
}


void StudentWorld::TunnelManActorsDoSomething()
{
    m_tunnelMan->doSomething();
    for (std::vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); ++it)
        (*it)->doSomething();
}
