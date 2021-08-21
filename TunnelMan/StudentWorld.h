#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "Actor.h"
#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include <vector>

class Actor;
class People;
class TunnelMan;
class Earth;
class Gold;
class OilBarrel;
class Sonar;
class WaterPool;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);
    ~StudentWorld();

    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    void setDisplayText();
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // distance functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    double getDistance(int x1, int y1, int x2, int y2) const;
    double getDistanceFromTunnelMan(int x, int y);
    bool shiftCoordinates(int &x, int &y, const int amountToShift, GraphObject::Direction directionToShift) const;
    
    // direction
    
    bool isThereBoulderInDirection(int x, int y, GraphObject::Direction direction, Actor *actor);
    bool isThereEarthInDirection(int x, int y, GraphObject::Direction direction);
    bool isThereTunnelManInLine(int x, int y, GraphObject::Direction &direction);
    bool isProtestorFacingTunnelMan(int x, int y, GraphObject::Direction direction);
    
    // at point
    bool noObjectsAtPoint(const int x, const int y);
    bool isThereEarthAtPoint(int x, int y);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // action functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void digEarth(int x, int y);
    void activateSonar(int x, int y, int radius);
    //void dropGold(Gold* gold);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // keeping track of games
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void decreaseNumOfProtestors();
    void decreaseNumOfBarrels();
    
private:
    int m_tick;
    
    int m_numOfProtestors;
    int m_numOfBarrelsLeft;
    
    std::vector<Actor*> m_actors;
    //std::vector<Gold*> m_droppedGold;
    Earth* m_earth[64][64];
    TunnelMan* m_tunnelMan;

    
    void TunnelManActorsDoSomething();
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // init
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void initEarth();
    void initTunnelMan();
    void initProtestors();
    void initBarrelsAndGold();
    
    void generateRandomCoordinates(int &x, int &y) const;
    void generateBarrelsAndGold(const int numOfObjects, const char object);
};

#endif // STUDENTWORLD_H_
