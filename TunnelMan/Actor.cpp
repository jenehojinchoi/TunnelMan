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


void Actor::getsAttacked(int damage) {};

bool Actor::getBribed()
{
    return false;
}

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
            if (getX() > 0 && !getWorld()->isThereBoulderInDirection(getX(), getY(), left, this)) {
                moveTo(getX()-1, getY());
                return true;
            }
            return false;
        }
            
        case right:
        {
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
    std::cout << "불리긴 함? " << "\n";
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
                if (getDirection() == left) {
                    if (moveInDirection(left)) {
                        if (getWorld()->isThereEarthAtPoint(getX(), getY())) {
                            getWorld()->digEarth(getX(), getY());
                            getWorld()->playSound(SOUND_DIG);
                        }
                    }
                }
                else setDirection(left);
                break;
            }
                
            case KEY_PRESS_RIGHT:
            {
                if (getDirection() == right) {
                    if (moveInDirection(right)) {
                        if (getWorld()->isThereEarthAtPoint(getX(), getY())) {
                            getWorld()->digEarth(getX(), getY());
                            getWorld()->playSound(SOUND_DIG);
                        }
                    }
                }
                else setDirection(right);
                break;
            }
                
            case KEY_PRESS_UP:
            {
                if (getDirection() == up) {
                    if (moveInDirection(up)) {
                        if (getWorld()->isThereEarthAtPoint(getX(), getY())) {
                            getWorld()->digEarth(getX(), getY());
                            getWorld()->playSound(SOUND_DIG);
                        }
                    }
                }
                else setDirection(up);
                break;
            }
                
            case KEY_PRESS_DOWN:
            {
                if (getDirection() == down) {
                    if (moveInDirection(down)) {
                        if (getWorld()->isThereEarthAtPoint(getX(), getY())) {
                            getWorld()->digEarth(getX(), getY());
                            getWorld()->playSound(SOUND_DIG);
                        }
                    }
                }
                else setDirection(down);
                break;
            }
                
            case KEY_PRESS_SPACE:
                if (m_squirt > 0) {
                    m_squirt--;
                    getWorld()->shootWithSquirt();
                }
                break;
            case 'z':
            case 'Z':
                if (m_sonar > 0) {
                    m_sonar--;
                    getWorld()->activateSonar(getX(), getY(), 12);
                }
                break;
            case KEY_PRESS_TAB:
                if (m_gold > 0) {
                    m_gold--;
                    getWorld()->dropGold(new Gold(getWorld(), getX(), getY(), false, true));
                }
                break;
        }
    }
}

void TunnelMan::getsAttacked(int damage)
{
    std::cout << "///////////////////////////////////////////////////////" << "\n";
    std::cout << "TunnelMan annoy called" << "\n";
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
// Boulder
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Boulder::Boulder(StudentWorld* world, int startX, int startY, bool isDisplayed): Actor(world, TID_BOULDER, startX, startY, down, 1.0, 1, isDisplayed), m_falling(false), m_tickWaiting(0) {}

void Boulder::fall()
{
    getWorld()->boulderHitsPeople(getX(), getY());
    
    if (!getWorld()->isThereEarthInDirection(getX(), getY(), down) && !getWorld()->isThereBoulderInDirection(getX(), getY(), down, this))
    {
        if (getY() > 0) moveTo(getX(), getY() - 1);
        else setDead();
    }
    //there is earth or boulder blocking the rock
    else setDead();
}

void Boulder::doSomething()
{
    if (!isAlive())
        return;
       
    if(!m_falling) {
        if (!getWorld()->isThereEarthInDirection(getX(), getY(), down))
            m_falling = true;
    }

    
    
    else { // if m_falling = true
        std::cout << "m_tickWaiting: " << m_tickWaiting << "\n";
       if (m_tickWaiting >= 30) {
           fall();

           if (m_tickWaiting == 30)
               getWorld()->playSound(SOUND_FALLING_ROCK);
       }
       ++m_tickWaiting;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Squirt
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Squirt::Squirt(StudentWorld* world, int startX, int startY, GraphObject::Direction d): Actor(world, TID_WATER_SPURT, startX, startY, d, 1.0, 1), m_ticksAlive(0) {}

void Squirt::doSomething()
{
    if(!isAlive())
        return;
    
    if (m_ticksAlive == 4) {
        setDead();
        return;
    }
    
    if (getWorld()->squirtHits(getX(), getY())) {
        setDead();
        return;
    }
    
    if (!getWorld()->isThereEarthInDirection(getX(), getY(), getDirection()) && !moveInDirection(getDirection()))
        setDead();
    //hits boulder or Earth
    else setDead();
    
    ++m_ticksAlive;
}

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
        // TODO: direct Protestor towards exit
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
    if (!isAlive())
        return;
    
    if (m_tickRest < m_ticksToWaitBetweenMoves) {
        m_tickRest++;
        return;
    }

    if (m_stunned) {
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
            //TODO: shout at TunnelMan;
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

    
    if (numSquaresToMoveInCurrentDirection <= 0) {
        setRandomDirection();

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

void Protestor::getsAttacked(int damage)
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

bool Protestor::sittingAtIntersection()
{
    Direction d1 = none;
    Direction d2 = none;

    switch(getDirection())
    {
        case up:
        case down:
            if(!getWorld()->isThereBoulderInDirection(getX(), getY(), left, nullptr)
               && !getWorld()->isThereEarthInDirection(getX(), getY(), left)) {
                d1 = left;
            }
            if(!getWorld()->isThereBoulderInDirection(getX(), getY(), left, nullptr)
               && !getWorld()->isThereEarthInDirection(getX(), getY(), left))
            {
                d2 = right;
            }
            break;
            
        case right:
        case left:
            if(!getWorld()->isThereBoulderInDirection(getX(), getY(), left, nullptr)
               && !getWorld()->isThereEarthInDirection(getX(), getY(), left)){
                d1 = up;
            }
            if(!getWorld()->isThereBoulderInDirection(getX(), getY(), left, nullptr)
               && !getWorld()->isThereEarthInDirection(getX(), getY(), left)) {
                d2 = down;
            }
            break;
            
        case none:
            return false;
    }
    
//    Pick a viable perpendicular direction. If both perpendicular directions are
//    viable, then pick one of the two choices randomly.
    
    if (d1 != none || d2 != none) {
        if (d1 != none && d2 != none) {
            int directionChoice = rand() % 2;
            
            if(directionChoice == 0) setDirection(d1);
            else setDirection(d2);
        }
        
        else if (d1 != none) setDirection(d1);
        else setDirection(d2);
        
        return true;
    }
    return false;
}

bool Protestor::getBribed()
{
    if (!m_leaving) {
        m_leaving = true;
        getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
        getWorld()->increaseScore(25);
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


Gold::Gold(StudentWorld * world,  int startX, int startY, bool isPickupAble, bool isDisplayed): Goodie(world, TID_GOLD, startX, startY, 10, SOUND_GOT_GOODIE, 100, isDisplayed), m_isPickupAbleByTunnelMan(isPickupAble) {}

void Gold::doSomething()
{
    if (!isAlive())
        return;

    if (m_isPickupAbleByTunnelMan) {
        
        double d = getWorld()->getDistanceFromTunnelMan(getX(), getY());
        
        if (!isVisible() && d <= 4.0) {
            setVisible(true);
            return;
        }
        
        else if(isPickupAbleByTunnelMan(d)) {
            getWorld()->addToInventory("gold");
        }
    }
    
    else isPickupAbleByProtestor();
}

void Gold::isPickupAbleByProtestor()
{
    if (getTickPassed() == getMaxTickLife()) {
        setDead();
    }
    else {
        if (getWorld()->checkForBribes(getX(), getY())) {
            setDead();
            getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
            getWorld()->increaseScore(25);
        }
        
        increaseTickPassed();
    }
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

void WaterPool::doSomething()
{
    if (!isAlive())
        return;

    if (getTickPassed() == getMaxTickLife())
        setDead();
    
    else {
        double d = getWorld()->getDistanceFromTunnelMan(getX(), getY());

        if (isPickupAbleByTunnelMan(d))
            getWorld()->addToInventory("squirt");

        increaseTickPassed();
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sonar
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Sonar::Sonar(StudentWorld * world, int startX, int startY, int maxTickLife): Goodie(world, TID_SONAR, startX, startY, 75, SOUND_GOT_GOODIE, maxTickLife, true)
{}

void Sonar::doSomething()
{
    if (!isAlive())
        return;

    if (getTickPassed() == getMaxTickLife())
        setDead();
    
    else {
        double d = getWorld()->getDistanceFromTunnelMan(getX(), getY());

        if (isPickupAbleByTunnelMan(d))
            getWorld()->addToInventory("sonar");

        increaseTickPassed();
    }
}
