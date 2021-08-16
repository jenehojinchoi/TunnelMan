#include "Actor.h"
#include "StudentWorld.h"
#include "GraphObject.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Actor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Actor::Actor(StudentWorld* world, int imageID, int startX, int startY, Direction direction, double size, unsigned int depth, bool isDisplayed): GraphObject(imageID, startX, startY, direction, size, depth), m_isAlive(true), m_world(world)
{
    setVisible(isDisplayed);
}

// destructor
Actor::~Actor()
{
    m_world = nullptr;
};

bool Actor::isAlive()
{
    return m_isAlive;
}

void Actor::setDead()
{
    m_isAlive = false;
}

bool Actor::canBeAnnoyed() const
{
    return false;
}

void Actor::annoy(int damage) {};

StudentWorld* Actor::getWorld()
{
    return m_world;
}

// moveTo(), direction comes from GraphObject
// boulder not working properly yet so I took it out for now
bool Actor::moveInDirection(Direction direction)
{
    switch(direction) {
        case left:
        {
            std::cout << "LEFT in moveInDirection" << "\n";
            if (getX() > 0) { //&& !getWorld()->isThereBoulderinDirection(getX(), getY(), left, this)) {
                moveTo(getX()-1, getY());
                std::cout << "should've moved to left" << "\n";
                return true;
            }
            return false;
        }
            
        case right:
        {
            std::cout << "RIGHT in moveInDirection" << "\n";
            if (getX() < (VIEW_HEIGHT - SPRITE_WIDTH))  { //&& !getWorld()->isThereBoulderinDirection(getX(), getY(), right, this)) {
                moveTo(getX()+1, getY());
                return true;
            }
            return false;
        }
            
        case up:
        {
            if (getY() < (VIEW_HEIGHT - SPRITE_WIDTH))  { //&& !getWorld()->isThereBoulderinDirection(getX(), getY(), up, this)) {
                moveTo(getX(), getY()+1);
                return true;
            }
            return false;
        }
            
        case down:
        {
            if (getY() > 0)  { //&& !getWorld()->isThereBoulderinDirection(getX(), getY(), down, this)) {
                moveTo(getX(), getY()-1);
                return true;
            }
            return false;
        }
            
        case none: return false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// People
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

People::People(StudentWorld* world, int imageID, int x, int y, Direction direction, int maxHitPoint): Actor(world, imageID, x, y, direction), m_hitPoint(maxHitPoint) {}

bool People::canBeAnnoyed() const
{
    return true;
}

void People::takeDamage(int damage)
{
    m_hitPoint -= damage;
}

int People::getHitPoint()
{
    return m_hitPoint;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TunnelMan
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TunnelMan::TunnelMan(StudentWorld* world): People(world, TID_PLAYER, 30, 60, right, 10), m_squirt(5), m_sonar(1), m_gold(0)
{}

TunnelMan::~TunnelMan() {}

int TunnelMan::getSonar()
{
    return m_sonar;
}

int TunnelMan::getGold()
{
    return m_gold;
}

int TunnelMan::getSquirt()
{
    return m_squirt;
}

void TunnelMan::increaseSonar()
{
    ++m_sonar;
}

void TunnelMan::increaseGold()
{
    ++m_gold;
}

void TunnelMan::increaseSquirt()
{
    m_squirt += 5;
}

void TunnelMan::doSomething()
{
    if (getHitPoint() <= 0)
        // dead
        return;
    
    int input;
    
    if (getWorld()->getKey(input)) {
        switch(input) {
            case KEY_PRESS_ESCAPE:
                // dead
                break;
            case KEY_PRESS_LEFT:
            {
                //std::cout << "LEFT in doSomething" << "\n";
                if (getDirection() == left) {
                    if (moveInDirection(left)) {
                        if (getWorld()->isThereEarthAtPoint(getX(), getY()))
                            getWorld()->digEarth(getX(), getY());
                    }
                }
                else setDirection(left);
                break;
            }
                
            case KEY_PRESS_RIGHT:
            {
                //std::cout << "RIGHT in doSomething" << "\n";
                if (getDirection() == right) {
                    if (moveInDirection(right)) {
                        if (getWorld()->isThereEarthAtPoint(getX(), getY()))
                            getWorld()->digEarth(getX(), getY());
                    }
                }
                else setDirection(right);
                break;
            }
                
            case KEY_PRESS_UP:
            {
                if (getDirection() == up) {
                    if (moveInDirection(up)) {
                        if (getWorld()->isThereEarthAtPoint(getX(), getY()))
                            getWorld()->digEarth(getX(), getY());
                    }
                }
                else setDirection(up);
                break;
            }
                
            case KEY_PRESS_DOWN:
            {
                if (getDirection() == down) {
                    if (moveInDirection(down)) {
                        if (getWorld()->isThereEarthAtPoint(getX(), getY()))
                            getWorld()->digEarth(getX(), getY());
                    }
                }
                else setDirection(down);
                break;
            }
                
            case KEY_PRESS_SPACE:
                // shoot()
                break;
            case 'z':
            case 'Z':
                if (m_sonar > 0) {
                    m_sonar--;
                    getWorld()->activateSonar(getX(), getY(), 12);
                    
                    getWorld()->playSound(SOUND_SONAR);
                }
                break;
            case KEY_PRESS_TAB:
                if (m_gold > 0) {
                    m_gold--;
                    //getWorld()->dropGold(new Gold(getWorld(), getX(), getY(), true, true));
                }
                break;
        }
    }
}

void TunnelMan::annoy(int damage)
{
    takeDamage(damage);
    if(getHitPoint() <= 0) setDead();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Earth
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Earth::Earth(int startX, int startY): Actor(nullptr, TID_EARTH, startX, startY, right, 0.25, 3, true) {}

void Earth::doSomething()
{
    return;
} // dont do anything

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protestors
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Goodies // includes oil, gold, water
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//Goodie::Goodie(StudentWorld* world, int imageID, int startX, int startY, bool isDisplayed): Actor(world, imageID, startX, startY, right, 1.0, 2, true) {}
//
//bool Goodie::isPickedUp()
//{
//    // if within a distance
//    setDead();
//    getWorld()->playSound(SOUND_GOT_GOODIE);
//    // increase score
//    return true;
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Gold
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//Gold::Gold(StudentWorld * world,  int startX, int startY, bool isDisplayed, bool isDropped): Goodie(world, TID_GOLD, startX, startY, isDisplayed) {}
//


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Oil Barrel
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//OilBarrel::OilBarrel(StudentWorld* world, int startX, int startY): Goodie(world, TID_BARREL, startX, startY, false) {}
//
//void doSomething()
//{
//    if (!isAlive())
//        return;
//
//// Otherwise, if the Barrel is not currently visible and the TunnelMan is within a radius of 4.0 of it (<= 4.00 units away)
//
//    if (!isVisible()) {// && distance <= 4.0
//        setVisible(true);
//        return;
//
////Otherwise, if the Barrel is within a radius of 3.0 (<= 3.00 units away) from the
//// TunnelMan, then the Barrel will activate,
//    } else {
//        setDead();
//        getWorld()->playSound(SOUND_FOUND_OIL);
//    }
//}

