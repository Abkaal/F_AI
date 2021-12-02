#include "jinkou/CBob.hpp"
//#include "jinkou/CNotBob.hpp"
#include "jinkou/CScrooge.hpp"
#include "jinkou/CChuck.hpp"
#include "jinkou/CFlintheart.hpp"
#include "jinkou/CRockerduck.hpp"
#include "jinkou/CGajeel.hpp"
#include "jinkou/CRaba.hpp"

using namespace logic;
using namespace resources;
using namespace fields;

using namespace jinkou;

void CBob::assignToBrigade(TBrigade* b, robots::robot* assignee, ERobot er){
    b->freshman=er;
    switch(er){
        // no need to check if dynamic_cast is not NULL - we forced the correct type earlier;
        case SCROOGE:       b->McDuck=dynamic_cast<CScrooge*>(assignee); break;
        case FLINTHEART:    b->Glomgold=dynamic_cast<CFlintheart*>(assignee); //no break here;
        case CHUCK:         b->Norris.push_back(dynamic_cast<CChuck*>(assignee)); break;
        case BOB:           b->builder=dynamic_cast<CBob*>(assignee); // no break here;
        case GAJEEL:        b->RedFox.push_back(dynamic_cast<CGajeel*>(assignee)); break;
        //case NOTBOB:        b->still_builder=dynamic_cast<CNotBob*>(assignee); break;
        case ROCKERDUCK:    b->Rockerfeller.push_back(dynamic_cast<CRockerduck*>(assignee)); break;
        case RABA:          b->herd.push_back(dynamic_cast<CRaba*>(assignee)); break;
        default:            assert(0);
    }
}

actions::action* CBob::create(const std::vector<actions::action *> &possibleactions, int ott){
    if(this->report) delete this->report;
    TCreateReport *tmp = new TCreateReport;
    this->report=tmp;
    ERobot to_make=NOTBOB;
    //if(this->getCoords()!=*go_to) return move(possibleactions); // if we're not on warehouse;
    if(this->bank){
        if(this->getCoords()!=this->bank->getCoords()) return move(possibleactions, this->bank->getCoords()); // if we're not on warehouse;
        else return move(possibleactions, this->quantumstockpile->getCoords());
    }
    if(!this->make_them->empty()) {
        to_make=this->make_them->front();
        if(swapped) to_make=this->make_them->at(1);
    }
    auto it=find_if(possibleactions.begin(), possibleactions.end(),
        [](actions::action *a){return dynamic_cast<actions::create*>(a)!=NULL;});
    auto createAction=dynamic_cast<actions::create*>(*it);
    robots::robot* r;
    CTaichou* t=NULL;
    if(this->orders->master) t=dynamic_cast<CTaichou*>(this->orders->master);
    if(t) this->new_brigade=t->getBrigade(this);
    if(to_make==SCROOGE) this->new_brigade=new TBrigade;
    tmp->what_made=to_make;
    switch(to_make){
        case SCROOGE:{
            r=new CScrooge(this->getCoords(),
                           this->quantumstockpile->getSite()->cpt->getDir()+1, 
                           this->new_brigade, this->quantumstockpile->getSite());
            CTaichou* MD=dynamic_cast<CTaichou*>(r);
            try{
                CShougun::audience(this)->appointTaichou(MD, this);
            }
            catch(exceptions::badformatexception &ex){
                game::getgame().getout()<<ex.getmessage()<<std::endl;
            }
            try{
                MD->setBrigade(this->new_brigade, this); // cause, as it's commonly known, it's Scrooge who's the Taichou;
            }
            catch(exceptions::badformatexception &ex){
                game::getgame().getout()<<ex.getmessage()<<std::endl;
            }
            break;
        }
        case CHUCK:         r=new CChuck(this->getCoords()); break;
        case BOB:           r=new CBob(this->getCoords(), this->new_brigade, this->make_them); break;
        //case NOTBOB:        r=new CNotBob(this->getCoords(), this->quantumstockpile, brig); break;
        case FLINTHEART:    r=new CFlintheart(this->getCoords(), this->quantumstockpile); break;
        case ROCKERDUCK:    r=new CRockerduck(this->getCoords()); break;
        case GAJEEL:        r=new CGajeel(this->getCoords()); break;
        case RABA:          r=new CRaba(this->getCoords()); break;
        default:            break; // will never happen, sth like assert false :D
    }
    createAction->created=r;
    createAction->oil_to_tank=ott;
    if(createAction->verifyparameters()){
        assignToBrigade(this->new_brigade, r, to_make);
        if(swapped) this->make_them->erase(make_them->begin()+1);
        else this->make_them->erase(make_them->begin());
        if(this->make_them->empty()){
            this->bank=NULL;
            delete make_them; make_them=NULL;
            currentMode=AUTO;
            tmp->finished=true;
            busy=false;
        }
        return createAction;
    }
    else {
        if(swapped) tmp->failed=true;
        else swapped=true;
        tmp->swapped=swapped;
        delete r; r=NULL;
        if(tmp->failed){
            this->bank=NULL;
            swapped=false;
            if(this->new_brigade->builder!=this) make_them=NULL; //jeżeli dorabiasz i tu wejdziesz, to się skończy kryptą.
            currentMode=AUTO;
            busy=false;
            return chooseAction(possibleactions);
        }
        else return create(possibleactions);
    }
}

void CBob::autoPilot(){
    if(make_them && !make_them->empty() &&
       getCost(make_them->at(0))<=this->quantumstockpile->getloadedresources()){
        this->deposits=this->giveDeposits();
        this->currentMode=CREATE;
    }
    else CGajeel::autoPilot();
}

void CBob::changeMode(){
    if(!shouldITakeOrders()) return;
    switch(order_codes.at(orders->o)){
        case CREATE:{
            currentMode=CREATE;
            if(this->orders->make_it) this->make_them=this->orders->make_it;
            //this->go_to = new std::pair<int,int>(this->quantumstockpile->getCoords());
            if(this->orders->change_site) this->quantumstockpile=this->orders->stockpile;
            if(this->orders->more_coords){
                if(this->deposits) delete this->deposits;
                this->deposits=this->orders->more_coords;
            }
            this->bank=NULL;
            busy=true;
            if(make_them/* && go_to*/) break;
            busy=false;
            //if(go_to) delete go_to; go_to=NULL;
            if(make_them) delete make_them; make_them=NULL;
            //no break;
        }
        default: CGajeel::changeMode();
    }
    delete orders; orders=NULL;
}

actions::action* CBob::chooseAction(const std::vector<actions::action *> &possibleactions) {
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
            case CREATE: the_action=create(possibleactions); break;
            case AUTO: the_action=NULL; autoPilot(); break;
            default: swapped=false; return CGajeel::chooseAction(possibleactions);
        }
    }
    return the_action;
}

void CBob::takeOrders(TOrders *os, CTaichou *requester){
    if(dynamic_cast<CTaichou*>(requester)){
        if(this->orders && os->o=="create"){
            this->orders->coords = new std::pair<int,int>(this->quantumstockpile->getCoords());
            if(os->make_it){
                if(!this->orders->make_it){
                    this->orders->make_it=os->make_it;
                }
                else{
                    this->orders->make_it->insert(this->orders->make_it->end(), os->make_it->begin(), os->make_it->end());
                }
            delete os;
        }
        else this->orders=os;
        }
    }
    else throw new exceptions::badformatexception("WHICH GODDAMN IDIOT TRIED TO GIVE ME ORDERS???");
}