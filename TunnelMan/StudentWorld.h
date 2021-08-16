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
    StudentWorld(std::string assetDir); //?
    ~StudentWorld();

    virtual int init();
    virtual int move();
    virtual void cleanUp();
    void initEarth();
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // distance functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    double getDistance(int x1, int y1, int x2, int y2);
    double getDistanceFromTunnelMan(int x, int y);
    
    // direction
    bool isThereBoulderinDirection(int x, int y, GraphObject::Direction direction, Actor *actor);
    bool isThereEarthInDirection(int x, int y, GraphObject::Direction direction);
    
    // at point
    bool isThereEarthAtPoint(int x, int y);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // action functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void digEarth(int x, int y);
    void activateSonar(int x, int y, int radius);
    void dropGold(Gold* gold);
    
private:
    int tick;
    std::vector<Actor*> m_actors;
    std::vector<Gold*> m_droppedGold;
    Earth* m_earth[64][64];
    TunnelMan* m_tunnelMan;
    
    void askPlayerAndObjectsToDoSomething();
};

#endif // STUDENTWORLD_H_
