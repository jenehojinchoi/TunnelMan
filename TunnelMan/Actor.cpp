#include "Actor.h"
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
}

bool Actor::isAlive()
{
    return m_isAlive;
}

void Actor::setDead()
{
    m_isAlive = false;
}

StudentWorld* Actor::getWorld()
{
    return m_world;
}


void Actor::annoy(int damage) {};

bool Actor::canBeAnnoyed() const
{
    return false;
}

// moveTo(), direction comes from GraphObject
// boulder not working properly yet so I took it out for now
bool Actor::moveInDirection(Direction direction)
{
    switch(direction) {
        case left:
        {
            //std::cout << "LEFT in moveInDirection" << "\n";
            if (getX() > 0 && !getWorld()->isThereBoulderInDirection(getX(), getY(), left, this)) {
                moveTo(getX()-1, getY());
                //std::cout << "should've moved to left" << "\n";
                return true;
            }
            return false;
        }
            
        case right:
        {
            //std::cout << "RIGHT in moveInDirection" << "\n";
            if (getX() < (VIEW_HEIGHT - SPRITE_WIDTH) && !getWorld()->isThereBoulderInDirection(getX(), getY(), right, this)) {
                moveTo(getX()+1, getY());
                return true;
            }
            return false;
        }
            
        case up:
        {
            if (getY() < (VIEW_HEIGHT - SPRITE_WIDTH) && !getWorld()->isThereBoulderInDirection(getX(), getY(), up, this)) {
                moveTo(getX(), getY()+1);
                return true;
            }
            return false;
        }
            
        case down:
        {
            if (getY() > 0 && !getWorld()->isThereBoulderInDirection(getX(), getY(), down, this)) {
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

// TODO: divide into regular and hardcore

Protestor::Protestor(StudentWorld* world, int level): People(world, TID_PROTESTER, MAX_COORDINATE, MAX_COORDINATE, left, 5), m_leaving(false), m_stunned(false), m_level(level), m_tickNonRest(0)
{
    m_ticksToWaitBetweenMoves = fmax(0, 3 - m_level / 4);
    setNumSquaresToMoveInCurrentDirection();
    m_tickSinceLastTurn = -200;
    m_tickNonRestSinceShouted = -15;
};

// numSquaresToMoveInDirection should be a random number between 8 and 60
void Protestor::setNumSquaresToMoveInCurrentDirection()
{
    numSquaresToMoveInCurrentDirection = (rand() % 53) + 8;
}

void Protestor::leaveOilField()
{
    if (getX() == MAX_COORDINATE && getY() == MAX_COORDINATE) {
        setDead();
        getWorld()->decreaseNumOfProtestors();
        return;
    }
    
    else {
//        Direction directionTowardsExit = getWorld()->getDirectionToExit(getX(), getY());
//        setDirection(directionTowardsExit);
        setRandomDirection();
        Direction d = getDirection();
        moveInDirection(d);
    }
    
}

void Protestor::protestorMove()
{
    if (!getWorld()->isThereEarthInDirection(getX(), getY(), getDirection())) {
        if (!moveInDirection(getDirection()))
            numSquaresToMoveInCurrentDirection = 0;
    }
    else numSquaresToMoveInCurrentDirection = 0;
}

void Protestor::doSomething()
{
    if (!isAlive()) return;
    
    if (m_tickRest < m_ticksToWaitBetweenMoves) {
        m_tickRest++;
        return;
    }

    if (m_stunned) {
        //Resetting how long the protester has to wait because it's not stunned anymore
        m_ticksToWaitBetweenMoves = fmax(0, 3 - m_level / 4);
        m_stunned = false;
    }
    
    if (m_leaving) {
        m_tickRest = 0;
        leaveOilField();
        return;
    }
    
    else if (getWorld()->getDistanceFromTunnelMan(getX(), getY()) <= 4.0 && getWorld()->isProtestorFacingTunnelMan(getX(), getY(), getDirection()))
    {
        if (m_tickNonRest - m_tickNonRestSinceShouted >= 15) {
            //getWorld()->shoutAtTunnelMan();
            m_tickNonRestSinceShouted = m_tickNonRest;
        }
        m_tickRest = 0;
        ++m_tickNonRest;
        return;
    }
    
    else if (getWorld()->getDistanceFromTunnelMan(getX(), getY()) > 4.0 && canSeeTunnelMan())
    {
        m_tickRest = 0;
        ++m_tickNonRest;
        numSquaresToMoveInCurrentDirection = 0;
        return;
    }
    
    --numSquaresToMoveInCurrentDirection;

    // cannot directly see the TunnelMan
    
    if (numSquaresToMoveInCurrentDirection <= 0) {
        setRandomDirection();

        // If the random direction is blocked either by Earth or a Boulder such that it can’t take even a single step in that chosen direction, then it will select a different direction and check it for blockage (it will continue checking directions until it has picked a direction that is not blocked).
        while (getWorld()->isThereEarthInDirection(getX(), getY(), getDirection()) || getWorld()->isThereBoulderInDirection(getX(), getY(), getDirection(), nullptr)) {
            setRandomDirection();
        }
        
        setNumSquaresToMoveInCurrentDirection();
    }
    
    else {
        if (m_tickNonRest - m_tickSinceLastTurn >= 200 && sittingAtIntersection()) {
            m_tickSinceLastTurn = m_tickNonRest;
            setNumSquaresToMoveInCurrentDirection();
        }
    }
    protestorMove();
    m_tickRest = 0;
    m_tickNonRest++;
}

void Protestor::annoy(int damage)
{
    if (!m_leaving) {
        takeDamage(damage);
        
        if (getHitPoint() <= 0) {
            getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
            
            // TODO: Increase score by annoying Protestor
            
            m_leaving = true;
            m_tickRest = m_ticksToWaitBetweenMoves;
        }
        
        else {
            getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
            m_stunned = true;
            m_ticksToWaitBetweenMoves = fmax(50, 100 - m_level * 10);
        }
    }
}

bool Protestor::canSeeTunnelMan()
{
    GraphObject::Direction direction;
    if(getWorld()->isThereTunnelManInLine(getX(), getY(), direction))
    {
        setDirection(direction);
        moveInDirection(direction);
        return true;
    }
    return false;
}

void Protestor::setRandomDirection()
{
    int choice = rand() % 4;
    switch(choice) {
        case 0:
            setDirection(up);
            break;
        case 1:
            setDirection(right);
            break;
        case 2:
            setDirection(down);
            break;
        case 3:
            setDirection(left);
            break;
    }
}

// is sitting at an intersection where it could turn and move at least one square in a perpendicular direction from its currently facing direction (e.g., it is currently facing left, and is at a junction where it could turn and move one step either upward/downward without being blocked by Earth or a Boulder), and The Regular Protester hasn’t made a perpendicular turn in the last 200 non- resting ticks.

bool Protestor::sittingAtIntersection()
{
    
//    Determine which of the two perpendicular directions are viable (a viable
//    direction is one that allows movement of at least one square without the
//    Regular Protester being blocked by Earth or a Boulder).
    
    Direction direction1 = none;
    Direction direction2 = none;

    switch(getDirection())
    {
        case up:
        case down:
            if(!getWorld()->isThereBoulderInDirection(getX(), getY(), left, nullptr)
               && !getWorld()->isThereEarthInDirection(getX(), getY(), left)) {
                direction1 = left;
            }
            if(!getWorld()->isThereBoulderInDirection(getX(), getY(), left, nullptr)
               && !getWorld()->isThereEarthInDirection(getX(), getY(), left))
            {
                direction2 = right;
            }
            break;
            
        case right:
        case left:
            if(!getWorld()->isThereBoulderInDirection(getX(), getY(), left, nullptr)
               && !getWorld()->isThereEarthInDirection(getX(), getY(), left)){
                direction1 = up;
            }
            if(!getWorld()->isThereBoulderInDirection(getX(), getY(), left, nullptr)
               && !getWorld()->isThereEarthInDirection(getX(), getY(), left)) {
                direction2 = down;
            }
            break;
            
        case none:
            return false;
    }
    
//    Pick a viable perpendicular direction. If both perpendicular directions are
//    viable, then pick one of the two choices randomly.
    
    if (direction1 != none || direction2 != none) {
        if (direction1 != none && direction2 != none) {
            int directionChoice = rand() % 2;
            
            if(directionChoice == 0) setDirection(direction1);
            else setDirection(direction2);
        }
        
        else if (direction1 != none) setDirection(direction1);
        else setDirection(direction2);
        
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Goodies // includes oil, gold, water
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Goodie::Goodie(StudentWorld* world, int imageID, int startX, int startY, int score, int sound, int maxTickLife, bool isDisplayed): Actor(world, imageID, startX, startY, right, 1.0, 2, isDisplayed), m_sound(sound), m_score(score) {}

//Otherwise, if the Barrel is within a radius of 3.0 (<= 3.00 units away) from the TunnelMan,
bool Goodie::isPickupAbleByTunnelMan(const double &distance)
{
    if (distance <= 3.0) {
        setDead();
        getWorld()->playSound(m_sound);
        getWorld()->increaseScore(m_score);
        return true;
    }
    return false;
}

void Goodie::increaseTickPassed()
{
    ++m_tick;
}

int Goodie::getTickPassed()
{
    return m_tick;
}

int Goodie::getMaxTickLife()
{
    return m_maxTickLife;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gold
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Gold::Gold(StudentWorld * world,  int startX, int startY, bool isDropped, bool isDisplayed): Goodie(world, TID_GOLD, startX, startY, 10, SOUND_GOT_GOODIE, 100, isDisplayed), m_isPickupAbleByTunnelMan(isDropped) {}

void Gold::doSomething()
{
    if (!isAlive())
        return;

    if (m_isPickupAbleByTunnelMan) {
        
        double d = getWorld()->getDistanceFromTunnelMan(getX(), getY());
        
        if (!isVisible() && d <= 4.0) {
            std::cout << "oil이 거리 안에 있음 보여라 얍" << "\n";
            setVisible(true);
            return;
        }
        
        else if(isPickupAbleByTunnelMan(d)) {
            getWorld()->addToInventory("gold");
        }
    }
    
    //TODO: if tunnelman dropped it
    // isPickupAbleByProtestor();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Oil Barrel
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

OilBarrel::OilBarrel(StudentWorld* world, int startX, int startY): Goodie(world, TID_BARREL, startX, startY, 1000, SOUND_FOUND_OIL, 0, false) {};

void OilBarrel::doSomething()
{
    if (!isAlive())
        return;

// Otherwise, if the Barrel is not currently visible and the TunnelMan is within a radius of 4.0 of it (<= 4.00 units away)

    double d = getWorld()->getDistanceFromTunnelMan(getX(), getY());
    
    if (!isVisible() &&  d <= 4.0) {
        setVisible(true);
        return;

//Otherwise, if the Barrel is within a radius of 3.0 (<= 3.00 units away) from the
// TunnelMan, then the Barrel will activate,
    } else if (isPickupAbleByTunnelMan(d)){
        getWorld()->decreaseNumOfBarrels();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Waterpool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


WaterPool::WaterPool(StudentWorld * world, int startX, int startY, int maxTickLife): Goodie(world, TID_WATER_POOL, startX, startY, 100, SOUND_GOT_GOODIE, maxTickLife, true)
{}
//------------------------------------------
void WaterPool::doSomething()
{
    if (!isAlive())
        return;

    if (getTickPassed() == getMaxTickLife())
        setDead();
    else
    {
        double d = getWorld()->getDistanceFromTunnelMan(getX(), getY());

        if (isPickupAbleByTunnelMan(d))
            getWorld()->addToInventory("squirt");

        increaseTickPassed();
    }
}

