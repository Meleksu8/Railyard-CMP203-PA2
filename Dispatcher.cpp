#include "Dispatcher.h"
#include <iostream>
#include <sstream>

void Dispatcher::cmdYard(const std::string &args)
{
    // YARD <numSidings> <sidingCapacity> <loopCapacity>   (Section 4.1)
    //  A new yard replaces the old one (yard = Yard(k, c, q);).
    //   "Yard built: <k> siding(s) of capacity <c>, loop capacity <q>."
    int k=0;
    int c=0;
    int q=0;

    std::istringstream ss(args);
    if(!(ss >> k >> c >> q)){
        return;
    }
    yard=Yard(k, c, q);
    std::cout << "Yard built: " << k << " siding(s) of capacity " << c << ", loop capacity " << q << "." << std::endl;
}

void Dispatcher::cmdTrain(const std::string &args)
{
    //     TRAIN <n> <w1> ... <wn>   (Section 4.2)
    //   - the yard is not idle -> "Error: The yard is not empty."
    //   - otherwise read the n wagon numbers into a dynamic array, load the train
    //     and free the array -> "Train of <n> wagon(s) arrived."
    int n=0;
    std::istringstream ss(args);
    if (!(ss >> n) || n < 0)
    {
        return;
    }
    if(!yard.isIdle()){
        std::cout << "Error: The yard is not empty." << std::endl;
        return;
    }
    
    int *wagons=new int[n];
    for(int i=0; i<n; i++){
        if(!(ss >> wagons[i])){
            delete[] wagons;
            return;
        }
    }
    yard.loadTrain(wagons, n);
    delete[] wagons;
    wagons=nullptr;

    std::cout << "Train of " << n << " wagon(s) arrived." << std::endl;
}

void Dispatcher::cmdMove(const std::string &args)
{
    std::istringstream ss(args);
    std::string fromName, toName;
    ss >> fromName >> toName;
    int from = 0, to = 0;
    parseTrack(fromName, from); // the input is well-formed, so both names are valid
    parseTrack(toName, to);

    //  MOVE <from> <to>   (Section 4.3). The two track numbers are in 'from' and 'to'.
    //  Make the move with yard.move and print one of:
    //    "Moved <w> from <from> to <to>."
    //       and then, if the train is complete, "Train assembled in <m> move(s)."
    //    "Error: No track <name>."              (check 'from' first, then 'to')
    //    "Error: Cannot move from <from> to <to>."
    //    "Error: <from> is empty."
    //    "Error: Wagon <w> cannot depart before wagon <next>."
    //    "Error: <to> is full."
    if(!yard.hasTrack(from)){
        std::cout << "Error: No track " << fromName << "." << std::endl;
        return;
    }
    if(!yard.hasTrack(to)){
        std::cout << "Error: No track " << toName << "." << std::endl;
        return;
    }

    MoveResult res=yard.check(from, to);
    int wagon=0;
    yard.movable(from, wagon);

    if(res == ILLEGAL_MOVE){
        std::cout << "Error: Cannot move from " << fromName << " to " << toName << "." << std::endl;
    }
    else if(res == WRONG_WAGON){
        std::cout << "Error: Wagon " << wagon << " cannot depart before wagon " << yard.nextWagon() << "." << std::endl;
    }
    else if(res == TARGET_FULL){
        std::cout << "Error: " << toName << " is full." << std::endl;
    }
    else if (res == MOVE_OK){
        yard.move(from, to);
        std::cout << "Moved " << wagon << " from " << fromName << " to " << toName << "." << std::endl;

        if(yard.isAssembled()){
            std::cout << "Train assembled in " << yard.getMoves() << " move(s)." << std::endl;
        }
    }
    else if(res == SOURCE_EMPTY){
        std::cout << "Error: " << fromName << " is empty." << std::endl;
    }
}

// GIVEN
void Dispatcher::processCommand(const std::string &line)
{
    std::istringstream ss(line);
    std::string cmd;
    if (!(ss >> cmd))
        return;
    std::string args;
    std::getline(ss, args);

    if (cmd == "YARD")
    {
        cmdYard(args);
        return;
    }
    if (cmd != "TRAIN" && cmd != "MOVE" && cmd != "PRINT")
    {
        std::cout << "Error: Unknown command '" << cmd << "'." << std::endl;
        return;
    }
    if (!yard.isBuilt())
    {
        std::cout << "Error: No yard." << std::endl;
        return;
    }
    if (cmd == "TRAIN")
        cmdTrain(args);
    else if (cmd == "MOVE")
        cmdMove(args);
    else
        yard.print();
}
