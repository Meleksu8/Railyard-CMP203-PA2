#include "Planner.h"

/* Level 6. The three planners of Planner.h:
 *   planRookie  GIVEN, an example of a planner. Read it before you start.
 *   planGreedy  YOU, the greedy yard master of Section 6.2.
 *   planTrain   YOU, your own planner (Section 6.3).
 * You may add helper functions to this file (make them static).
 */

// Makes a move on the working copy of the yard and writes it into the plan.
static void go(Yard &yard, MoveList &plan, int from, int to)
{
    yard.move(from, to);
    plan.add(from, to);
}

// ---------------------------------------------------------------- Rookie (GIVEN)

// The first siding with room that is not 'except' (0 if there is none).
static int sidingWithRoom(const Yard &yard, int except)
{
    for (int s = 1; s <= yard.getNumSidings(); ++s)
        if (s != except && yard.getSiding(s).size() < yard.getSidingCapacity())
            return s;
    return 0;
}

//  1. Park the whole train: every wagon goes from A to the first siding that has room.
//  2. Dig the wagons out in order: move the wagons above the next wagon to other
//     sidings, then send the next wagon to D.
// Step 2 cannot get stuck in a League yard (Section 6.1 explains why).
void planRookie(const Yard &start, MoveList &plan)
{
    Yard yard = start; // work on a copy
    //sidinglere hepsini taşıdı
    while (!yard.getArrival().isEmpty())
        go(yard, plan, TRACK_A, sidingWithRoom(yard, 0));

    while (!yard.isAssembled())
    {
        int next = yard.nextWagon();
        int home = 0; // the siding that holds the next wagon
        for (int s = 1; s <= yard.getNumSidings() && home == 0; ++s)
            if (yard.getSiding(s).indexOf(next) != -1)
                home = s;

        int above = yard.getSiding(home).size() - 1 - yard.getSiding(home).indexOf(next);
        for (int i = 0; i < above; ++i)
            go(yard, plan, home, sidingWithRoom(yard, home));
        go(yard, plan, home, TRACK_D);
    }

}

// ---------------------------------------------------------------- Greedy (YOU)

void planGreedy(const Yard &start, MoveList &plan)
{
    // Section 6.2. Work on a copy of 'start'. Until the train is assembled:
    //    1. If the next wagon is at the front of A or on top of a siding, send it to D.
    //    2. Otherwise move the wagon in front of it (the front wagon of A, or the top
    //       wagon of its siding) to the siding that the greedy rule chooses.
    //  Follow the rule exactly: every move of your plan is compared with the move of the rule.
    Yard yard=start;
    while(!yard.isAssembled()){
        int frontA=0;
        int next=yard.nextWagon();
        int sidingTop=0;

        if(yard.getArrival().peek(frontA) && frontA==next){
            go(yard, plan, TRACK_A, TRACK_D);
            continue;
        }

        bool isMoved=false;
        for(int s=1; s<=yard.getNumSidings(); s++){
            if((yard.getSiding(s).peek(sidingTop) && sidingTop==next)){
                go(yard, plan, s, TRACK_D);
                isMoved=true;
                break;
            }
        }
        if(isMoved){
            continue;
        }

        int fromTrack=TRACK_A;
        int w= frontA;
        for(int s=1; s<= yard.getNumSidings(); s++){
            if(yard.getSiding(s).indexOf(next) != -1){
                fromTrack=s;
                yard.getSiding(s).peek(w);
                break;
            }
        }
        int good=0;
        int smallestTop=-1;
        int goodTop=0; //placeholder
        for(int s=1; s<=yard.getNumSidings(); s++){
            if(s==fromTrack || yard.getSiding(s).size() >= yard.getSidingCapacity()){
                continue;
            }
            if(!yard.getSiding(s).isEmpty() && (yard.getSiding(s).peek(goodTop) && goodTop > w)){
                if(smallestTop == -1 || smallestTop > goodTop){
                    smallestTop=goodTop;
                    good=s;
                }
            }else{
                if(yard.getSiding(s).isEmpty() && good==0 && smallestTop==-1){
                    good=s;
                }
            }
        }
        if(good != 0){
            go(yard, plan, fromTrack, good);
            continue;
        }
        int bad=0;
        int largestTop=-1;
        int badTop=0; //placeholder
        for(int s=1; s<=yard.getNumSidings(); s++){
            if(s==fromTrack || yard.getSiding(s).size() >= yard.getSidingCapacity()){
                continue;
            }
            if(yard.getSiding(s).peek(badTop) && badTop < w){
                if(badTop>largestTop){
                    largestTop=badTop;
                    bad=s;
                }
            }
        }
        if(bad !=0){
            go(yard, plan, fromTrack, bad);
        }else{break;}
    }
}

// ---------------------------------------------------------------- your planner (YOU)
// helper function 

static int findBestSiding(Yard &yard, int wagon, int excludeSiding, int nextWagon, int window)
{
    int bestGood = -1;
    int bestGoodDiff = 0; // top - wagon

    int bestBad = -1;
    int bestBadPenalty = 10000000;

    int cap = yard.getSidingCapacity();
    int numSidings = yard.getNumSidings();

    for (int s = 1; s <= numSidings; s++)
    {
        if (s == excludeSiding) continue;

        auto &sid = yard.getSiding(s);
        int sz = sid.size();
        if (sz >= cap) continue; // capacity is full

        int freeSpace = cap - sz;
        int topVal = 0;
        bool hasTop = sid.peek(topVal);

        if (!hasTop || topVal > wagon)
        {
            int diff = hasTop ? (topVal - wagon) : 100000;
            if (bestGood == -1 || diff < bestGoodDiff)
            {
                bestGood = s;
                bestGoodDiff = diff;
            }
            continue;
        }

        int nearFutureCount = 0;
        for (int index = 0; index < sz; index++)
        {
            int w = sid.get(index);
            if (w >= nextWagon && w < nextWagon + window)
            {
                ++nearFutureCount;
            }
        }

        int penalty = nearFutureCount * 10000;
        // wagon - topVal küçükse (yani topVal, wagon'a yakınsa) daha iyi
        penalty += (wagon - topVal) * 20;
        // dolu sidinglere hafif ceza
        penalty += (cap - freeSpace) * 5;

        if (penalty < bestBadPenalty)
        {
            bestBadPenalty = penalty;
            bestBad = s;
        }
    }

    if (bestGood != -1) return bestGood;
    return bestBad;
}

void planTrain(const Yard &start, MoveList &plan)
{
    plan.clear();
    Yard yard = start;

    while (!yard.isAssembled())
    {
        int frontA = 0;
        bool hasA = yard.getArrival().peek(frontA);
        int next = yard.nextWagon();

        if (hasA && frontA == next)
        {
            go(yard, plan, TRACK_A, TRACK_D);
            continue;
        }

        int frontL = 0;
        if (yard.getLoop().peek(frontL) && frontL == next)
        {
            go(yard, plan, TRACK_L, TRACK_D);
            continue;
        }

        int cap = yard.getSidingCapacity();
        int mostSpace = 0;
        for (int s = 1; s <= yard.getNumSidings(); ++s)
        {
            int space = cap - yard.getSiding(s).size();
            if (space > mostSpace) mostSpace = space;
        }

        // Büyük trenlerde (long) 50 vagonluk gibi dar pencereye sıkışmamak için ölçekli pencere
        int maxWin = (cap > 800) ? (cap / 200) : 3;
        if (maxWin < 3) maxWin = 3;
        //if (maxWin > 10) maxWin = 10;

        int window = mostSpace;
        if (window > maxWin) window = maxWin;
        if (window < 1) window = 1;

        int runLen = 0;
        int aSize = yard.getArrival().size();
        if (aSize > 0)
        {
            runLen = 1;
            for (int i = 0; i + 1 < aSize; ++i)
            {
                if (yard.getArrival().get(i) + 1 == yard.getArrival().get(i + 1))
                {
                    ++runLen;
                }
                else
                {
                    break;
                }
            }
        }

        if (yard.getLoop().getCapacity() > 0 &&
            yard.getLoop().size() < yard.getLoop().getCapacity() &&
            hasA && frontA > next && runLen >= 2)
        {
            go(yard, plan, TRACK_A, TRACK_L);
            continue;
        }

        int sWithNext = 0;
        for (int s = 1; s <= yard.getNumSidings(); ++s)
        {
            if (yard.getSiding(s).indexOf(next) != -1)
            {
                sWithNext = s;
                break;
            }
        }

        bool moved = false;

        if (sWithNext != 0)
        {
            int topS = 0;
            if (yard.getSiding(sWithNext).peek(topS) && topS == next)
            {
                go(yard, plan, sWithNext, TRACK_D);
                moved = true;
            }
            else if (yard.getSiding(sWithNext).peek(topS))
            {
                int targetS = findBestSiding(yard, topS, sWithNext, next, window);
                if (targetS != -1)
                {
                    go(yard, plan, sWithNext, targetS);
                    moved = true;
                }
            }
        }
        else if (hasA)
        {
            int targetS = findBestSiding(yard, frontA, -1, next, window);
            if (targetS != -1)
            {
                go(yard, plan, TRACK_A, targetS);
                moved = true;
            }
        }

        if (!moved)
        {
            break;
        }
    }
}