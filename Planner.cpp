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

    const int Scapacity = yard.getSidingCapacity();
    const int trainSize = yard.getTrainSize();
    const int MAX_MOVES = 200000;
    int moves = 0;

    auto stepMove = [&](int from, int to) -> bool {
        if (moves >= MAX_MOVES) return false;
        go(yard, plan, from, to);
        ++moves;
        return true;
    };

    auto flushDepartures = [&]() -> bool {
        bool any = false;
        while (!yard.isAssembled() && moves < MAX_MOVES)
        {
            int next = yard.nextWagon();
            int val = 0;
            if (yard.getArrival().peek(val) && val == next)
            {
                if (!stepMove(TRACK_A, TRACK_D)) break;
                any = true;
            }
            else if (yard.getSiding(1).peek(val) && val == next)
            {
                if (!stepMove(1, TRACK_D)) break;
                any = true;
            }
            else if (yard.getSiding(2).peek(val) && val == next)
            {
                if (!stepMove(2, TRACK_D)) break;
                any = true;
            }
            else if (yard.getSiding(3).peek(val) && val == next)
            {
                if (!stepMove(3, TRACK_D)) break;
                any = true;
            }
            else
            {
                break;
            }
        }
        return any;
    };

    while (!yard.isAssembled() && moves < MAX_MOVES)
    {
        bool progressed = flushDepartures();
        if (yard.isAssembled() || moves >= MAX_MOVES) break;

        int next = yard.nextWagon();
        int sidingWithNext = 0;
        for (int s = 1; s <= 3; ++s)
        {
            if (yard.getSiding(s).indexOf(next) != -1)
            {
                sidingWithNext = s;
                break;
            }
        }

        // Durum A: 'next' henüz siding'lerde değil (Arrival içinde bekliyor).
        // Arrival'dan çekip S1/S2/S3'e dağıt ki 'next' ortaya çıksın.
        if (sidingWithNext == 0)
        {
            int mid = next + (trainSize - next) / 2;
            while (!yard.getArrival().isEmpty() && moves < MAX_MOVES)
            {
                if (flushDepartures())
                {
                    progressed = true;
                    next = yard.nextWagon();
                    break;
                }

                int frontA = 0;
                if (!yard.getArrival().peek(frontA)) break;

                // Önce S2/S3 kapasitesi uygunsa küçük/büyük ayrımıyla dağıt, yer yoksa S1'i kullan
                if (frontA <= mid && yard.getSiding(2).size() < Scapacity)
                {
                    if (!stepMove(TRACK_A, 2)) break;
                    progressed = true;
                }
                else if (frontA > mid && yard.getSiding(3).size() < Scapacity)
                {
                    if (!stepMove(TRACK_A, 3)) break;
                    progressed = true;
                }
                else if (yard.getSiding(1).size() < Scapacity)
                {
                    if (!stepMove(TRACK_A, 1)) break;
                    progressed = true;
                }
                else if (yard.getSiding(2).size() < Scapacity)
                {
                    if (!stepMove(TRACK_A, 2)) break;
                    progressed = true;
                }
                else if (yard.getSiding(3).size() < Scapacity)
                {
                    if (!stepMove(TRACK_A, 3)) break;
                    progressed = true;
                }
                else
                {
                    break;
                }
            }
        }
        // Durum B: 'next' bir siding'in içinde (sidingWithNext), üstündeki vagonları diğer iki siding'e aktar.
        else
        {
            int other1 = (sidingWithNext == 1) ? 2 : 1;
            int other2 = (sidingWithNext == 3) ? 2 : 3;

            while (!yard.getSiding(sidingWithNext).isEmpty() && moves < MAX_MOVES)
            {
                int topVal = 0;
                if (!yard.getSiding(sidingWithNext).peek(topVal)) break;

                if (topVal == yard.nextWagon())
                {
                    if (!stepMove(sidingWithNext, TRACK_D)) break;
                    progressed = true;
                    break;
                }

                int top1 = 0, top2 = 0;
                bool has1 = yard.getSiding(other1).peek(top1);
                bool has2 = yard.getSiding(other2).peek(top2);
                bool can1 = yard.getSiding(other1).size() < Scapacity;
                bool can2 = yard.getSiding(other2).size() < Scapacity;

                // Azalan sırayı (top > topVal) koruyan ve farkı en küçük olan siding'i seç
                bool good1 = can1 && (!has1 || top1 > topVal);
                bool good2 = can2 && (!has2 || top2 > topVal);

                int target = -1;
                if (good1 && good2)
                {
                    int score1 = has1 ? (top1 - topVal) : 100000;
                    int score2 = has2 ? (top2 - topVal) : 100000;
                    target = (score1 <= score2) ? other1 : other2;
                }
                else if (good1)
                {
                    target = other1;
                }
                else if (good2)
                {
                    target = other2;
                }
                else if (can1 && can2)
                {
                    target = (yard.getSiding(other1).size() <= yard.getSiding(other2).size()) ? other1 : other2;
                }
                else if (can1)
                {
                    target = other1;
                }
                else if (can2)
                {
                    target = other2;
                }

                if (target == -1) break;
                if (!stepMove(sidingWithNext, target)) break;
                progressed = true;
            }
        }

        if (!progressed) break;
    }
}