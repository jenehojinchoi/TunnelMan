#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"
#include "GameConstants.h"

#define MAX_COORDINATE (VIEW_HEIGHT - SPRITE_WIDTH)
#define MIN_COORDINATE (0)

class StudentWorld;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Actor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Actor: public GraphObject
{
public:
    Actor(StudentWorld* world, int imageID, int startX, int startY, Direction direction, double size = 1.0, unsigned int depth = 0, bool isDisplayed = true);
    ~Actor();
    
    virtual void doSomething() = 0;
    bool isAlive();
    void setDead();
    virtual void annoy(int damage);
    virtual bool canBeAnnoyed() const;
    
    StudentWorld* getWorld();
    bool moveInDirection(Direction direction);
    
private:
    bool m_isAlive;
    StudentWorld* m_world;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// People
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class People : public Actor
{
public:
    People(StudentWorld* world, int imageID, int startX, int startY, Direction direction, int maxHitPoint);
    bool canBeAnnoyed() const final;
    int getHitPoint();
    void takeDamage(int damage);
    
private:
    int m_hitPoint; //Keeps track of how much health TunnelMan and protester objects have
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TunnelMan
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TunnelMan: public People
{
public:
    TunnelMan(StudentWorld* world);
    ~TunnelMan();
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // get functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    int getSquirt();
    int getSonar();
    int getGold();
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Game Logic functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void increaseSquirt();
    void increaseSonar();
    void increaseGold();
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TunnelMan's action functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void doSomething() override;
    void annoy(int damage) override;
    void shoot();
    
private:
    int m_hitPoint = 10;
    int m_squirt = 5;
    int m_sonar = 1;
    int m_gold = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Earth
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Earth: public Actor
{
public:
    Earth(int startX, int startY);
    virtual void doSomething();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Boulder
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Boulder: public Actor
{
public:
    Boulder(int startX, int startY);
    virtual void doSomething();
    
private:
    int m_ticksWaiting;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Squirt
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Squirt: public Actor
{
public:
    Squirt(int startX, int startY);
    virtual void doSomething();

private:
    int m_ticksAlive;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protestors
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Protestor: public People
{
public:
    Protestor(StudentWorld* world, int level);
    virtual void doSomething() override;
    void annoy(int damage) override; // TODO:
    
private:
    int m_ticksToWaitBetweenMoves;
    
    int m_tickRest;
    int m_tickNonRest;
    int m_tickSinceLastTurn;
    int m_tickNonRestSinceShouted;
    int numSquaresToMoveInCurrentDirection;
    
    int m_level;
    bool m_leaving;
    bool m_stunned;
    
    void getBribed();
    void leaveOilField();
    void protestorMove();
    void setNumSquaresToMoveInCurrentDirection();
    bool canSeeTunnelMan();
    void setRandomDirection();
    bool sittingAtIntersection();
    
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Goodies // includes oil, gold, water
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// goodies need tick life, tick passed
class Goodie: public Actor
{
public:
    Goodie(StudentWorld* world, int imageID, int startX, int startY, int score, int sound, int maxTickLife, bool isDisplayed);
    bool isPickupAbleByTunnelMan(const double &distance);
    void increaseTickPassed();
    int getTickPassed();
    int getMaxTickLife();
    
private:
    int m_sound;
    int m_score;
    int m_tick;         // check ticks passed
    int m_maxTickLife;  // check max tick life of goodies
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gold
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Gold: public Goodie
{
public:
    Gold(StudentWorld* world, int startX, int startY, bool dropped, bool isDisplayed);
    void doSomething() override;
    //TODO: void isPickupAbleByProtestor();

private:
    bool m_isPickupAbleByTunnelMan;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Oil Barrel
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class OilBarrel: public Goodie
{
public:
    OilBarrel(StudentWorld* world, int startX, int startY);
    void doSomething() override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sonar
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Sonar: public Goodie
{
public:
    Sonar(StudentWorld* world, int startX, int startY);
    void doSomething() override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Waterpool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WaterPool: public Goodie {
public:
    WaterPool(StudentWorld* world, int startX, int startY, int maxTickLife);
    void doSomething() override;
};


#endif // ACTOR_H_

