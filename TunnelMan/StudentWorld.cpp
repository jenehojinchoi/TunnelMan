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
        initSonarAndWaterPool();
        clearDeadActors();
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
    initEarth();
    initTunnelMan();
    initBoulder();
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
        
        int sonarChance = rand() % 5;
        
        // Sonar
        if (sonarChance == 0) {
            Sonar* sonar = new Sonar(this, 0, MAX_COORDINATE);
            sonar->updateTickMade();
            m_actors.push_back(sonar);
        }
        
        // Water pool
        else {
            int x, y;
            generateRandomCoordinates(x, y);

            //Generate new random coordinates until the area is clear of earth and objects
            while (isThereEarthAtPoint(x, y) || isThereObjectAtPoint(x, y)) {
                generateRandomCoordinates(x, y);
            }
            WaterPool* water = new WaterPool(this, x, y);
            water->updateTickMade();
            m_actors.push_back(water);
            std::cout << "짜자 " << "\n";
        }
    }
}

void StudentWorld::initBoulder()
{
    int numOfBoulders = fmin(getLevel() / 2 + 2, 9);
    int x, y;
    generateRandomCoordinatesForBoulder(x, y);
    
    for (int i = 0; i < numOfBoulders; ++i) {
        generateRandomCoordinatesForBoulder(x, y);
        
        while (isThereObjectAtPoint(x, y) || ((x > (30 - SPRITE_WIDTH)) && x < 34 && y > 0)) {
            generateRandomCoordinates(x, y);
        }
        
        digEarth(x, y); // delete earth around boulders
        m_actors.push_back(new Boulder(this, x, y, true));
    }
    
}

void StudentWorld::generateRandomCoordinates(int &x, int &y) const
{
    x = rand() % (MAX_COORDINATE + 1);
    y = rand() % (MAX_COORDINATE - SPRITE_WIDTH + 1);
}

//y coordinate 20-56 inclusive
void StudentWorld::generateRandomCoordinatesForBoulder(int &x, int &y) const
{
    x = rand() % (MAX_COORDINATE + 1);
    y = rand() % (37) + 20;
}
void StudentWorld::generateBarrelsAndGold(const int numOfObjects, const char object)
{
    int x, y;
    generateRandomCoordinates(x, y);

    for (int i = 0; i < numOfObjects; i++) {
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
                break;
            }
            // gold
            case 'G':
            {
                m_actors.push_back(new Gold(this, x, y, true, false));
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
    switch (direction) {
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
    
    if (x < 0 || x >= 64 || y < 0 || y >= 64) return false;
    
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

bool StudentWorld::isAboveEarth(int x, int y)
{
    for (int i = x; i < x + 4; i++) {
        if (m_earth[i][y] != nullptr)
            return true;
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

bool StudentWorld::canProtestorShout(int x, int y, GraphObject::Direction d) const
{
    int amountToShift = 1;
    
    if(d == GraphObject::left || d == GraphObject::down)
        amountToShift = 4;
    
    if(!shiftCoordinates(x, y, amountToShift, d)) return false;

    for(int i = x; i < x + SPRITE_WIDTH; ++i) {
        for(int j = y; j < y + SPRITE_HEIGHT; ++j) {
            if(i == m_tunnelMan->getX() && j == m_tunnelMan->getY())
                return true;
        }
    }
    
    return false;
}


int StudentWorld::getPathToPoint(int x, int y, int targetX, int targetY, GraphObject::Direction &d)
{
//    std::cout << "//////////////////////////////////////////////////////////" << "\n";
//    std::cout << "getPathToPoint" << "\n";
//    std::cout << "x: " << x << "\n";
//    std::cout << "y: " << y << "\n";
//
    std::queue<Point> q;
    Point nextPoint;
    
    bool visited[VIEW_WIDTH][VIEW_HEIGHT];

    for (int i = 0; i < VIEW_WIDTH; ++i) {
        for (int j = 0; j < VIEW_HEIGHT; ++j) {
            visited[i][j] = false;
        }
    }
    
    // mark the entrance as visited
   // visited[x][y] = true;
    GraphObject::Direction directions [] = {GraphObject::up, GraphObject::down, GraphObject::left, GraphObject::right};
    
    // up: 0
    // down: 1
    // left: 2
    // right: 3
    
    for (int i = 0; i < 4; ++i) {
        int a = x;
        int b = y;

        if (!isThereBoulderInDirection(x, y, directions[i], nullptr)
            && !isThereEarthInDirection(x, y, directions[i])) {
            if (shiftCoordinates(a, b, 1, directions[i])) {
                visited[a][b] = true;
                Point p(a, b, to_string(i));
                q.push(p);
            }
        }
    }
    
    while(!q.empty()) {
        
        nextPoint = q.front();
        q.pop();
        
        //check all directions
        for (int i = 0; i < 4; ++i) {
            // found target
            if (nextPoint.x == targetX && nextPoint.y == targetY) {
                q = queue<Point>();
                break;
            }
            
            if (!isThereBoulderInDirection(x, y, directions[i], nullptr)
                && !isThereEarthInDirection(x, y, directions[i])) {
                int x = nextPoint.x;
                int y = nextPoint.y;
                
                if (shiftCoordinates(x, y, 1, directions[i])) {
                    if (!visited[x][y]) {
                        visited[x][y] = true;
                        Point n(x, y, nextPoint.string+to_string(i));
                        q.push(n);
                    }
                }
            }
        }
    }
    
    int index = stoi(nextPoint.string.substr(0,1));
    //std::cout << "index: " << index << "\n";
    d = directions[index];
    //initialStep = nextPoint.initialStepToReachPoint;
    return int(nextPoint.string.size());
}

int StudentWorld::getPathToTunnelMan(int x, int y, GraphObject::Direction &d)
{
    int targetX = m_tunnelMan->getX();
    int targetY = m_tunnelMan->getY();
    return getPathToPoint(x, y, targetX, targetY, d);
}

bool StudentWorld::canMoveInDirection(int x, int y, GraphObject::Direction direction)
{
    switch (direction) {
            
        case GraphObject::left:
            return (x != 0 && !isThereEarthAtPoint(x-1, y) && isThereBoulderAtPoint(x, y));
            
        case GraphObject::right:
            return (x != 60 && !isThereEarthAtPoint(x+1, y) && !isThereBoulderAtPoint(x+1, y));
            
        case GraphObject::up:
            return (y != 60 &&!isThereEarthAtPoint(x, y+1) && !isThereBoulderAtPoint(x, y+1));
            
        case GraphObject::down:
            return (y != 0 && !isThereEarthAtPoint(x, y-1) && !isThereBoulderAtPoint(x, y-1));
            
        case GraphObject::none:
            return false;
            
    }
    return false;
}

bool StudentWorld::isThereBoulderAtPoint(int x, int y)
{
    vector<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++) {
        if ((*it)->getID() == TID_BOULDER) {
            if (x == (*it)->getX() && y == (*it)->getY())
                return true;
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
//                delete m_earth[i][j];
                m_earth[i][j] -> setVisible(false);
                m_earth[i][j] = nullptr;
            }
        }
    }
}

void StudentWorld::shootWithSquirt()
{
    //std::cout << "Shoot!!!" << "\n";
    m_actors.push_back(new Squirt(this, m_tunnelMan->getX(), m_tunnelMan->getY(), m_tunnelMan->getDirection()));
    playSound(SOUND_PLAYER_SQUIRT);
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

bool StudentWorld::checkForBribes(const int x, const int y)
{
    double d;
    for (vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        if((*it)->canBeAnnoyed()) {
            d = getDistance(x, y, (*it)->getX(), (*it)->getY());
        
            if (d <= 3.0 && (*it)->getID() == TID_PROTESTER) {
                if((*it)->getBribed())
                    return true;    //Protester was bribed
            }
        }
    }
    return false;
}


void StudentWorld::dropGold(Gold* gold)
{
    std::cout << "DROP GOLD!!!!!" << "\n";
    gold->updateTickDropped();
    m_actors.push_back(gold);
}

void StudentWorld::boulderHitsPeople(const int x, const int y)
{
    for (vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); it++) {
        if ((*it)->canBeAnnoyed()) { // check if people (protestor)
            double d = getDistance(x, y, (*it)->getX(), (*it)->getY());
            if (d <= 3.0) {
                (*it)->getAttacked(100);
                increaseScore(500); // increase score by 500 if a protestor is bonked with a boulder
            }
        }
    }
    
    // check if tunnelman
    double d2 = getDistanceFromTunnelMan(x, y);
    if (d2 <= 6.0) { // double 3.0
        m_tunnelMan->getAttacked(100);
    }
}

bool StudentWorld::squirtHits(const int x, const int y)
{
    bool result = false;
    for (std::vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); it++) {
        // check protestors
        if ((*it)->canBeAnnoyed()) {
            double d = getDistance(x, y, (*it)->getX(), (*it)->getY());
            
            if (d <= 3.0) {
                (*it)->getAttacked(2); // if protestor was annoyed, return true
                result = true;
            }
        }
    }
    return result; // protestor not annoyed
}

void StudentWorld::protestorShoutsAtTunnelMan()
{
    m_tunnelMan->getAttacked(2);
    playSound(SOUND_PROTESTER_YELL);
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
    int P = fmin(15, int(2 +getLevel() * 1.5)); // target number of protestors
    // A new Protester (Regular or Hardcore) may only be added to the oil field after at least T ticks have passed since the last Protester of any type was added, where:
    int T = fmax(25, 200-(int)getLevel());
    
    if (this->m_numOfProtestors > P) return;

    if (this->m_tick % T == 0 || this->m_tick == 0) {
        int p = fmin(90, getLevel() * 10 + 30); // probability
        int r = (rand() % 100) + 1; // random number

        if (r <= p)
            m_actors.push_back(new HardcoreProtestor(this, this->getLevel()));
        else
            m_actors.push_back(new Protestor(this, this->getLevel(), TID_PROTESTER));

        ++this->m_numOfProtestors;
    }
}

void StudentWorld::TunnelManActorsDoSomething()
{
    std::cout << "m_tick: " << m_tick << "\n";
    m_tunnelMan->doSomething();
    for (std::vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        (*it)->doSomething();
    }
}


void StudentWorld::clearDeadActors()
{
    std::vector<Actor*>::iterator it = m_actors.begin();
    
    while (it != m_actors.end()) {
        if (!((*it)->isAlive())) {
            (*it)->setVisible(false); // made it set invisible
            (*it) = nullptr;
            m_actors.erase(it);
            it = m_actors.begin();
        }
        else
            ++it;
    }
}

void StudentWorld::moveToExit(Protestor *pr, int a, int b)
{
    std::cout << "MOVE TO EXIT CALLED!" << "\n";
    for (int i = 0; i < 64; i++){
        for (int j = 0; j < 64; j++){
            visited[i][j] = 0;
        }
    }
    
    queue<Point> q;
    q.push(Point(60,60));
    visited[60][60] = 1;
    
    while (!q.empty()) {
        Point c = q.front();
        q.pop();
        
        int x=c.x;
        int y=c.y;

        //left
        if(canMoveInDirection(x,y, GraphObject::left) && visited[x-1][y]==0){
            q.push(Point(x-1,y));
            visited[x-1][y] = visited[x][y]+1;
        }
        //right
        if(canMoveInDirection(x,y, GraphObject::right) && visited[x+1][y]==0){
            q.push(Point(x+1,y));
            visited[x+1][y] = visited[x][y]+1;
        }
        //up
        if(canMoveInDirection(x,y, GraphObject::up) && visited[x][y+1]==0){
            q.push(Point(x,y+1));
            visited[x][y+1] = visited[x][y]+1;
        }
        // down
        if(canMoveInDirection(x,y, GraphObject::down) && visited[x][y-1]==0){
            q.push(Point(x,y-1));
            visited[x][y-1] = visited[x][y]+1;
        }
    }
    
    if(canMoveInDirection(a,b, GraphObject::left) && visited[a-1][b] < visited[a][b])
        pr->moveInDirection(GraphObject::left);
    if(canMoveInDirection(a,b, GraphObject::right) && visited[a+1][b] < visited[a][b])
        pr->moveInDirection(GraphObject::right);
    if(canMoveInDirection(a,b, GraphObject::up) && visited[a][b+1] < visited[a][b])
        pr->moveInDirection(GraphObject::up);
    if(canMoveInDirection(a,b, GraphObject::down) && visited[a][b-1] < visited[a][b])
        pr->moveInDirection(GraphObject::down);
    return ;
}
