#include "jinkou/CMyRobot.hpp"

#include "jinkou/Shikikan.hpp"

using namespace jinkou;

bool CMyRobot::shouldITakeOrders(){
    if(!orders) return false;
    else {
        if(!orders->change_site && busy && !override) return false;
    }
    override=false;
    return true;
}

actions::action* CMyRobot::load(const std::vector<actions::action *> &possibleactions){
    std::pair<int, int> delta;
    busy=true; // just making sure another time, that we're really busy;
    assert(this->bank && this->shopping_list); // die roughly if you don't know where's the bank and what to take from it;
    delta=this->whereAreWeGoingFromHere(this->bank->getCoords());
    if(delta!=std::pair<int, int>(0, 0)){ //jeżeli nie jesteś w banku
        return findMoveAction(possibleactions, delta); //idź do banku
    }
    // we're sure you know where's the bank and what you should take from it, so:
    Invert(*shopping_list); //weź to, co masz przynieść
    auto tmp=bank;
    bank=NULL; //zapomnij, gdzie był bank
    currentMode=AUTO;
    logic::resourceset tmp2=(*shopping_list);
    delete shopping_list; shopping_list=NULL;
    busy=true;
    return findTransferAction(possibleactions, this, tmp, tmp2);
}
        
actions::action* CMyRobot::drop(const std::vector<actions::action *> &possibleactions, int fuel){ // the 2nd par.: how many oil they will fuel up;
    std::pair<int,int> delta;
    assert(this->quantumstockpile); // die roughly if you don't know your warehouse;
    delta=this->whereAreWeGoingFromHere(this->quantumstockpile->getCoords());
    if(delta!=std::pair<int, int>(0, 0)){
        return findMoveAction(possibleactions, delta);
    }
    logic::resourceset rs=this->getloadedresources();
    int o=this->quantumstockpile->getloadedresources().get<resources::oil>();
    int oo=rs.get<resources::oil>();
    rs.set<resources::oil>(std::min(o+oo,fuel)-oo);
    busy=false;
    currentMode=AUTO;
    return findTransferAction(possibleactions, this, quantumstockpile, rs);
}

actions::action* CMyRobot::scout(const std::vector<actions::action *> &possibleactions){
    std::pair<int, int> delta=std::pair<int, int>(1, 1);
    if(scout_from){
        delta=this->whereAreWeGoingFromHere(*this->scout_from);
        if(delta!=std::pair<int, int>(0, 0)){
            return findMoveAction(possibleactions, delta);
        }
        delete scout_from; scout_from=NULL;
    }
    assert(scout_to);
    try{
        CShougun::audience(dynamic_cast<CMyRobot*>(this))->toMap(getSurroundings(), getCoords()); //WARNING! Tutaj ma być wywołane toMap. I DON'T CARE HOW!
    }
    catch(exceptions::badformatexception &ex){
        logic::game::getgame().getout()<<ex.getmessage()<<std::endl;
    }
    delta=this->whereAreWeGoingFromHere(*this->scout_to);
    if(delta!=std::pair<int, int>(0, 0)){
        return findMoveAction(possibleactions, delta);
    }
    TScoutReport *report = new TScoutReport;
    report->finished=true;
    report->type=SCOUT;
    this->report=report;
    busy=false;
    currentMode=AUTO;
    delete scout_to; scout_to=NULL;
    return NULL; // correct result - we've just finished;
}

actions::action* CMyRobot::superScout(const std::vector<actions::action *> &possibleactions){
    if(!this->sd){
        this->sd=new TScoutingData;
    }
    else if(this->sd->turns==0){
        if(this->sd->tmp_ss<SITE_SIZE){
            this->sd->tmp_ss+=2;
            this->sd->direction=1;
            this->sd->steps=2;
            this->sd->turns=8*(this->sd->tmp_ss-1)+2;
            this->sd->started=false;
        }
        else{
            // scouting ends:
            TScoutReport *report = new TScoutReport;
            report->finished=true;
            report->type=SCOUT;
            this->report=report;
            busy=false;
            delete this->sd; this->sd=NULL; currentMode=AUTO;
            delete scout_to; scout_to=NULL;
            return NULL; // it's a correct result meaning we've just finished;
        }
    }
    assert(scout_to); // here we store the coordinates of the newly built Zaiko;
    //std::pair<int,int> tmp=this->getCoords();
    std::pair<int,int> delta=std::pair<int,int>(0,0);
    // actual scouting:
    if(this->sd->steps==0){
        this->sd->direction+=1;
        if(!this->sd->started){
            this->sd->steps=this->sd->tmp_ss-1; this->sd->started=true;
        }
        else this->sd->steps=2*(this->sd->tmp_ss-1);
    }
    if(!this->sd->got_to_zaiko){
        delta=whereAreWeGoingFromHere(*scout_to);
        if(delta!=std::pair<int,int>(0,0)) {
            --this->sd->steps;
            --this->sd->turns;
            return findMoveAction(possibleactions, delta);
        }
        else {
            this->sd->got_to_zaiko=true;
            return this->superScout(possibleactions); // simplest in code and ugliest as well ;x
        }
    }
    else{
        delta=dirToPair(this->sd->direction);
        --this->sd->steps;
        --this->sd->turns;
        return findMoveAction(possibleactions, delta);
    }
}

actions::action* CMyRobot::move(const std::vector<actions::action *> &possibleactions){
    std::pair<int, int> delta=this->whereAreWeGoingFromHere();
    if(delta==std::pair<int, int>(0, 0)){
        if(this->currentMode==MOVE) this->currentMode=AUTO;
        return NULL;
    }
    else{
        return findMoveAction(possibleactions, delta);
    }
}

actions::action* CMyRobot::move(const std::vector<actions::action *> &possibleactions, std::pair<int, int> acoords){
    std::pair<int, int> delta=acoords;
    if(delta==std::pair<int, int>(0, 0)) return NULL;
    else{
        return findMoveAction(possibleactions, delta);
    }
}

void CMyRobot::changeMode(){
    if(!shouldITakeOrders()) return;
    if(orders->change_site){
        this->shopping_list=orders->shopping_list;
        this->bank=this->quantumstockpile; //bo bieresz ze starego, a nie z tego, co to właśnie stawiają
        this->quantumstockpile=orders->stockpile;
    }
    switch(order_codes.at(orders->o)){
        case SUPERSCOUT:{
            currentMode=SUPERSCOUT;
            if(orders->coords) scout_to=orders->coords;
            busy=true;
            if(scout_to) break;
            busy=false;
            delete scout_to; scout_to=NULL;
        }
        case SCOUT:{
            currentMode=SCOUT;
            if(orders->coords) scout_from=orders->coords;
            if(orders->coords2) scout_to=orders->coords2;
            busy=true;
            if(scout_from && scout_to) break;
            busy=false;
            delete scout_from; scout_from=NULL;
            delete scout_to; scout_to=NULL;
            //no break;
        }
        case MOVE:{
            currentMode=MOVE;
            if(orders->coords) go_to=orders->coords;
            busy=true;
            if(go_to) break;
            busy=false;
            delete go_to; go_to=NULL;
            //no break;
        }
        case AUTO:{ //może by callować autoPilot()?
            currentMode=N;
            busy=false;
        }
        default:{
            currentMode=N; break;
        }
    }
    if(this->shopping_list) currentMode=FETCH;
    delete orders; orders=NULL;
}

actions::action* CMyRobot::fuelUp(const std::vector<actions::action *> &possibleactions){
    auto& rs = this->getmyposition()->getrobots(this);
    auto wit = find_if(rs.begin(), rs.end(), [](robot *r){
                            robots::warehouse *ww = dynamic_cast<robots::warehouse *>(r);
                            return ww != NULL;
                        });
    logic::resourceset shopping_list;
    shopping_list.set<resources::oil>(50-(this->getloadedresources().get<resources::oil>()));
    return findTransferAction(possibleactions, *wit, this, shopping_list);
}

std::pair<int, int> CMyRobot::whereAreWeGoingFromHere(std::pair<int, int> coords){
    std::pair<int, int> delta=std::pair<int,int>(0, 0);
    if(this->getCoords()!=coords){
        if(coords.first!=this->getCoords().first){
            if(coords.first>this->getCoords().first) {
                delta.first=-1; // move left;
                delta.second=0;
            }
            else {
                delta.first=1; //move right;
                delta.second=0;
            }
        }
        else if(coords.second!=this->getCoords().second){
            if(coords.first>this->getCoords().first) {
                delta.first=0; // move up;
                delta.second=-1;
            }
            else {
                delta.first=0; //move down;
                delta.second=1;
            }
        }
    }
    return delta;
}

std::pair<int, int> CMyRobot::whereAreWeGoingFromHere(){
    std::pair<int, int> delta=std::pair<int,int>(0,0);
    if(this->go_to){
        delta=whereAreWeGoingFromHere(*this->go_to);
        if(delta==std::pair<int, int>(0, 0)){
            delete go_to; go_to=NULL;
        }
    }
    return delta;
}

actions::action* CMyRobot::performaction(const std::vector<actions::action *> &possibleactions){
    actions::action* the_action;
    the_action=chooseAction(possibleactions);
    actions::move *moveaction=dynamic_cast<actions::move*>(the_action);
    if(moveaction){
        coords.first+=moveaction->delta.first;
        coords.second+=moveaction->delta.second;
    }
    return the_action;
}