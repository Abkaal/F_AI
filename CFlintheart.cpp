#include "jinkou/CFlintheart.hpp"
#include "jinkou/Shikikan.hpp"

using namespace logic;
using namespace resources;
using namespace fields;

using namespace jinkou;

actions::action* CFlintheart::build(const std::vector<actions::action *> &possibleactions){ //#TODO: sprzątnąć ilość if'ów
    if(this->getloadedresources().get<resources::gold>()<75){//ładuje się
        if(quantumstockpile && this->getCoords()!=quantumstockpile->getCoords()){
            return move(possibleactions, quantumstockpile->getCoords());
        }
        logic::resourceset shopping_list;
        shopping_list.set<oil>(100-(this->getloadedresources().get<oil>()));
        shopping_list.set<metal>(100);
        shopping_list.set<gold>(75);
        currentMode=((this->goSuperScout)?SUPERSCOUT:SCOUT); this->goSuperScout=false;
        return findTransferAction(possibleactions, this->built_place, this, shopping_list);
    }
    if(go_to && *go_to != this->getCoords()) return move(possibleactions); // move, but don't change the mode;
    auto it=find_if(possibleactions.begin(), possibleactions.end(),
        [](actions::action *a){return dynamic_cast<actions::build*>(a)!=NULL;});
    auto buildAction=dynamic_cast<actions::build*>(*it);
    robots::warehouse* w;
    try{
        if(!this->quantumstockpile) w=new CZaiko(this->getCoords(), built_place->getSite()->cpt->getNewSite(this));
        else w=new CZaiko(this->getCoords(), quantumstockpile->getSite()->cpt->getNewSite(this));
    }
    catch(exceptions::badformatexception &ex){
        game::getgame().getout()<<ex.getmessage()<<std::endl;
    }
    buildAction->built=w;
    if(buildAction->verifyparameters()){
        currentMode=N;
        busy=false;
        shouldIBuild=false;
        delete go_to; go_to=NULL;
        TBuildReport *report = new TBuildReport;
        report->done=true;
        report->type=BUILD;
        this->report=report;
        return buildAction;
    }
    else return NULL;
}

void CFlintheart::autoPilot(){
    if(shouldIBuild){
        currentMode=BUILD;
    }
    else CMyMiner::autoPilot();
}

void CFlintheart::changeMode(){
    if(!shouldITakeOrders()) return;
    switch(order_codes.at(orders->o)){
        case BUILD:{
            currentMode=BUILD;
            // SOMETHING STINKS HERE. STINKS OF A DEAD BUG.
            if(this->orders->coords) scout_from=orders->coords;
            if(this->orders->coords2) scout_to=orders->coords2;
            if(this->orders->more_coords){
                go_to = new std::pair<int,int>((*orders->more_coords)[0]);
                delete this->orders->more_coords;
            }
            shouldIBuild=true;
            busy=true;
            if(this->orders->change_site) this->goSuperScout=true;
            if(go_to && scout_from && scout_to) break;
            busy=false;
            shouldIBuild=false;
            delete scout_from; scout_from=NULL;
            delete scout_to; scout_to=NULL;
            delete go_to; go_to=NULL;
            //no break;
        }
        default: CMyMiner::changeMode();
    }
    delete orders; orders=NULL;
}

actions::action* CFlintheart::chooseAction(const std::vector<actions::action *> &possibleactions) {
    try{
        CShougun::audience(dynamic_cast<CMyRobot*>(this))->toMap(getSurroundings(), getCoords());
    }
    catch(exceptions::badformatexception &ex){
        game::getgame().getout()<<ex.getmessage()<<std::endl;
    }
    changeMode();
    actions::action *the_action=NULL;
    while(!the_action){
        switch(currentMode){
            case BUILD: the_action=build(possibleactions); break;
            case AUTO: the_action=NULL; autoPilot(); break; 
            case SUPERSCOUT: the_action=superScout(possibleactions); break;
            default: the_action=defaultBehaviour(possibleactions); break;
        }
    }
    return the_action;
}