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


void planTrain(const Yard &start, MoveList &plan)
{
    plan.clear();
    Yard yard = start;

    const int Scapacity=yard.getSidingCapacity();
    const int Lcapacity=yard.getLoopCapacity();
    const int numSidings=yard.getNumSidings();
    const int trainSize=yard.getTrainSize();
    const int MAX_MOVES=200000;
    int moves=0;

    //1. We moved all the wagons in the arrival stack.
    auto stepMove = [&](int from, int to) -> bool {
        if (moves >= MAX_MOVES) return false;
        go(yard, plan, from, to);
        ++moves;
        return true;
    };

    // Helper: it finds best siding which have sufficient capacity and saves the (top > wagonVal) relationship 
    auto pickTargetSiding = [&](int wagonVal, int excludeSiding) -> int {
        int bestGood = -1;
        int bestDiff = 1000000000;
        int bestFallback = -1;
        int minSize = 1000000000;

        for (int s = 1; s <= numSidings; ++s)
        {
            if (s == excludeSiding) continue;
            if (yard.getSiding(s).size() >= Scapacity) continue;

            if (yard.getSiding(s).size() < minSize)
            {
                minSize = yard.getSiding(s).size();
                bestFallback = s;
            }

            int topS = 0;
            bool hasTop = yard.getSiding(s).peek(topS);
            if (!hasTop || topS > wagonVal)
            {
                int diff = hasTop ? (topS - wagonVal) : 1000000;
                if (diff < bestDiff)
                {
                    bestDiff = diff;
                    bestGood = s;
                }
            }
        }
        return (bestGood != -1) ? bestGood : bestFallback;
    };

    // Helper: if the loop is full, it moves front wagon to the appropriate siding.
    auto freeLoopSlot = [&]() -> bool {
        if (Lcapacity <= 0 || yard.getLoop().size() < Lcapacity) return true;
        int frontL = 0;
        if (!yard.getLoop().peek(frontL)) return true;

        int next = yard.nextWagon();
        if (frontL == next)
        {
            return stepMove(TRACK_L, TRACK_D);
        }
        int target = pickTargetSiding(frontL, -1);
        if (target == -1) return false;
        return stepMove(TRACK_L, target);
    };

    auto flushDepartures = [&]() -> bool {
        bool any = false;
        while (!yard.isAssembled() && moves < MAX_MOVES)
        {
            int next = yard.nextWagon();
            int val = 0;
            bool moved = false;

            if (yard.getArrival().peek(val) && val == next)
            {
                if (!stepMove(TRACK_A, TRACK_D)) break;
                moved = true;
            }
            else if (Lcapacity > 0 && yard.getLoop().peek(val) && val == next)
            {
                if (!stepMove(TRACK_L, TRACK_D)) break;
                moved = true;
            }
            else
            {
                for (int s = 1; s <= numSidings; ++s)
                {
                    if (yard.getSiding(s).peek(val) && val == next)
                    {
                        if (!stepMove(s, TRACK_D)) return any;
                        moved = true;
                        break;
                    }
                }
            }

            if (!moved) break;
            any = true;
        }
        return any;
    };

    //------------------–-------
    //FOR LONG TRAINS
    //--------------------------
    if (Scapacity >= trainSize && numSidings >= 2)
    {
        long long phase1End = 0;
        long long phase2End = 0;
        long long bucketMoves[8] = {0}; // Counter for next 250 slices
        const int BUCKET_SPAN = 250;

        auto recordBucketMove = [&](int curNext) {
            int idx = (curNext - 1) / BUCKET_SPAN;
            if (idx < 0) idx = 0;
            if (idx >= 8) idx = 7;
            bucketMoves[idx]++;
        };

        // Phase 1: All the wagons in the arrival moves to the S1 and if the wagon is equal to next, directly depart it
        while (!yard.getArrival().isEmpty() && moves < MAX_MOVES)
        {
            if (flushDepartures()) { recordBucketMove(yard.nextWagon()); continue; }
            int frontA = 0;
            if (!yard.getArrival().peek(frontA)) break;
            if (frontA == yard.nextWagon())
            {
                if (!stepMove(TRACK_A, TRACK_D)) break;
                recordBucketMove(yard.nextWagon());
            }
            else
            {
                if (!stepMove(TRACK_A, 1)) break;
                recordBucketMove(yard.nextWagon());
            }
        }
        phase1End = moves;

        // Phase 2: Distributes the wagons in S1 to the numSidings range (1..numSidings).
        int interval = (trainSize + numSidings - 1) / numSidings;
        if (interval < 1) interval = 1;

        while (!yard.getSiding(1).isEmpty() && moves < MAX_MOVES)
        {
            if (flushDepartures()) { recordBucketMove(yard.nextWagon()); continue; }
            int s1Top = 0;
            if (!yard.getSiding(1).peek(s1Top)) break;
            if (s1Top == yard.nextWagon())
            {
                if (!stepMove(1, TRACK_D)) break;
                recordBucketMove(yard.nextWagon());
                continue;
            }

            int bucket = (s1Top - 1) / interval;
            if (bucket < 0) bucket = 0;
            if (bucket >= numSidings) bucket = numSidings - 1;
            int target = bucket + 1;

            int chosen = -1;
            int bestDiff = 2147483647;
            if (target != 1 && yard.getSiding(target).size() < Scapacity)
            {
                chosen = target;
            }
            else
            {
                for (int s = 2; s <= numSidings; ++s)
                {
                    if (yard.getSiding(s).size() >= Scapacity) continue;
                    int topS = 0;
                    if (yard.getSiding(s).peek(topS) && topS > s1Top)
                    {
                        int diff = topS - s1Top;
                        if (diff < bestDiff) { bestDiff = diff; chosen = s; }
                    }
                }
                if (chosen == -1)
                {
                    for (int s = 2; s <= numSidings; ++s)
                    {
                        if (yard.getSiding(s).size() < Scapacity) { chosen = s; break; }
                    }
                }
            }

            if (chosen == -1 || !stepMove(1, chosen)) break;
            recordBucketMove(yard.nextWagon());
        }
        phase2End = moves;

        // Phase 3: Whichever siding 'next' is on, move it to D by opening only the items above it.
        while (!yard.isAssembled() && moves < MAX_MOVES)
        {
            if (flushDepartures()) { recordBucketMove(yard.nextWagon()); continue; }

            int next = yard.nextWagon();
            int src = -1;
            for (int s = 1; s <= numSidings; ++s)
            {
                if (yard.getSiding(s).indexOf(next) != -1)
                {
                    src = s;
                    break;
                }
            }
            if (src == -1) break;

            int topVal = 0;
            if (!yard.getSiding(src).peek(topVal)) break;
            if (topVal == next)
            {
                if (!stepMove(src, TRACK_D)) break;
                recordBucketMove(yard.nextWagon());
                continue;
            }

            // 1st Priority: Among sidings that do not contain 'next', the one where topS > topVal and the difference is the smallest.
            int bestGood = -1;
            int bestDiff = 2147483647;
            int bestEmpty = -1;
            int bestHighTop = -1;
            int maxTop = -2147483647;
            int bestFallback = -1;
            int minSz = 2147483647;

            for (int s = 1; s <= numSidings; ++s)
            {
                if (s == src) continue;
                int sz = yard.getSiding(s).size();
                if (sz >= Scapacity) continue;

                bool hasNext = (yard.getSiding(s).indexOf(next) != -1);

                if (sz < minSz && !hasNext) { minSz = sz; bestFallback = s; }
                if (sz == 0 && bestEmpty == -1 && !hasNext) bestEmpty = s;

                int topS = 0;
                if (yard.getSiding(s).peek(topS))
                {
                    if (!hasNext && topS > topVal)
                    {
                        int diff = topS - topVal;
                        if (diff < bestDiff)
                        {
                            bestDiff = diff;
                            bestGood = s;
                        }
                    }
                    if (!hasNext && topS > maxTop)
                    {
                        maxTop = topS;
                        bestHighTop = s;
                    }
                }
            }

            // If all other sidings have 'next', as a last resort selects it without checking for 'hasNext'.
            if (bestGood == -1 && bestEmpty == -1 && bestHighTop == -1 && bestFallback == -1)
            {
                for (int s = 1; s <= numSidings; ++s)
                {
                    if (s == src) continue;
                    int sz = yard.getSiding(s).size();
                    if (sz >= Scapacity) continue;
                    if (bestFallback == -1) bestFallback = s;

                    int topS = 0;
                    if (yard.getSiding(s).peek(topS) && topS > topVal)
                    {
                        int diff = topS - topVal;
                        if (diff < bestDiff) { bestDiff = diff; bestGood = s; }
                    }
                }
            }

            int dst = -1;
            if (bestGood != -1) dst = bestGood;
            else if (bestEmpty != -1) dst = bestEmpty;
            else if (bestHighTop != -1) dst = bestHighTop;
            else dst = bestFallback;

            if (dst == -1 || !stepMove(src, dst)) break;
            recordBucketMove(yard.nextWagon());
        }
        
    }
    //----------------------
    // LONG TRAIN LOGIC ENDS
    //----------------------

    //-------------
    //FOR MAIN FILES
    //--------------
    while (!yard.isAssembled() && moves < MAX_MOVES)
    {
        bool progressed = flushDepartures();
        if (yard.isAssembled() || moves >= MAX_MOVES) break;

        int next = yard.nextWagon();
        int sidingWithNext = 0;
        for (int s = 1; s <= numSidings; ++s)
        {
            if (yard.getSiding(s).indexOf(next) != -1)
            {
                sidingWithNext = s;
                break;
            }
        }

        // Scenario A: 'next' is not yet on the sidings (it is within an Arrival track or a Loop)
        if (sidingWithNext == 0)
        {
            // If there is a 'next' in the loop and the items ahead can be moved to an exit/siding, rotate the loop.
            int frontL = 0;
            if (Lcapacity > 0 && !yard.getLoop().isEmpty() && yard.getLoop().peek(frontL) && frontL != next)
            {
                int target = pickTargetSiding(frontL, -1);
                if (target != -1 && stepMove(TRACK_L, target))
                {
                    progressed = true;
                    continue;
                }
            }

            int mid = next + (trainSize - next) / 2;
            while (!yard.getArrival().isEmpty() && moves < MAX_MOVES)
            {
                if (flushDepartures())
                {
                    progressed = true;
                    next = yard.nextWagon();
                    mid = next + (trainSize - next) / 2;
                    break;
                }

                int frontA = 0;
                if (!yard.getArrival().peek(frontA)) break;

                // Holds on the loop if there's space available for nearby carriages.
                if (Lcapacity > 0 && frontA > next && frontA <= next + Lcapacity &&
                    yard.getLoop().size() < Lcapacity)
                {
                    if (!stepMove(TRACK_A, TRACK_L)) break;
                    progressed = true;
                    continue;
                }

                int targetSiding = -1;
                if (frontA <= mid)
                {
                    // Distributes the smaller wagons to the first half sidings.
                    for (int s = 1; s <= numSidings; ++s)
                    {
                        if (yard.getSiding(s).size() < Scapacity)
                        {
                            targetSiding = s;
                            break;
                        }
                    }
                }
                else
                {
                    // Distributes the large railcars starting from the last sidings.
                    for (int s = numSidings; s >= 1; --s)
                    {
                        if (yard.getSiding(s).size() < Scapacity)
                        {
                            targetSiding = s;
                            break;
                        }
                    }
                }

                if (targetSiding == -1)
                {
                    if (!freeLoopSlot()) break;
                    progressed = true;
                    continue;
                }

                if (!stepMove(TRACK_A, targetSiding)) break;
                progressed = true;
            }
        }
        // Case B: 'next' is inside a siding; opens the contents above it to other sidings / loop.
        else
        {
            while (!yard.getSiding(sidingWithNext).isEmpty() && moves < MAX_MOVES)
            {
                if (flushDepartures())
                {
                    progressed = true;
                    break;
                }

                int topVal = 0;
                if (!yard.getSiding(sidingWithNext).peek(topVal)) break;
                if (topVal == yard.nextWagon())
                {
                    if (!stepMove(sidingWithNext, TRACK_D)) break;
                    progressed = true;
                    break;
                }

                // If a carriage is due to arrive in the near future, it will be held back if there is space available in the loop.
                int curNext = yard.nextWagon();
                if (Lcapacity > 0 && topVal > curNext && topVal <= curNext + Lcapacity &&
                    yard.getLoop().size() < Lcapacity)
                {
                    if (!stepMove(sidingWithNext, TRACK_L)) break;
                    progressed = true;
                    continue;
                }

                int target = pickTargetSiding(topVal, sidingWithNext);
                if (target == -1)
                {
                    if (!freeLoopSlot()) break;
                    progressed = true;
                    continue;
                }

                if (!stepMove(sidingWithNext, target)) break;
                progressed = true;
            }
        }

        if (!progressed) break;
    }
}