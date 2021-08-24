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


void Actor::getAttacked(int damage) {};

bool Actor::getBribed()
{
    return false;
}

bool Actor::canBeAnnoyed() const
{
    return false;
}

// moveInDirection returns true when it is moved correctly
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

void TunnelMan::getAttacked(int damage)
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
    
    if (!getWorld()->isThereEarthInDirection(getX(), getY(), getDirection())) {
        if (!moveInDirection(getDirection()))
            setDead();
    }
    //hits boulder or Earth
    else setDead();
    
    ++m_ticksAlive;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protestors
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Protestor::Protestor(StudentWorld* world, int level, int imageID, int hitPoint, int score): People(world, imageID, MAX_COORDINATE, MAX_COORDINATE, left, hitPoint), m_leaving(false), m_stunned(false), m_level(level), m_tickNonRest(0), m_score(score), m_tickRest(0)
{
    m_ticksToWaitBetweenMoves = 2*fmax(0, 3 - (m_level / 4));
    std::cout << "m_ticksToWaitBetweenMoves: " << m_ticksToWaitBetweenMoves << "\n";
    setNumSquaresToMoveInCurrentDirection();
    m_tickLastTurn = -200;
    m_tickNonRestSinceShouted = -15;
};

// numSquaresToMoveInDirection should be a random number between 8 and 60
void Protestor::setNumSquaresToMoveInCurrentDirection()
{
    numSquaresToMoveInCurrentDirection = (rand() % 53) + 8;
}

void Protestor::leaveOilField()
{
    //std::cout << "LEAVING OIL FIELD" << "\n";
    // 1. x = 60, y= 60
    if (getX() == MAX_COORDINATE && getY() == MAX_COORDINATE) {
        setDead();
        getWorld()->decreaseNumOfProtestors();
        return;
    }
//
//    // 2. move closer to exit
    else {
        GraphObject::Direction d;
        getWorld()->getPathToPoint(getX(), getY(), MAX_COORDINATE, MAX_COORDINATE, d);
        setDirection(d);
        moveInDirection(d);
        //getWorld()->moveToExit(this, getX(), getY());
        //return;
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
    // 1.
    if (!isAlive()) {
        //std::cout << "step 1" << "\n";
        return;
    }
    
    // 2.
    if (!m_leaving && m_tickRest < m_ticksToWaitBetweenMoves) {
        //std::cout << "step 2" << "\n";
        m_tickRest++;
        return;
    }
    
    // 3.
    if (m_leaving) {
        //std::cout << "step 3" << "\n";
        //std::cout << "LEAVE!" << "\n";
        m_tickRest = 0;
        leaveOilField();
        return;
    }

    if (m_stunned) {
        //std::cout << "stunned" << "\n";
        m_ticksToWaitBetweenMoves = fmax(0, 3 - m_level / 4);
        m_stunned = false;
    }

    
    // 4.
    else if (getWorld()->getDistanceFromTunnelMan(getX(), getY()) <= 4.0 && getWorld()->canProtestorShout(getX(), getY(), getDirection())) {
        //std::cout << "step 4" << "\n";
        if (m_tickNonRest - m_tickNonRestSinceShouted >= 15) {
            
            getWorld()->protestorShoutsAtTunnelMan();
            m_tickNonRestSinceShouted = m_tickNonRest;
        }
        m_tickRest = 0;
        ++m_tickNonRest;
        return;
    }
    
    // 5. regular protestor and hardcore protestor behave differently
    // canSeeTunnelMan is different
    else if (getWorld()->getDistanceFromTunnelMan(getX(), getY()) > 4.0 && canSeeTunnelMan()) {
        //std::cout << "step 5" << "\n";
        m_tickRest = 0;
        ++m_tickNonRest;
        numSquaresToMoveInCurrentDirection = 0;
        return;
    }
    
    // 6.
    --numSquaresToMoveInCurrentDirection;

    if (numSquaresToMoveInCurrentDirection <= 0) {
        //std::cout << "step 6" << "\n";
        setRandomDirection();

        while (getWorld()->isThereEarthInDirection(getX(), getY(), getDirection()) || getWorld()->isThereBoulderInDirection(getX(), getY(), getDirection(), nullptr)) {
            setRandomDirection();
        }
        
        setNumSquaresToMoveInCurrentDirection();
    }
    
    // 7.
    else {
        //std::cout << "step 7" << "\n";
        if (m_tickNonRest - m_tickLastTurn >= 200 && sittingAtIntersection()) {
            //std::cout << "step 7-1" << "\n";
            m_tickLastTurn = m_tickNonRest;
            setNumSquaresToMoveInCurrentDirection();
        }
    }
    protestorMove();
    m_tickRest = 0;
    m_tickNonRest++;
}

void Protestor::getAttacked(int damage)
{
    if (!m_leaving) {
        takeDamage(damage);
        
        if (getHitPoint() <= 0) {
            getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
            
            if (damage == 2) getWorld()->increaseScore(m_score);
            // else protestor is bonked with a boulder
            
            m_leaving = true;
            m_tickRest = m_ticksToWaitBetweenMoves;
        }
        
        else {
            getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
            m_stunned = true;
            m_ticksToWaitBetweenMoves = 2*fmax(50, 100 - m_level * 10);
        }
    }
}

bool Protestor::canSeeTunnelMan()
{
    GraphObject::Direction direction;
    if (getWorld()->isThereTunnelManInLine(getX(), getY(), direction)) {
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
    if (getDirection() == up || getDirection() == down) {
        return (getWorld()->canMoveInDirection(getX(),getY(), left) || getWorld()->canMoveInDirection(getX(),getY(), right));
    }
    else {
        return (getWorld()->canMoveInDirection(getX(),getY(), up) || getWorld()->canMoveInDirection(getX(),getY(), down));
    }
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

HardcoreProtestor::HardcoreProtestor(StudentWorld * world, int level, int imageID, int hitPoint, int score): Protestor(world, level, TID_HARD_CORE_PROTESTER, 20, 250)
{}

bool HardcoreProtestor::getBribed()
{
    if (!m_leaving) {
        m_stunned = true;
        getWorld()->increaseScore(25);
        getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
        m_ticksToWaitBetweenMoves = fmax(50, 100 - (m_level * 10));
        return true;
    }
    return false;
}


bool HardcoreProtestor::canSeeTunnelMan()
{
    // 5. for hardCoreProtestor
    int M = 16 + m_level * 2; 
    Direction d;

    int n = getWorld()->getPathToTunnelMan(getX(), getY(), d);
    
    if (n <= M) {
        setDirection(d);
        moveInDirection(d);
        return true;
    }
    
    else if(getWorld()->isThereTunnelManInLine(getX(), getY(), d)) {
        setDirection(d);
        moveInDirection(d);
        return true;
    }
    return false; // temporary
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
        
        else if (isPickupAbleByTunnelMan(d)) {
            getWorld()->addToInventory("gold");
        }
    }
    
    else {
        isPickupAbleByProtestor();
    }
    increaseTickPassed();
}

void Gold::updateTickDropped()
{
    m_tickDropped = getTickPassed();
}

void Gold::isPickupAbleByProtestor()
{
    if (getTickPassed() == m_tickDropped + 100) {
        setDead();
    }
    
    else {
        if (getWorld()->checkForBribes(getX(), getY())) {
            setDead();
            getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
            getWorld()->increaseScore(25);
        }
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

    double d = getWorld()->getDistanceFromTunnelMan(getX(), getY());
    
    // Barrel is not currently visible and the TunnelMan is within a radius of 4.0 of it (<= 4.00 units away
    if (!isVisible() &&  d <= 4.0) {
        setVisible(true);
        return;

    //Otherwise, if the Barrel is within a radius of 3.0 (<= 3.00 units away) from the
    // TunnelMan, then the Barrel will activate,
    } else if (isPickupAbleByTunnelMan(d)){
        getWorld()->decreaseNumOfBarrels();
    }
    
    increaseTickPassed();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Waterpool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


WaterPool::WaterPool(StudentWorld * world, int startX, int startY): Goodie(world, TID_WATER_POOL, startX, startY, 100, SOUND_GOT_GOODIE, 0, true)
{}

void WaterPool::updateTickMade()
{
    m_tickMade = getTickPassed();
}

void WaterPool::doSomething()
{
    if (!isAlive())
        return;

    int tickLife = fmax(100, 300 - 10 * getWorld()->getLevel());
    // after maxlife, object is setDead.
    if (getTickPassed() == m_tickMade + tickLife) {
        setDead();
    }
    
    else {
        double d = getWorld()->getDistanceFromTunnelMan(getX(), getY());

        if (isPickupAbleByTunnelMan(d))
            // if picked up, add to tunnelman's inventory
            getWorld()->addToInventory("squirt");
    }
    
    increaseTickPassed();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sonar
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Sonar::Sonar(StudentWorld * world, int startX, int startY): Goodie(world, TID_SONAR, startX, startY, 75, SOUND_GOT_GOODIE, 0, true)
{}

void Sonar::updateTickMade()
{
    m_tickMade = getTickPassed();
}

void Sonar::doSomething()
{
    if (!isAlive())
        return;

    int tickLife = fmax(100, 300 - 10 * getWorld()->getLevel());
    // after maxlife, object is setDead.
    if (getTickPassed() == m_tickMade + tickLife)
        setDead();
    
    else {
        double d = getWorld()->getDistanceFromTunnelMan(getX(), getY());

        if (isPickupAbleByTunnelMan(d))
            // if picked up, add to tunnelman's inventory
            getWorld()->addToInventory("sonar");
    }
    increaseTickPassed();
}
