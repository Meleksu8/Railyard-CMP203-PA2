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

static int findBestSiding(const Yard& yard, int next, int source) {
    int good=-3;
    int smallestTop=-4;
    int goodTop=0; //placeholder
    for(int s=1; s<=yard.getNumSidings(); s++){
        if(s==source || yard.getSiding(s).size() >= yard.getSidingCapacity()){
            continue;
        }
        if(!yard.getSiding(s).isEmpty() && (yard.getSiding(s).peek(goodTop) && goodTop > next)){
            if(smallestTop == -4 || smallestTop > goodTop){
                smallestTop=goodTop;
                good=s;
            }
        }else{
            if(yard.getSiding(s).isEmpty() && good==0 && smallestTop==-4){
                good=s;
            }
        }
    }
    return good;
}

static bool departed (Yard & yard, MoveList &plan){
    int frontA=0;
     int next=yard.nextWagon();
     if(yard.getArrival().peek(frontA) && frontA==next ){
            go(yard, plan, TRACK_A, TRACK_D);
            return true;
        }

        for(int s=1; s<=yard.getNumSidings(); s++){
            int topS=0;
            if(yard.getSiding(s).peek(topS) && topS==next){
                go(yard, plan, s, TRACK_D);
                return true;
            }
        }
        return false;
}

static int solver (Yard &yard, int counter, MoveList &plan){

    while (!yard.isAssembled() && (counter>0 || counter==-1))
    {
        if(counter>0){
            counter--;
        }
        int next=yard.nextWagon();
        int trainSize=yard.getTrainSize();

        bool isDeparted=departed(yard,plan);
        if(isDeparted){
            continue;
        }

        //sidingler içindeki nextin nerde olduğunu buluyor
        int fromTrack=TRACK_A;
        int w= 0;
        for(int s=1; s<= yard.getNumSidings(); s++){
            if(yard.getSiding(s).indexOf(next) != -1){
                fromTrack=s;
                yard.getSiding(s).peek(w);
                break;
            }
        }
        
        int bestS=findBestSiding(yard, next, fromTrack);
        if(bestS != -3){
            go(yard, plan, fromTrack, bestS);
            continue;
        }

        int bad=0;
        int largestDeparted=-1;
        int largestDS=0; //placeholder
        for(int s=1; s<=yard.getNumSidings(); s++){
            if(s==fromTrack || yard.getSiding(s).size() >= yard.getSidingCapacity()){
                continue;
            }
            Yard copy = yard;
            MoveList temp;
            go(copy, temp, fromTrack, s);
            int numOfDep=solver(copy, -1, temp);
            if(numOfDep>largestDeparted){
                largestDeparted=numOfDep;
                largestDS=s;
            }
            
        }
        go(yard, plan, fromTrack, largestDS);
    }
    return yard.getDeparted();
}

void planTrain(const Yard &start, MoveList &plan)
{
    Yard yard = start; // yard kopya
    if (yard.getTrainSize() == yard.getSidingCapacity()) {
        planGreedy(yard, plan);
        return;
    }
    int counter=-1;
    solver(yard, counter, plan);
 //     plan.clear();
    
    
}



