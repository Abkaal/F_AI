#include "jinkou/CMyMiner.hpp"

using namespace jinkou;

void CMyMiner::changeMode(){
    if(!shouldITakeOrders()) return;
    switch(order_codes.at(orders->o)){
        case MINE:{
            currentMode=AUTO;
            if(orders->more_coords) {
                if(deposits) {
                    for(auto& x:*orders->more_coords){
                        deposits->push_back(x);
                    }
                    delete orders->more_coords;
                }
                else deposits=orders->more_coords;
            }
            busy=true;
            if(deposits && deposits->size()>0) break;
            busy=false;
            if(deposits) delete deposits; deposits=NULL;
            //no break;
        }
        default:{
            CMyRobot::changeMode(); break;
        }
    }
    delete orders; orders=NULL;
}

bool fieldIsEmpty(fields::field *the_field){
    bool result=false;
    
    fields::resourcefield<resources::oil> *oil=dynamic_cast<fields::resourcefield<resources::oil>*>(the_field);
    if(oil) {result|=oil->isempty(); return result;}
    fields::resourcefield<resources::metal> *metal=dynamic_cast<fields::resourcefield<resources::metal>*>(the_field);
    if(metal) {result|=metal->isempty(); return result;}
    fields::resourcefield<resources::diamond> *diamond=dynamic_cast<fields::resourcefield<resources::diamond>*>(the_field);
    if(diamond) {result|=diamond->isempty(); return result;}
    fields::resourcefield<resources::gold> *gold=dynamic_cast<fields::resourcefield<resources::gold>*>(the_field);
    if(gold) result|=gold->isempty();
    
    return result;
}

actions::action* CMyMiner::mine(const std::vector<actions::action *> &possibleactions){
    // in this function return NULL means that chooseAction will be called again;
    //std::pair<int, int> delta;
    // 0 Raba Mode:
    if(!rabaCount && (+this->getloadedresources())>300){ // else mine more;
        return drop(possibleactions, 50);
    }
    // normal mode:
    if(go_to && CShougun::audience(this)->getMap(*go_to)=='E'){
        currentMode=AUTO;
        delete go_to; go_to=NULL;
        return NULL;
    }
    if(this->whereAreWeGoingFromHere()==std::pair<int,int>(0,0)){
        fields::field* the_field=this->getSurroundings()[std::pair<int, int>(0, 0)];
        if(fieldIsEmpty(the_field)){
            // left here for I don't know how long:
            /*
            if(this->deposits->size()<=0){
                TMineReport *mr = new TMineReport;
                mr->what_mined=CShougun::audience(this)->getMap(this->getCoords());
                mr->mined=true;
                this->report=mr;
            }
            */
            CShougun::audience(this)->depositRunOut(this->getCoords());
            currentMode=AUTO;
            return NULL;
        }
        //toto vvv chwilowe! ma wychodzić na spotkanie Raba, jak ktoś to napisze
        if(+this->getloadedresources()>300) return findMoveAction(possibleactions, std::pair<int,int>(1,1), false);
        return findMineAction(possibleactions);
    }
    else{
        continueMining=true;
        currentMode=MOVE;
        return NULL;
    }
}

void CMyMiner::autoPilot(){
    if(continueMining){
        continueMining=false;
        currentMode=MINE;
        return;
    }
    if(deposits && deposits->size()>0){
        currentMode=MINE;
        go_to = new std::pair<int, int>((*deposits)[0]);
        deposits->erase(deposits->begin());
        if(!(deposits->size())){
            delete deposits;
            deposits=NULL;
        }
    }
    else{
        if(deposits) delete deposits; deposits=NULL;
        busy=false;
        currentMode=N;
    }
}

actions::action* CMyMiner::defaultBehaviour(const std::vector<actions::action *> &possibleactions){
    actions::action *the_action=NULL;
    bool dozer=true;
    while(dozer && !the_action){ //WARNING! infinite loop potential
        switch(currentMode){
            case MOVE: the_action=move(possibleactions); break;
            case MINE: the_action=mine(possibleactions); break;
            case AUTO: the_action=NULL; autoPilot(); break;
            case SUPERSCOUT:
            case SCOUT: the_action=scout(possibleactions); break;
            case FETCH: the_action=load(possibleactions); break;
            case BUILD:
            case CREATE: currentMode=AUTO; break;
            case N: dozer=false; break;
        }
    }
    return the_action;
}

std::vector<std::pair<int,int>>* CMyMiner::giveDeposits(){
    // this one is always called with implicit current==true;
    std::vector<std::pair<int,int>>* tmp=deposits;
    deposits=NULL;
    auto x=this->getSurroundings();
    std::pair<int,int> p=std::pair<int,int>(0,0);
    if(!(x.at(p)->isempty())) tmp->insert(tmp->begin(), this->getCoords());
    return tmp;
}

std::vector<std::pair<int,int>>* CMyMiner::giveDeposits(CTaichou* requester, bool current){
    std::vector<std::pair<int,int>>* tmp=deposits;
    if(dynamic_cast<CTaichou*>(requester)){
        this->override=true;
        deposits=NULL;
        //if(current && !this->getSurroundings()[std::pair<int,int>(0,0)]->isempty()) tmp->insert(tmp->begin(), this->getCoords());
        if(current){
            //std::map<std::pair<int,int>,fields::fields>
            auto x=this->getSurroundings();
            std::pair<int,int> p=std::pair<int,int>(0,0);
            if(!(x.at(p)->isempty())) tmp->insert(tmp->begin(), this->getCoords());
        }
        return tmp;
    }
    else throw new exceptions::badformatexception("WHICH GODDAMN IDIOT WANTED TO STEAL MY DEPOSITS???");
}