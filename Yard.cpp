#include "Yard.h"
#include <iostream>

// ---------------------------------------------------------------- track names (GIVEN)

std::string trackName(int track)
{
    if (track == TRACK_A)
        return "A";
    if (track == TRACK_D)
        return "D";
    if (track == TRACK_L)
        return "L";
    return "S" + std::to_string(track);
}

bool parseTrack(const std::string &name, int &track)
{
    if (name == "A")
        track = TRACK_A;
    else if (name == "D")
        track = TRACK_D;
    else if (name == "L")
        track = TRACK_L;
    else
    {
        if (name.size() < 2 || name.size() > 5 || name[0] != 'S' || name[1] == '0')
            return false;
        int number = 0;
        for (std::string::size_type i = 1; i < name.size(); ++i)
        {
            if (name[i] < '0' || name[i] > '9')
                return false;
            number = number * 10 + (name[i] - '0');
        }
        track = number;
    }
    return true;
}

// ---------------------------------------------------------------- Yard

Yard::Yard(int _numSidings, int _sidingCapacity, int _loopCapacity)
    : sidings(nullptr), numSidings(_numSidings), sidingCapacity(_sidingCapacity),
      loopCapacity(_loopCapacity), trainSize(0), departed(0), moves(0)
{
    // Allocate numSidings empty sidings.
    if(numSidings>0){
        sidings=new WagonStack[numSidings];
    }
}

Yard::~Yard()
{
    // Free the array of sidings. This must also work for a yard that is not built.
    delete[] sidings;
    sidings=nullptr;
}

Yard::Yard(const Yard &other)
    : sidings(nullptr), numSidings(other.numSidings), sidingCapacity(other.sidingCapacity), loopCapacity(other.loopCapacity), loop(other.loop), arrival(other.arrival),
      trainSize(other.trainSize), departed(other.departed), moves(other.moves)
{
    // Deep copy: every siding, the loop, the arrival track and all counters.
    //  A copy of a yard that is not built is not built.
    if (other.isBuilt() && other.numSidings>0)
    {
        sidings=new WagonStack[numSidings];
        for(int i=0; i<numSidings; i++){
            sidings[i]=other.sidings[i];
        }
        
    }
    else{
        sidings=nullptr;
        numSidings=0;
    }
}

Yard &Yard::operator=(const Yard &other)
{
    // Deep copy. The old sidings must not leak, y = y must change nothing, and it
    //  must work when other (or *this) is not built.
    if(&other==this){
        return *this;
    }
    delete[] sidings;
    sidings=nullptr;

    numSidings=other.numSidings;
    sidingCapacity=other.sidingCapacity;
    loopCapacity=other.loopCapacity;
    trainSize=other.trainSize;
    departed=other.departed;
    moves=other.moves;
    arrival=other.arrival;
    loop=other.loop;

    if(other.isBuilt() && other.numSidings > 0){
        sidings=new WagonStack[numSidings];
        for(int i=0; i<numSidings; i++){
            sidings[i]=other.sidings[i];
        }
    }else{
        numSidings=0;
    }
    return *this;
}

bool Yard::hasTrack(int track) const
{
    // A, D, the sidings 1 .. numSidings, and L only if loopCapacity > 0.
    if(track==TRACK_A || track== TRACK_D){
        return true;
    }
    if(track>=1 && track<=numSidings){
        return true;
    }
    if(track==TRACK_L && loopCapacity > 0){
        return true;
    }
    return false;
}

bool Yard::isIdle() const
{
    // true if A, every siding and the loop are empty.
    if(isBuilt()){
        for(int i=0; i<numSidings; i++){
            if (!sidings[i].isEmpty())
            {
                return false;
            }
            
        }
    }
    if(!arrival.isEmpty() || !loop.isEmpty()){
        return false;
    }
    return true;
}

bool Yard::isAssembled() const
{
    // true if a train has arrived and all of its wagons are in D.
    if(trainSize>0 && trainSize==departed){
        return true;
    }
    return false;
}

void Yard::loadTrain(const int *wagons, int n)
{
    // The wagons enter A in the given order (wagons[0] is the front).
    //  trainSize = n, departed = 0, moves = 0.
    if(isBuilt() && isIdle()){
        for(int i=0; i<n; i++){
            arrival.enqueue(wagons[i]);
        }
        trainSize=n;
        departed=0;
        moves=0;
    }
}

bool Yard::movable(int track, int &wagon) const
{
    // The front of A or L, or the top of a siding. false if there is none.
    if (!hasTrack(track) || track==TRACK_D)
    {
        return false;
    }
    if(track==TRACK_A){
        return arrival.peek(wagon);
    }
    if(track==TRACK_L){
        return loop.peek(wagon);
    }
    if(track>=1 && track<=numSidings){
        return sidings[track-1].peek(wagon);
    }
    return false;
}

MoveResult Yard::check(int from, int to) const
{
    // The checks in the order of MoveResult (Section 1.2):
    //   1. both tracks exist in this yard          -> NO_SUCH_TRACK
    //   2. not from D, not to A, not from == to    -> ILLEGAL_MOVE
    //   3. there is a wagon on 'from'              -> SOURCE_EMPTY
    //   4. to D: the wagon is nextWagon()          -> WRONG_WAGON
    //   5. to a siding or the loop: it has room    -> TARGET_FULL
    if(!hasTrack(from) || !hasTrack(to)){
        return NO_SUCH_TRACK;
    }
    if(from==TRACK_D || to==TRACK_A || from==to){
        return ILLEGAL_MOVE;
    }
    if((from==TRACK_A && arrival.isEmpty()) || (from>=1 && from<= numSidings && sidings[from-1].isEmpty()) || (from==TRACK_L && loop.isEmpty())){
        return SOURCE_EMPTY;
    }
    int wagon=0;
    movable(from, wagon);
    if(to==TRACK_D && wagon != nextWagon()){
        return WRONG_WAGON;
    }
    if(to==TRACK_L && loop.size() >= loopCapacity){
        return TARGET_FULL;
    }
    if(to>=1 && to<= numSidings && sidings[to-1].size() >= sidingCapacity){
        return TARGET_FULL;
    }
    return MOVE_OK;
}

MoveResult Yard::move(int from, int to)
{
    //  If check(from, to) is not MOVE_OK, return it and change nothing.
    //  Otherwise take the wagon from 'from', put it on 'to' (for D: departed grows
    //  by one), count the move and return MOVE_OK.
    if(check(from, to) != MOVE_OK){
        return check(from, to);
    }
    int wagon=0;
    if (from==TRACK_A)
    {
        arrival.dequeue(wagon);
    }
    else if(from==TRACK_L){
        loop.dequeue(wagon);
    }
    else if(from>=1 && from<= numSidings){
        sidings[from-1].pop(wagon);
    }

    if(to==TRACK_A){
        arrival.enqueue(wagon);
    }
    else if(to==TRACK_L){
        loop.enqueue(wagon);
    }
    else if(to>=1 && to<= numSidings){
        sidings[to-1].push(wagon);
    }
    else if(to==TRACK_D){
        departed+=1;
    }
    moves+=1;
    
    return MOVE_OK;
}

// GIVEN. A is printed from the front, a siding from the bottom, L from the front.
void Yard::print() const
{
    std::cout << "--- Yard: " << departed << " of " << trainSize << " departed, " << moves
              << " move(s) ---" << std::endl;
    std::cout << "A:";
    for (int i = 0; i < arrival.size(); ++i)
        std::cout << " " << arrival.get(i);
    std::cout << std::endl;
    for (int s = 1; s <= numSidings; ++s)
    {
        std::cout << "S" << s << ":";
        for (int i = 0; i < sidings[s - 1].size(); ++i)
            std::cout << " " << sidings[s - 1].get(i);
        std::cout << std::endl;
    }
    if (loopCapacity > 0)
    {
        std::cout << "L:";
        for (int i = 0; i < loop.size(); ++i)
            std::cout << " " << loop.get(i);
        std::cout << std::endl;
    }
}
