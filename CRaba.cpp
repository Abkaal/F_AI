#include "jinkou/CRaba.hpp"
#include "jinkou/Shikikan.hpp"

#include "actions/transfer.h"

using namespace logic;
using namespace resources;
using namespace fields;

using namespace jinkou;

actions::action* CRaba::autoPilot(const std::vector<actions::action *> &possibleactions){
    //default mule behaviour - being a mule
    std::pair<int, int> delta;
    if(+this->getloadedresources()>300) return drop(possibleactions,80);
    else{ // should find sth to carry to Zaiko:
        if(this->shopping_list) return load(possibleactions);
        delta=this->whereAreWeGoingFromHere(this->myMaster->getCoords());
        if(delta==std::pair<int, int>(0, 0)){
            logic::resourceset masterState=this->myMaster->getloadedresources();
            if(+masterState<300) return NULL;
            logic::resourceset transfer_set=masterState;
            Invert(transfer_set);
            if(this->getloadedresources().get<oil>()>50){
                transfer_set.set<oil>(50-masterState.get<oil>());
            }
            busy=true;
            return findTransferAction(possibleactions, this, myMaster, transfer_set);
        }
        else return move(possibleactions, this->myMaster->getCoords());
    }
}

void CRaba::changeMode(){
    if(!shouldITakeOrders()) return;
    switch(order_codes.at(orders->o)){
        case FETCH:{
            currentMode=FETCH;
            if(orders->shopping_list) shopping_list=orders->shopping_list;
            if(orders->stockpile) bank=orders->stockpile;
            busy=true;
            if(shopping_list && bank) break;
            busy=false;
            delete shopping_list; shopping_list=NULL;
            bank=NULL;
        }
        case AUTO:{
            currentMode=AUTO;
            if(orders->master) myMaster=orders->master;
            if(orders->stockpile) quantumstockpile=orders->stockpile;
            if(orders->shopping_list) shopping_list=orders->shopping_list;
            if(myMaster && quantumstockpile) break;
            if(myMaster){
                quantumstockpile=NULL; currentMode=N; break;
            }
            myMaster=NULL;
            quantumstockpile=NULL;
            delete shopping_list; shopping_list=NULL;
        }
        default:{
            CMyRobot::changeMode();
        }
    }
    delete orders; orders=NULL;
}

actions::action* CRaba::chooseAction(const std::vector<actions::action *> &possibleactions) {
    changeMode();
    switch(currentMode){ //ma mieć FETCH i AUTO, SCOUT poszedł do MyRobot;
        case SUPERSCOUT: {
            actions::action* a=superScout(possibleactions);
            if(a) {return a; break;}
            // else no break - we want to call autoPilot;
        }
        case SCOUT: return scout(possibleactions); break;
        case FETCH: return load(possibleactions); break;
        case AUTO:  return autoPilot(possibleactions); break;
        case N:     return NULL; break;
        default:    return NULL; break;
    }
}

//Noo. I tak to się kończy, jak mnie samego zostawisz przy wspólnym projekcie :P -- XD teraz pytanie, czy z tego więcej jublu, czy pożytku xD